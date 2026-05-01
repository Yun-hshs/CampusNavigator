#pragma once
#include "graph/Graph.h"
#include <QVector>

class Dijkstra {
public:
    /// Returns the shortest path as a list of node IDs (start → ... → end).
    /// Returns an empty vector if unreachable.
    static QVector<int> findPath(const Graph& graph, int startId, int endId);
};
