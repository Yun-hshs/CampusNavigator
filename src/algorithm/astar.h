#ifndef ASTAR_H
#define ASTAR_H

#include <QVector>
#include <QHash>
#include "graph.h"

class AStar {
public:
    struct Result {
        QVector<int> path;
        double totalDistance = 0;
        double totalWeight = 0;
        bool found = false;
    };

    static Result findShortestPath(const Graph &graph, int start, int end);

private:
    static double heuristic(const QPointF &a, const QPointF &b);
};

#endif // ASTAR_H