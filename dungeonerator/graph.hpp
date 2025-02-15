#pragma once
#include <iostream>
#include <map>
#include <vector>

class Graph
{
public:
    void AddVertex(int vertex) // data, generate vertex id
    {
        _graph[vertex] = {};
    }

    void AddEdge(int v1, int v2) // v1, v2, data
    {
        _graph[v1].push_back(v2);
        _graph[v2].push_back(v1);
    }

    void RemoveVertex(int vertex) { // remove vertex by id
        if (_graph.find(vertex) == _graph.end()) {
            std::cout << "invalid vertex" << std::endl;
            return;
        }

        _graph.erase(vertex);

        for (auto& e : _graph) {
            std::erase(e.second, vertex);
        }
    }

    void RemoveEdge(int v1, int v2) {
        if (!_graph.contains(v1) || !_graph.contains(v2)) {
            std::cout << "Invalid vertices" << std::endl;

            std::erase(_graph[v1], v2);
            std::erase(_graph[v2], v1);
        }
    }

    void PrintGraph() const {
        for (const auto& e : _graph) {
            std::cout << e.first << " : ";
            for (const auto& edge : e.second) {
                std::cout << edge << " ";
            }
            std::cout << std::endl;
        }
    }

private:

    std::map<int, std::vector<int>> _graph{};
};

