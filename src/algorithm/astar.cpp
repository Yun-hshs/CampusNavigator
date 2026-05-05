#include "astar.h"
#include <queue>
#include <limits>
#include <QHash>
#include <QtMath>

using PQEntry = std::pair<double, int>; // (fScore, nodeId)

double AStar::heuristic(const QPointF &a, const QPointF &b)
{
    double dx = a.x() - b.x();
    double dy = a.y() - b.y();
    return qSqrt(dx * dx + dy * dy);
}

AStar::Result AStar::findShortestPath(const Graph &graph, int start, int end)
{
    Result result;
    if (!graph.hasNode(start) || !graph.hasNode(end)) {
        return result;
    }

    QPointF endCoord = graph.nodeCoord(end);

    QHash<int, double> gScore;
    QHash<int, double> fScore;
    QHash<int, int> prev;

    for (int nid : graph.allNodeIds()) {
        gScore[nid] = std::numeric_limits<double>::infinity();
        fScore[nid] = std::numeric_limits<double>::infinity();
    }
    gScore[start] = 0.0;
    fScore[start] = heuristic(graph.nodeCoord(start), endCoord);

    std::priority_queue<PQEntry, std::vector<PQEntry>, std::greater<PQEntry>> pq;
    pq.emplace(fScore[start], start);

    QHash<int, bool> closed;

    while (!pq.empty()) {
        auto [f, u] = pq.top();
        pq.pop();

        if (closed[u]) continue;
        closed[u] = true;

        if (u == end) break;

        for (const EdgeInfo &e : graph.edgesFrom(u)) {
            if (closed[e.toNode]) continue;

            double tentG = gScore[u] + e.weight;
            if (tentG < gScore[e.toNode]) {
                prev[e.toNode] = u;
                gScore[e.toNode] = tentG;
                fScore[e.toNode] = tentG + heuristic(graph.nodeCoord(e.toNode), endCoord);
                pq.emplace(fScore[e.toNode], e.toNode);
            }
        }
    }

    if (gScore[end] == std::numeric_limits<double>::infinity()) {
        return result;
    }

    result.totalWeight = gScore[end];
    for (int cur = end; cur != start; cur = prev[cur]) {
        result.path.prepend(cur);
    }
    result.path.prepend(start);

    result.totalDistance = 0;
    for (int i = 0; i < result.path.size() - 1; ++i) {
        int from = result.path[i], to = result.path[i + 1];
        for (const EdgeInfo &e : graph.edgesFrom(from)) {
            if (e.toNode == to) {
                result.totalDistance += e.distance;
                break;
            }
        }
    }

    result.found = true;
    return result;
}