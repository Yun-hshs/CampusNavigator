#include "dijkstra.h"
#include <queue>
#include <limits>
#include <algorithm>
#include <QtMath>

Dijkstra::Result Dijkstra::findPath(const Graph& graph, int startBuildingId, int endBuildingId) {
    Result result;
    int startIdx = graph.toIndex(startBuildingId);
    int endIdx   = graph.toIndex(endBuildingId);
    if (startIdx < 0 || endIdx < 0) return result;

    int n = graph.buildingCount();
    QVector<double> dist(n, std::numeric_limits<double>::max());
    QVector<int> prev(n, -1);
    QVector<int> prevRoad(n, -1);
    QVector<int> prevSeg(n, -1);
    dist[startIdx] = 0;

    using P = std::pair<double, int>;
    std::priority_queue<P, std::vector<P>, std::greater<P>> pq;
    pq.push({0, startIdx});

    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (d > dist[u]) continue;
        if (u == endIdx) break;

        for (const auto& e : graph.neighbors(u)) {
            double nd = dist[u] + e.weight;
            if (nd < dist[e.to]) {
                dist[e.to] = nd;
                prev[e.to] = u;
                prevRoad[e.to] = e.roadIdx;
                prevSeg[e.to] = e.segIdx;
                pq.push({nd, e.to});
            }
        }
    }

    if (dist[endIdx] == std::numeric_limits<double>::max())
        return result;

    result.reachable = true;
    result.totalWeight = dist[endIdx];

    QVector<int> indices;
    for (int at = endIdx; at != -1; at = prev[at])
        indices.push_back(at);
    std::reverse(indices.begin(), indices.end());

    for (int idx : indices)
        result.path.push_back(graph.toId(idx));

    // Build road-level polyline from the building path
    for (int i = 0; i < indices.size(); ++i) {
        int idx = indices[i];
        const Building& b = graph.buildingAt(idx);
        QPointF center(b.x + b.w / 2, b.y + b.h / 2);
        result.pathPoints.append(center);

        if (i + 1 < indices.size()) {
            int ri = prevRoad[indices[i + 1]];
            int si = prevSeg[indices[i + 1]];
            if (ri >= 0 && si >= 0) {
                const auto& roads = graph.roads();
                if (ri < roads.size()) {
                    int segHi = -1;
                    // Find the segment range for the next building
                    int nextIdx = indices[i + 1];
                    for (const auto& e : graph.neighbors(idx)) {
                        if (e.to == nextIdx && e.roadIdx >= 0) {
                            segHi = e.segIdx;
                            ri = e.roadIdx;
                            break;
                        }
                    }
                    if (segHi >= 0 && ri < roads.size()) {
                        int segLo = qMin(si, segHi);
                        segHi = qMax(si, segHi);
                        for (int s = segLo; s <= segHi && s + 1 < roads[ri].path.size(); ++s)
                            result.pathPoints.append(roads[ri].path[s + 1]);
                    }
                }
            }
        }
    }

    return result;
}
