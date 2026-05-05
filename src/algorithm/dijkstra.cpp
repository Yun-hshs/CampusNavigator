#include "dijkstra.h"
#include <queue>
#include <limits>
#include <QHash>

using PQEntry = std::pair<double, int>; // (weight, nodeId)

Dijkstra::Result Dijkstra::findShortestPath(const Graph &graph, int start, int end)
{
    Result result;
    if (!graph.hasNode(start) || !graph.hasNode(end)) {
        return result;
    }

    QHash<int, double> dist;
    QHash<int, int> prev;
    for (int nid : graph.allNodeIds()) {
        dist[nid] = std::numeric_limits<double>::infinity();
    }
    dist[start] = 0.0;

    std::priority_queue<PQEntry, std::vector<PQEntry>, std::greater<PQEntry>> pq;
    pq.emplace(0.0, start);

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();

        if (d > dist[u]) continue; // 跳过过期条目
        if (u == end) break;       // 找到终点

        for (const EdgeInfo &e : graph.edgesFrom(u)) {
            double nd = d + e.weight;
            if (nd < dist[e.toNode]) {
                dist[e.toNode] = nd;
                prev[e.toNode] = u;
                pq.emplace(nd, e.toNode);
            }
        }
    }

    if (dist[end] == std::numeric_limits<double>::infinity()) {
        return result; // 不可达
    }

    // 回溯路径
    result.totalWeight = dist[end];
    for (int cur = end; cur != start; cur = prev[cur]) {
        result.path.prepend(cur);
    }
    result.path.prepend(start);

    // 计算实际距离
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