#pragma once

#include <vector>

class Dungeon
{
public:
    Dungeon() = default;

    Dungeon(Dungeon&&) noexcept = default;
    Dungeon(const Dungeon&) = default;

    Dungeon& operator=(Dungeon&&) = delete;
    Dungeon& operator=(const Dungeon&) = default;

    struct DungeonVertex
    {
        DungeonVertex() = default;
        DungeonVertex(float x, float y, float size)
            : mPx(x), mPy(y), mSize(size)
        {}

        float mPx{};
        float mPy{};
        float mSize{};
        std::vector<std::uint32_t> mConnections;

        bool operator==(const DungeonVertex& other) const;
    };

    struct DungeonEdge
    {
        DungeonEdge() = default;
        DungeonEdge(std::uint32_t a, std::uint32_t b)
            : mNode1(a), mNode2(b)
        {}

        std::uint32_t mNode1{};
        std::uint32_t mNode2{};

        bool operator==(const DungeonEdge& other) const;
    };

    std::vector<DungeonVertex> mVertices{};
    std::vector<DungeonEdge> mEdges{};

    float mSizeX = 100.0f;
    float mSizeY = 100.0f;

    struct DungeonGenerationData
    {
        int mNrVertices = 30;
        int mNrLoops = 5;

        float mMinVertexSize = 1.0f;
        float mMaxVertexSize = 3.0f;

        bool operator==(const DungeonGenerationData& other) const;
    };

    DungeonGenerationData mGenerationData{};

    void Generate();
};

class Dungeonerator {
public:





private:



};
