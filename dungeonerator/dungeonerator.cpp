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

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dis(mGenerationData.mMinVertexSize, mGenerationData.mMaxVertexSize);

	std::vector<Dungeon::DungeonVertex> verts{};
	std::vector<Dungeon::DungeonEdge> edges{};
	std::forward_list<Dungeon::DungeonEdge> listEdges{};
	size_t nrEdges = 0;

	PoissonGenerator::DefaultPRNG PRNG(std::time(NULL));

	const auto points = PoissonGenerator::generatePoissonPoints(mGenerationData.mNrVertices, PRNG, true);

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
				listEdges.emplace_front(a, b);
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

	for (auto& vert : verts)
	{
		vert.mConnections.clear();
	}

	size_t edgeCount = 0;

	{ // Remove double edges
		std::unordered_set<Dungeon::DungeonEdge> encounteredEdges{};

		auto prev = listEdges.before_begin();
		auto it = listEdges.begin();

		for (size_t i = 0; i < nrEdges - 1; i++)
		{
			auto& edge = *it;
			Dungeon::DungeonEdge inverseEdge(edge);
			inverseEdge.mNode1 = edge.mNode2;
			inverseEdge.mNode2 = edge.mNode1;

			if (encounteredEdges.contains(edge))
			{
				listEdges.erase_after(prev);
			}
			else
			{
				if (encounteredEdges.contains(inverseEdge))
				{
					listEdges.erase_after(prev);
				}
				encounteredEdges.emplace(edge);
				encounteredEdges.emplace(inverseEdge);

				verts[edge.mNode1].mConnections.emplace_back(edge.mNode2);
				verts[edge.mNode2].mConnections.emplace_back(edge.mNode1);

				++edgeCount;
			}

			++prev;
			if (prev == listEdges.end()) {
				break;
			}
			it = prev;
			++it;
			if (it == listEdges.end()) {
				break;
			}
		}
	}

#ifdef LOGGING
	std::cout << "Cleaning edges in "<< TimeToDouble(Timer::now() - running) << " seconds" << std::endl;
	running = Timer::now();
#endif

	std::vector<std::uint32_t> mstSet {};

	std::vector<Dungeon::DungeonVertex> mstVerts = verts;
	for (auto& vert : mstVerts)
	{
		vert.mConnections.clear();
	}

	std::vector<Dungeon::DungeonEdge> mstEdges{};
	std::unordered_set<std::uint32_t> mstKeySet{};
	mstSet.emplace_back(0);
	mstVerts.emplace_back(verts[0]);
	std::uniform_int<std::uint32_t> intDistribution;

	std::vector<std::uint32_t> keys{};
	keys.reserve(edgeCount);
	for (int i = 0; i < edgeCount; i++) {
		keys.emplace_back(intDistribution(gen));
	}

	std::uint32_t nrOfSearches = 0;

#ifdef LOGGING
	std::cout << "MST init "<< TimeToDouble(Timer::now() - running) << " seconds" << std::endl;
	running = Timer::now();
#endif

	while (mstSet.size() <= points.size())
	{
		std::uint32_t a {};
		std::uint32_t b {};
		std::uint32_t min = INT_MAX;

		for (size_t i = 0; i < mstSet.size(); i++)
		{
			const auto& vert = verts[mstSet[i]];
			for (size_t j = 0; j < vert.mConnections.size(); j++)
			{
				nrOfSearches++;
				std::uint32_t edge = vert.mConnections[j];
				std::uint32_t key = keys[edge];
				if (key < min && !mstKeySet.contains(edge))
				{
					a = mstSet[i];
					b = vert.mConnections[j];
					min = key;
				}
			}
		}

		// Keep adding verts to the set
		auto pair = std::make_pair(a, b);

		mstSet.emplace_back(pair.second);
		mstKeySet.emplace(pair.second);

		mstEdges.emplace_back(pair.first, pair.second);
		mstVerts[pair.first].mConnections.emplace_back(pair.second);
		mstVerts[pair.second].mConnections.emplace_back(pair.first);
	}

#ifdef LOGGING
	std::cout << "Made MST in "<< TimeToDouble(Timer::now() - running) << " seconds" << std::endl;
	std::cout << "MST required " << nrOfSearches << " searches" << std::endl;
	running = Timer::now();
#endif

	auto nextHalfEdge = [](size_t e) {
			return ((e % 3) == 2) ? e - 2 : e + 1;
		};

	for (int i = 0; i < mGenerationData.mNrLoops; i++)
	{
		std::uniform_int_distribution<size_t> distribution(0, d.halfedges.size() - 1);
		size_t idx = distribution(gen);

		auto p1 = d.triangles[idx];
		auto p2 = d.triangles[nextHalfEdge(idx)];

		auto& p1Connections = mstVerts[p1].mConnections;
		auto& p2Connections = mstVerts[p2].mConnections;

		// If edge already exists continue
		if (std::find(p1Connections.begin(), p1Connections.end(), p2) != p1Connections.end()
				|| std::find(p2Connections.begin(), p2Connections.end(), p1) != p2Connections.end())
		{
			continue;
		}

		mstEdges.push_back(Dungeon::DungeonEdge(p1, p2));
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
