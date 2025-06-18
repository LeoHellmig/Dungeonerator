#pragma once

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wall"
#include "generationUtils/delaunator.hpp"
#include "generationUtils/PoissonGenerator.hpp"
#pragma clang diagnostic pop
#include <random>
#include <chrono>
#include <unordered_set>
#include <queue>

namespace DungeonGenerator
{

struct VertexSizeBounds {
    float mMin = 1.0f;
    float mMax = 1.0f;
};

struct DungeonSize {
    float mSizeX = 100.0f;
    float mSizeY = 100.0f;
};

struct DungeonGenerationData
{
    DungeonGenerationData() = default;
    DungeonGenerationData(int verts, int loops, int seed, VertexSizeBounds vertexSizeBounds = {1.0f, 1.0f}, DungeonSize dungeonSize = {100.0f, 100.0f}, bool isCircle = true, bool generateGameplayContent = false, float treasureRoomPercentage = 0.3f)
        :   mNrVertices(std::max(verts, 3)),
            mNrLoops(std::min(loops, verts)),
            mMinVertexSize(vertexSizeBounds.mMin),
            mMaxVertexSize(vertexSizeBounds.mMax),
            mSizeX(dungeonSize.mSizeX),
            mSizeY(dungeonSize.mSizeY),
            mSeed(std::max(seed, 1)),
            mIsCircle(isCircle),
            mGenerateGameplayContent(generateGameplayContent),
            mTreasureRoomPercentage(treasureRoomPercentage) {}

    int mNrVertices = 3;
    int mNrLoops = 0;

    float mMinVertexSize = 1.0f;
    float mMaxVertexSize = 3.0f;

    float mSizeX = 100.0f;
    float mSizeY = 100.0f;

    int mSeed = 1;

    bool mIsCircle = false;

    bool mGenerateGameplayContent = false;
    float mTreasureRoomPercentage = 0.1f;
};

enum class RoomType
{
    START,
    BOSS,
    ENEMY,
    TREASURE,
    NUM_TYPES,
};

struct DungeonVertex
{
    DungeonVertex() = default;
    DungeonVertex(float x, float y, float size)
        : mPx(x), mPy(y), mSize(size)
    {}

    float mPx{};
    float mPy{};
    float mSize{};
    std::vector<std::uint32_t> mConnections; // Indices to connected vertices
    RoomType mType { RoomType::ENEMY };
};

struct DungeonEdge
{
    DungeonEdge() = default;
    DungeonEdge(std::uint32_t a, std::uint32_t b)
        : mNode1(a), mNode2(b)
    {}

    std::uint32_t mNode1{};
    std::uint32_t mNode2{};
};

class Dungeon
{
public:
    std::vector<DungeonVertex> mVertices{};
    std::vector<DungeonEdge> mEdges{};

    DungeonGenerationData mGenerationData{};

    explicit Dungeon(DungeonGenerationData generationData)
        : mGenerationData(generationData)
    {
        Generate();
    }

private:
    void Generate();
};

    using Timer = std::chrono::high_resolution_clock;

    static_assert(sizeof(uint32_t) == sizeof(unsigned int));

    using namespace DungeonGenerator;

#ifdef LOGGING
    inline double TimeToDouble(const std::common_type_t<std::chrono::duration<long long, std::ratio<1, 1000000000>>, std::chrono::duration<long long, std::ratio<1, 1000000000>>> time) {
        return std::chrono::duration_cast<std::chrono::duration<double>>(time).count();
    }
#endif

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
	std::vector<DungeonVertex> vertices;
	vertices.reserve(points.size());

#ifdef LOGGING
	std::cout << "Poisson in "<< TimeToDouble(Timer::now() - running) << " seconds"<< std::endl;
	running = Timer::now();
#endif

	std::vector<float> coords{};
	coords.reserve(points.size() * 2);

	for (auto& point : points)
	{
		const float x = point.x * mGenerationData.mSizeX;
		const float y = point.y * mGenerationData.mSizeY;

		coords.emplace_back(x);
		coords.emplace_back(y);

		vertices.emplace_back(x, y, sizeDistribution(gen));
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

	if (mGenerationData.mGenerateGameplayContent) {
		std::uniform_real_distribution<float> roomTypeDistribution(0.0f, 1.0f);

		for (auto& vert : vertices)
		{
				float roomType = roomTypeDistribution(gen);
				vert.mType = roomType < mGenerationData.mTreasureRoomPercentage ? RoomType::TREASURE : RoomType::ENEMY;
		}

		vertices.front().mType = RoomType::START;
		vertices.back().mType = RoomType::BOSS;
	}

	std::vector<DungeonEdge> mstEdges;
	mstEdges.reserve(edgeSet.size());

#ifdef LOGGING
	std::cout << "MST init "<< TimeToDouble(Timer::now() - running) << " seconds" << std::endl;
	running = Timer::now();
	uint32_t nrOfSearches = 0;
#endif

	// 0: weight
	// 1: connected to
	// 2: parent
	std::priority_queue<std::tuple<uint32_t, uint32_t, uint32_t>, std::vector<std::tuple<uint32_t, uint32_t, uint32_t>>, std::greater<>> pq;
	std::vector<bool> visited(vertices.size(), false);

	pq.emplace(0, 0, std::numeric_limits<uint32_t>().max());

	while(!pq.empty())
	{
		auto [wt, u, parent] = pq.top();
		pq.pop();

#ifdef LOGGING
		++nrOfSearches;
#endif

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
    int maxIterations = mGenerationData.mNrVertices * 3;
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
			if (iterations > static_cast<size_t>(maxIterations))
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

}