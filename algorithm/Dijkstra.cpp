#include "Dijkstra.h"
#include <QMap>
#include <QSet>
#include <queue>
#include <limits>
#include <algorithm>

QVector<int> Dijkstra::findPath(const Graph& graph, int startId, int endId) {
    if (!graph.hasNode(startId) || !graph.hasNode(endId))
        return {};

    QMap<int, double> dist;
    QMap<int, int> prev;
    QSet<int> visited;

    for (const auto& node : graph.allNodes()) {
        dist[node.id] = std::numeric_limits<double>::max();
    }
    dist[startId] = 0;

    using P = std::pair<double, int>;
    std::priority_queue<P, std::vector<P>, std::greater<P>> pq;
    pq.push({0, startId});

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();

        if (visited.contains(u))
            continue;
        visited.insert(u);

        if (u == endId)
            break;

        for (const auto& e : graph.getEdges(u)) {
            if (visited.contains(e.to))
                continue;
            double nd = dist[u] + e.weight;
            if (nd < dist[e.to]) {
                dist[e.to] = nd;
                prev[e.to] = u;
                pq.push({nd, e.to});
            }
        }
    }

    // Unreachable
    if (dist[endId] == std::numeric_limits<double>::max())
        return {};

    // Reconstruct path
    QVector<int> path;
    for (int at = endId; at != startId; at = prev[at])
        path.push_back(at);
    path.push_back(startId);
    std::reverse(path.begin(), path.end());

    return path;
}
