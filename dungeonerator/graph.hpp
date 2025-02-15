#pragma once
#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>

using gId = int;

using VertexId = gId;
using EdgeId = gId;

template <typename T>
class Edge {
public:
    Edge(const VertexId v1, const VertexId v2, const T &data)
        : _v1(v1),
          _v2(v2),
          _data(data) {
    }

private:
    VertexId _v1;
    VertexId _v2;

    T _data;
};

template <typename vertexType, typename edgeType>
class Graph
{
public:

    VertexId AddVertex(vertexType& vertexData)
    {
        VertexId curId = _nextVertex;
        _nextVertex++;

        _vertices[curId]; // Add to vertices
        _vertexData[curId] = vertexData; // Add data

        return curId;
    }

    EdgeId AddEdge(VertexId v1, VertexId v2, edgeType& edgeData)
    {

        EdgeId curId = _nextEdge;
        _nextEdge++;

        _vertices[v1].emplace_back(curId);
        _vertices[v2].emplace_back(curId);

        Edge<edgeType> edge = Edge<edgeType>(v1, v2, edgeData);
        _edgeData[curId] = edge;

        return curId;
    }

    void RemoveVertex(const VertexId v)
    {
        if (!_vertices.contains(v)) {
            std::cout << "tried to remove invalid vertex" << std::endl;
            return;
        }

        for (EdgeId edgeId& : _vertices[v]) {
            auto& edge = _edgeData[edgeId];

            // Remove edge from both vertices
            _vertices[edge._v1].erase(edgeId);
            _vertices[edge._v2].erase(edgeId);

            // Remove data
            _edgeData.erase(edgeId);

            // Remove vertex data
            _vertexData.erase(v);
        }

        // Remove vertex
        _vertices.erase(v);
    }

    void RemoveEdge(VertexId v1, VertexId v2) {
        if (!_vertices.contains(v1) || !_vertices.contains(v2)) {
            std::cout << "tried to remove edge between invalid vertices" << std::endl;
            return;
        }

        // Find edge id
        EdgeId edgeId = std::find_if(_edgeData, _edgeData.end(), [&](Edge<edgeType>& edge) {
            if ((edge._v1 == v1 && edge._v2 == v2) || (edge._v1 == v2 && edge._v2 == v1)) {
                return true;
            }
            return false;
        });

        // Remove edge data
        _edgeData.erase(edgeId);

        // Remove edge from vertices
        std::erase(_vertices[v1], edgeId);
        std::erase(_vertices[v2], edgeId);
    }


    // void PrintGraph() const {
    //     for (const auto& e : _graph) {
    //         std::cout << e.first << " : ";
    //         for (const auto& edge : e.second) {
    //             std::cout << edge << " ";
    //         }
    //         std::cout << std::endl;
    //     }
    // }

private:

    std::unordered_map<VertexId, std::vector<EdgeId>> _vertices;
    std::unordered_map<VertexId, vertexType> _vertexData;
    std::unordered_map<EdgeId, Edge<edgeType>> _edgeData;

    VertexId _nextVertex = 0;
    EdgeId _nextEdge = 0;
};

