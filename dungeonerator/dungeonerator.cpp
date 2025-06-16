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
#include <queue>

using Timer = std::chrono::high_resolution_clock;

static_assert(sizeof(uint32_t) == sizeof(unsigned int));

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

	std::mt19937 gen(mGenerationData.mSeed);
	std::uniform_real_distribution<float> sizeDistribution(mGenerationData.mMinVertexSize, mGenerationData.mMaxVertexSize);
	std::uniform_int<std::uint32_t> weightDistribution(0, std::numeric_limits<uint32_t>().max());

	PoissonGenerator::DefaultPRNG PRNG(mGenerationData.mSeed);
	auto points = PoissonGenerator::generatePoissonPoints(mGenerationData.mNrVertices, PRNG, mGenerationData.mIsCircle);

	if (points.size() > static_cast<size_t>(mGenerationData.mNrVertices))
	{
		points.erase(points.end() - (points.size() - static_cast<size_t>(mGenerationData.mNrVertices)), points.end());
	}

	std::vector<std::vector<std::vector<uint32_t>>> adjacent(points.size());
	std::vector<Dungeon::DungeonVertex> vertices;
	vertices.reserve(points.size());

#ifdef LOGGING
	std::cout << "Poisson in "<< TimeToDouble(Timer::now() - running) << " seconds"<< std::endl;
	running = Timer::now();
#endif

	std::vector<float> coords{};
	coords.reserve(points.size() * 2);

	for (auto& point : points)
	{
		const float x = point.x * mSizeX;
		const float y = point.y * mSizeY;

		coords.emplace_back(x);
		coords.emplace_back(y);

		vertices.push_back(Dungeon::DungeonVertex(x, y, sizeDistribution(gen)));
	}

#ifdef LOGGING
	std::cout << "Converting poisson to coords in "<< TimeToDouble(Timer::now() - running) << " seconds" << std::endl;
	running = Timer::now();
#endif

	const delaunator::Delaunator delaunay(coords);

#ifdef LOGGING
	std::cout << "Delauny in "<< TimeToDouble(Timer::now() - running) << " seconds" << std::endl;
	running = Timer::now();
#endif

	std::unordered_set<uint64_t> edgeSet{};

	for (std::size_t i = 0; i < delaunay.triangles.size(); i+=3)
	{
		const auto& addEdge = [&](uint32_t a, uint32_t b)
			{
				if (a < b) {
					std::swap(a, b);
				}
				uint64_t edgeKey = ((uint64_t) a << 32) | b;

				if (edgeSet.contains(edgeKey))
				{
					return;
				}

				auto weight = weightDistribution(gen);
				adjacent[a].push_back({b, weight});
				adjacent[b].push_back({a, weight});

				edgeSet.insert(edgeKey);
			};

		addEdge(static_cast<uint32_t>(delaunay.triangles[i]), static_cast<uint32_t>(delaunay.triangles[i + 1]));
		addEdge(static_cast<uint32_t>(delaunay.triangles[i + 1]), static_cast<uint32_t>(delaunay.triangles[i + 2]));
		addEdge(static_cast<uint32_t>(delaunay.triangles[i + 2]), static_cast<uint32_t>(delaunay.triangles[i]));
	}

	std::uniform_real_distribution<float> roomTypeDistribution(0.0f, 1.0f);

	for (auto& vert : vertices)
	{
		if (mGenerationData.mGenerateGameplayContent) {
			float roomType = roomTypeDistribution(gen);
			vert.mType = roomType < mGenerationData.mTreasureRoomPercentage ? Dungeon::RoomType::TREASURE : Dungeon::RoomType::ENEMY;
		}
		vert.mConnections.clear();
	}

	if (mGenerationData.mGenerateGameplayContent)
	{
		vertices.front().mType = Dungeon::RoomType::START;
		vertices.back().mType = Dungeon::RoomType::BOSS;
	}

	std::vector<Dungeon::DungeonEdge> mstEdges;
	mstEdges.reserve(edgeSet.size());

#ifdef LOGGING
	std::cout << "MST init "<< TimeToDouble(Timer::now() - running) << " seconds" << std::endl;
	running = Timer::now();
#endif

	// 0: weight
	// 1: connected to
	// 2: parent
	std::priority_queue<std::tuple<uint32_t, uint32_t, uint32_t>, std::vector<std::tuple<uint32_t, uint32_t, uint32_t>>, std::greater<>> pq;
	std::vector<bool> visited(vertices.size(), false);

	pq.emplace(0, 0, std::numeric_limits<uint32_t>().max());

	uint32_t nrOfSearches = 0;

	while(!pq.empty())
	{
		auto [wt, u, parent] = pq.top();
		pq.pop();

		++nrOfSearches;

		if (visited[u] == true) {
			continue;
		}

		visited[u] = true;

		if (parent != std::numeric_limits<uint32_t>().max()) {
			mstEdges.emplace_back(parent, u);
			vertices[u].mConnections.emplace_back(parent);
			vertices[parent].mConnections.emplace_back(u);
		}

		for (auto v : adjacent[u]) {
			if (visited[v[0]] == false) {
				pq.emplace(v[1], v[0], u);
			}
		}
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
	std::uniform_int_distribution<size_t> distribution(0, delaunay.halfedges.size() - 1);
	for (int i = 0; i < mGenerationData.mNrLoops; i++)
	{
		++iterations;
		size_t idx = distribution(gen);
		auto p1 = delaunay.triangles[idx];
		auto p2 = delaunay.triangles[nextHalfEdge(idx)];

		auto& p1Connections = vertices[p1].mConnections;
		auto& p2Connections = vertices[p2].mConnections;

		// If edge already exists continue
		if (std::find(p1Connections.begin(), p1Connections.end(), p2) != p1Connections.end()
				|| std::find(p2Connections.begin(), p2Connections.end(), p1) != p2Connections.end())
		{
			if (iterations > mGenerationData.mNrVertices * 3)
			{
				break;
			}
			--i;
			continue;
		}

		mstEdges.emplace_back(p1, p2);
		vertices[p1].mConnections.push_back(p2);
		vertices[p2].mConnections.push_back(p1);
	}

#ifdef LOGGING
	std::cout << "Added extra edges in "<< TimeToDouble(Timer::now() - running) << " seconds" << std::endl;
	std::cout << "Dungeon generated in "<< TimeToDouble(Timer::now() - start) << " seconds" << std::endl;
#endif

	mVertices = vertices;
	mEdges = mstEdges;
}