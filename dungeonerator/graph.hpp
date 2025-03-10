#pragma once
#include <iostream>
#include <map>
#include <unordered_map>
#include <unordered_set>
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

    VertexId _v1;
    VertexId _v2;

    T _data;
};

template <typename T>
class Vertex {
public:
    explicit Vertex(T vertexData)
        : _data(vertexData)
    {
    }

    std::vector<EdgeId> _edges;
    T _data;
};



class VertexData {
public:
    explicit VertexData(int a)
        : a(a) {
    }

    int a;
};

class EdgeData {
public:
    explicit EdgeData(int weight)
        : weight(weight) {
    }

    int weight;
};

template <typename vertexType, typename edgeType>
class Graph
{
public:

    VertexId AddVertex(vertexType vertexData)
    {
        VertexId curId = _nextVertex;
        _nextVertex++;

        _vertices.try_emplace(curId, Vertex(vertexData)); // Add to vertices

        return curId;
    }

    EdgeId AddEdge(VertexId v1, VertexId v2, edgeType edgeData)
    {
        EdgeId curId = _nextEdge;
        _nextEdge++;

        _vertices.at(v1)._edges.emplace_back(curId);
        _vertices.at(v2)._edges.emplace_back(curId);

        _edges.try_emplace(curId, Edge<edgeType>(v1, v2, edgeData));

        return curId;
    }

    void RemoveVertex(const VertexId v)
    {
        if (!_vertices.contains(v)) {
            std::cout << "tried to remove invalid vertex" << std::endl;
            return;
        }

        for (size_t i = 0; i < _vertices.at(v)._edges.size(); i++)
        {
            EdgeId edgeId = _vertices.at(v)._edges[i];

            auto& edge = _edges.at(edgeId);

            if (edge._v1 == v)
            {
                std::erase(_vertices.at(edge._v2)._edges, edgeId);
            } else {
                std::erase(_vertices.at(edge._v1)._edges, edgeId);
            }

            // Remove edge data
            _edges.erase(edgeId);
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
        const auto& it = std::find_if(_edges.begin(), _edges.end(), [&](std::pair<EdgeId, Edge<edgeType>> pair) {
            const auto& edge = pair.second;
            if ((edge._v1 == v1 && edge._v2 == v2) || (edge._v1 == v2 && edge._v2 == v1)) {
                return true;
            }
            return false;
        });

        if (it == _edges.end()) {
            return;
        }

        EdgeId edgeId = it->first;

        // Remove edge data
        _edges.erase(edgeId);

        // Remove edge from vertices
        std::erase(_vertices.at(v1)._edges, edgeId);
        std::erase(_vertices.at(v2)._edges, edgeId);
    }

    const std::unordered_map<VertexId, Vertex<vertexType>>& VertexData()
    {
        return _vertices;
    }

    const std::unordered_map<EdgeId, Edge<edgeType>>& EdgeData()
    {
        return _edges;
    }

private:

    std::unordered_map<VertexId, Vertex<vertexType>> _vertices;
    std::unordered_map<EdgeId, Edge<edgeType>> _edges;

    VertexId _nextVertex = 0;
    EdgeId _nextEdge = 0;
};

