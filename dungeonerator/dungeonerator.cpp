#include "dungeonerator.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wall"
#include "generationUtils/delaunator.hpp"
#include "generationUtils/PoissonGenerator.hpp"
#pragma clang diagnostic pop
#include <random>
#include <chrono>
#include <unordered_set>
#include <forward_list>

using Timer = std::chrono::high_resolution_clock;

#ifdef LOGGING
double TimeToDouble(const std::common_type_t<std::chrono::duration<long long, std::ratio<1, 1000000000>>, std::chrono::duration<long long, std::ratio<1, 1000000000>>> time) {
	return std::chrono::duration_cast<std::chrono::duration<double>>(time).count();
}
#endif

bool Dungeon::DungeonVertex::operator==(const DungeonVertex& other) const
{
	return this->mSize == other.mSize
		&& this->mPx == other.mPx
		&& this->mPy == other.mPy
		&& this->mConnections == other.mConnections;
}

bool Dungeon::DungeonEdge::operator==(const DungeonEdge& other) const
{
	return this->mNode1 == other.mNode1
		&& this->mNode2 == other.mNode2;
}

template<> struct std::hash<Dungeon::DungeonEdge>
{
	std::size_t operator()(const Dungeon::DungeonEdge& s) const noexcept
	{
		uint64_t combined = (uint64_t(s.mNode1) << 32) | s.mNode2;
		return std::hash<uint64_t>{}(combined);
	}
};

bool Dungeon::DungeonGenerationData::operator==(const DungeonGenerationData& other) const
{
	return this->mNrVertices == other.mNrVertices
		&& this->mNrLoops == other.mNrLoops
		&& this->mMinVertexSize == other.mMinVertexSize
		&& this->mMaxVertexSize == other.mMaxVertexSize;
}

void Dungeon::Generate() {
#ifdef LOGGING
	const auto start = Timer::now();
	auto running = Timer::now();
	std::cout << "Dungeon generation started" << std::endl;
#endif

	//std::random_device rd;
	std::mt19937 gen(mGenerationData.mSeed);
	std::uniform_real_distribution<float> dis(mGenerationData.mMinVertexSize, mGenerationData.mMaxVertexSize);

	std::vector<Dungeon::DungeonVertex> verts{};
	size_t nrEdges = 0;

	PoissonGenerator::DefaultPRNG PRNG(mGenerationData.mSeed);

	auto points = PoissonGenerator::generatePoissonPoints(mGenerationData.mNrVertices, PRNG, mGenerationData.mIsCircle);

	if (points.size() > mGenerationData.mNrVertices)
	{
		points.erase(points.end() - (points.size() - static_cast<size_t>(mGenerationData.mNrVertices)), points.end());
	}

#ifdef LOGGING
	std::cout << "Poisson in "<< TimeToDouble(Timer::now() - running) << " seconds"<< std::endl;
	running = Timer::now();
#endif

	std::vector<float> coords{};
	coords.reserve(points.size() * 2);

	for (auto& point : points)
	{
		float x = point.x * mSizeX;
		float y = point.y * mSizeY;

		coords.emplace_back(x);
		coords.emplace_back(y);

		verts.push_back(Dungeon::DungeonVertex(x, y, dis(gen)));
	}

#ifdef LOGGING
	std::cout << "Converting poisson to coords in "<< TimeToDouble(Timer::now() - running) << " seconds" << std::endl;
	running = Timer::now();
#endif

	delaunator::Delaunator d(coords);

	for (std::size_t i = 0; i < d.triangles.size(); i+=3)
	{
		const auto& addEdge = [&](std::uint32_t a, std::uint32_t b)
			{
				verts[a].mConnections.push_back(b);
				verts[b].mConnections.push_back(a);
				++nrEdges;
			};

		addEdge(static_cast<std::uint32_t>(d.triangles[i]), static_cast<std::uint32_t>(d.triangles[i + 1]));
		addEdge(static_cast<std::uint32_t>(d.triangles[i + 1]), static_cast<std::uint32_t>(d.triangles[i + 2]));
		addEdge(static_cast<std::uint32_t>(d.triangles[i + 2]), static_cast<std::uint32_t>(d.triangles[i]));
	}

#ifdef LOGGING
	std::cout << "Delauny in "<< TimeToDouble(Timer::now() - running) << " seconds" << std::endl;
	running = Timer::now();
#endif

	std::vector<std::uint32_t> mstSet {};

	std::uniform_real_distribution<float> roomTypeDist(0.0f, 1.0f);
	std::vector<Dungeon::DungeonVertex> mstVerts = verts;
	for (auto& vert : mstVerts)
	{
		float roomType = roomTypeDist(gen);

		if (mGenerationData.mGenerateGameplayContent) {
			vert.mType = roomType < mGenerationData.mTreasureRoomPercentage ? Dungeon::RoomType::TREASURE : Dungeon::RoomType::ENEMY;
		}

		vert.mConnections.clear();
	}

	if (mGenerationData.mGenerateGameplayContent) {
		mstVerts.front().mType = Dungeon::RoomType::START;
		mstVerts.back().mType = Dungeon::RoomType::BOSS;
	}

	std::vector<Dungeon::DungeonEdge> mstEdges{};
	std::unordered_set<std::uint32_t> mstKeySet{};
	mstSet.emplace_back(0);
	mstKeySet.emplace(0);
	std::uniform_int<std::uint32_t> intDistribution(0, INT_MAX);

	std::vector<std::uint32_t> keys{};
	keys.reserve(nrEdges);
	for (size_t i = 0; i < nrEdges + 1; i++) {
		keys.emplace_back(intDistribution(gen));
	}

	std::size_t nrOfSearches = 0;

#ifdef LOGGING
	std::cout << "MST init "<< TimeToDouble(Timer::now() - running) << " seconds" << std::endl;
	running = Timer::now();
#endif

	size_t pointsSize = points.size();
	size_t mstSize = mstSet.size();

	while (mstSize < pointsSize)
	{
		std::uint32_t a {};
		std::uint32_t b {};
		std::uint32_t min = INT_MAX;

		for (size_t i = 0; i < mstSize; i++)
		{
			const auto& vert = verts[mstSet[i]];
			for (size_t j = 0; j < vert.mConnections.size(); j++)
			{
				++nrOfSearches;
				std::uint32_t edge = vert.mConnections[j];
				std::uint32_t key = keys[edge];
				if (key < min && !mstKeySet.contains(edge))
				{
					a = mstSet[i];
					b = edge;
					min = key;
				}
			}
		}

		++mstSize;
		// Keep adding verts to the set
		mstSet.emplace_back(b);
		mstKeySet.emplace(b);

		mstEdges.emplace_back(a, b);
		mstVerts[a].mConnections.emplace_back(b);
		mstVerts[b].mConnections.emplace_back(a);
	}

#ifdef LOGGING
	std::cout << "Made MST in "<< TimeToDouble(Timer::now() - running) << " seconds" << std::endl;
	std::cout << "MST required " << nrOfSearches << " searches" << std::endl;
	running = Timer::now();
#endif

	auto nextHalfEdge = [](size_t e) {
			return ((e % 3) == 2) ? e - 2 : e + 1;
		};

	size_t iterations = 0;
	for (int i = 0; i < mGenerationData.mNrLoops; i++)
	{
		++iterations;
		std::uniform_int_distribution<size_t> distribution(0, d.halfedges.size() - 1);
		size_t idx = distribution(gen);

		auto p1 = d.triangles[idx];
		auto p2 = d.triangles[nextHalfEdge(idx)];

		auto& p1Connections = mstVerts[p1].mConnections;
		auto& p2Connections = mstVerts[p2].mConnections;

		// If edge already exists continue
		if (std::ranges::find(p1Connections, p2) != p1Connections.end()
				|| std::ranges::find(p2Connections, p1) != p2Connections.end())
		{
			if (iterations > mGenerationData.mNrVertices * 3) {
				break;
			}
			--i;
			continue;
		}

		mstEdges.emplace_back(p1, p2);
		mstVerts[p1].mConnections.push_back(p2);
		mstVerts[p2].mConnections.push_back(p1);
	}

#ifdef LOGGING
	std::cout << "Added extra edges in "<< TimeToDouble(Timer::now() - running) << " seconds" << std::endl;
	std::cout << "Dungeon generated in "<< TimeToDouble(Timer::now() - start) << " seconds" << std::endl;
#endif

	mVertices = mstVerts;
	mEdges = mstEdges;
}
