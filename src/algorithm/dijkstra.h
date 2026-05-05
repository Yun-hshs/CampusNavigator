#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include <QVector>
#include <QHash>
#include "graph.h"

class Dijkstra {
public:
    struct Result {
        QVector<int> path;        // 节点ID路径
        double totalDistance = 0; // 实际距离 (米)
        double totalWeight = 0;   // 加权距离
        bool found = false;
    };

    static Result findShortestPath(const Graph &graph, int start, int end);
};

#endif // DIJKSTRA_H