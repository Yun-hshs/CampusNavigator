#include "navigator.h"
#include <queue>
#include <limits>
#include <algorithm>

std::vector<int> Navigator::dijkstra(const Graph& graph, int start, int end) {
    int n = graph.size();
    std::vector<double> dist(n, std::numeric_limits<double>::max());
    std::vector<int> prev(n, -1);

    dist[start] = 0;

    using P = std::pair<double, int>;
    std::priority_queue<P, std::vector<P>, std::greater<P>> pq;
    pq.push({0, start});

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();

        if (d > dist[u]) continue;

        for (auto& e : graph.getNeighbors(u)) {
            if (dist[u] + e.weight < dist[e.to]) {
                dist[e.to] = dist[u] + e.weight;
                prev[e.to] = u;
                pq.push({dist[e.to], e.to});
            }
        }
    }

    std::vector<int> path;
    for (int at = end; at != -1; at = prev[at]) {
        path.push_back(at);
    }

    std::reverse(path.begin(), path.end());
    return path;
}