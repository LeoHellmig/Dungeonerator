#pragma once

#include <vector>

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

}