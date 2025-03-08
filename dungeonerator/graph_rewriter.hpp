#pragma once

#include <functional>

#include "graph.hpp"

// Base vertex data type including terminal data
class BaseVertexData {
private:
    bool isTerminal = false;
};

template <typename vertexType>
concept DerivedFromVertexBase = std::is_base_of_v<BaseVertexData, vertexType>;

template <DerivedFromVertexBase vertexType, typename edgeType>
class GraphRule {
public:
    Graph<vertexType, edgeType> _left;
    Graph<vertexType, edgeType> _right;
};

template <DerivedFromVertexBase vertexType, typename edgeType>
class GraphRewriter {
public:
    using RuleType = GraphRule<vertexType, edgeType>;
    using GraphType = Graph<vertexType, edgeType>;
    using Vertex = Vertex<vertexType>;

    void FindAndApplyRule(RuleType rule) {

    }

    bool FindSubgraph(GraphType graph) {

        bool subGraphFound = false;
        for (const auto & root : _graph._vertices) {
            if (root.second._data.isTerminal) {
                continue;
            }

            auto previousVertex = graph._vertices.begin();
            auto currentVertex = graph._vertices.begin();

            // vector of found vertices?

            std::vector<VertexId> foundVertices;

            auto matchVertex = [&](auto& self, VertexId matchingVertex) -> bool {

                auto matchingData = _graph._vertices.at(matchingVertex);

                // Compare if both vertices are matching in being terminal or not
                if (matchingData._data.isTerminal != currentVertex->second._data.isTerminal) {
                    return false;
                }

                // Compare by user comparison function (e.g. could be checking for room type)
                if (!_vertexComparisonFunction(matchingData._data, currentVertex->second._data)) {
                    return false;
                }

                // The vertices match
                // Now check for children

                // No further connected vertices
                if (currentVertex->second._edges.size() <= 1) {
                    return true;
                }

                // The matching vertex should have at least the same amount of connecting vertices
                if (matchingData._edges.size() < currentVertex->second._edges.size()) {
                    return false;
                }

                // short list of possible matches, removing already found edges
                std::vector<EdgeId> possibleEdges = matchingData._edges;
                size_t foundChildren = 0;
                for (auto edge : currentVertex->second._edges)
                {
                    const auto& edgeData = graph._edges.at(edge);
                    if (edgeData._v1 == previousVertex->first || edgeData._v2 == previousVertex->first) {
                        continue;
                    }

                    for (auto matchingEdge : matchingData._edges) {
                        const auto& matchingEdgeData = _graph._edges.at(matchingEdge);

                        VertexId otherVertex = matchingVertex == matchingEdgeData._v1 ? matchingEdgeData._v2 : matchingEdgeData._v1;

                        // Match child
                        if (self(self, otherVertex)) {
                            foundChildren++;
                            possibleEdges.erase(matchingEdge);
                        }
                    }
                }

                return foundChildren == currentVertex->second._edges.size();
            };

            if (matchVertex(matchVertex, root)) {
                subGraphFound = true;
                break;
            }
        }

        return subGraphFound;
    }

private:
    GraphType _graph;
    std::vector<VertexId> _nonTerminalVertices;

    std::function<bool(const vertexType&, const vertexType&)> _vertexComparisonFunction = []([[maybe_unused]]const vertexType& lh,[[maybe_unused]] const vertexType& rh) { return true; };
};
