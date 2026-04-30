#pragma once
#include "graph.h"
#include <QVector>
#include <QPointF>

class Dijkstra {
public:
    struct Result {
        QVector<int> path;            // building IDs along the path
        QVector<QPointF> pathPoints;  // road-level polyline for rendering
        double totalWeight = 0;
        bool reachable = false;
    };

    static Result findPath(const Graph& graph, int startBuildingId, int endBuildingId);
};
