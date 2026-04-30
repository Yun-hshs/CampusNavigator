#include "graph.h"
#include <QtMath>
#include <limits>

static double dist(const QPointF& a, const QPointF& b) {
    return std::sqrt((a.x() - b.x()) * (a.x() - b.x()) + (a.y() - b.y()) * (a.y() - b.y()));
}

static int orientation(const QPointF& p, const QPointF& q, const QPointF& r) {
    double val = (q.y() - p.y()) * (r.x() - q.x()) - (q.x() - p.x()) * (r.y() - q.y());
    if (std::abs(val) < 1e-9) return 0;
    return (val > 0) ? 1 : 2;
}

static bool onSegment(const QPointF& p, const QPointF& q, const QPointF& r) {
    return q.x() <= std::max(p.x(), r.x()) && q.x() >= std::min(p.x(), r.x()) &&
           q.y() <= std::max(p.y(), r.y()) && q.y() >= std::min(p.y(), r.y());
}

static bool segmentsIntersect(const QPointF& p1, const QPointF& q1,
                               const QPointF& p2, const QPointF& q2,
                               QPointF& out) {
    int o1 = orientation(p1, q1, p2);
    int o2 = orientation(p1, q1, q2);
    int o3 = orientation(p2, q2, p1);
    int o4 = orientation(p2, q2, q1);

    if (o1 != o2 && o3 != o4) {
        double a1 = q1.y() - p1.y(), b1 = p1.x() - q1.x();
        double c1 = a1 * p1.x() + b1 * p1.y();
        double a2 = q2.y() - p2.y(), b2 = p2.x() - q2.x();
        double c2 = a2 * p2.x() + b2 * p2.y();
        double det = a1 * b2 - a2 * b1;
        if (std::abs(det) > 1e-9)
            out = QPointF((b2 * c1 - b1 * c2) / det, (a1 * c2 - a2 * c1) / det);
        return true;
    }

    if (o1 == 0 && onSegment(p1, p2, q1)) { out = p2; return true; }
    if (o2 == 0 && onSegment(p1, q2, q1)) { out = q2; return true; }
    if (o3 == 0 && onSegment(p2, p1, q2)) { out = p1; return true; }
    if (o4 == 0 && onSegment(p2, q1, q2)) { out = q1; return true; }

    return false;
}

void Graph::build(const QVector<Building>& buildings, const QVector<Road>& roads) {
    m_buildings = buildings;
    m_roads = roads;
    m_roadSegments.clear();
    m_adj.clear();
    m_adj.resize(buildings.size());
    m_idToIndex.clear();

    for (int i = 0; i < buildings.size(); ++i)
        m_idToIndex[buildings[i].id] = i;

    // Decompose each road into segments
    for (const auto& road : roads) {
        QVector<QPointF> segs;
        for (int i = 1; i < road.path.size(); ++i)
            segs.append(road.path[i - 1]);
        m_roadSegments.append(segs);
    }

    // For each building, find nearest road point and connect to adjacent buildings
    // on the same road. Build edges between buildings that share road proximity.
    struct BProj {
        int roadIdx = -1;
        int segIdx = -1;
        double t = 0;
        QPointF pt;
        double dist = std::numeric_limits<double>::max();
    };

    QVector<BProj> projections(buildings.size());

    for (int bi = 0; bi < buildings.size(); ++bi) {
        QPointF bc(buildings[bi].x + buildings[bi].w / 2,
                   buildings[bi].y + buildings[bi].h / 2);

        BProj& best = projections[bi];

        for (int ri = 0; ri < roads.size(); ++ri) {
            const auto& road = roads[ri];
            for (int si = 0; si + 1 < road.path.size(); ++si) {
                QPointF A = road.path[si];
                QPointF B = road.path[si + 1];
                QPointF AB = B - A;
                double len2 = AB.x() * AB.x() + AB.y() * AB.y();
                double t = ((bc.x() - A.x()) * AB.x() + (bc.y() - A.y()) * AB.y()) / len2;
                t = qBound(0.0, t, 1.0);
                QPointF proj(A.x() + t * AB.x(), A.y() + t * AB.y());
                double d = dist(bc, proj);
                if (d < best.dist) {
                    best.roadIdx = ri;
                    best.segIdx = si;
                    best.t = t;
                    best.pt = proj;
                    best.dist = d;
                }
            }
        }
    }

    // Build adjacency: buildings that project onto the same road get connected
    for (int i = 0; i < buildings.size(); ++i) {
        for (int j = i + 1; j < buildings.size(); ++j) {
            const auto& pi = projections[i];
            const auto& pj = projections[j];

            if (pi.roadIdx < 0 || pj.roadIdx < 0) continue;

            // Same road: connect if they're on the same or adjacent segments
            if (pi.roadIdx == pj.roadIdx) {
                int segLo = qMin(pi.segIdx, pj.segIdx);
                int segHi = qMax(pi.segIdx, pj.segIdx);
                double d = 0;
                for (int s = segLo; s <= segHi; ++s) {
                    double segLen = dist(m_roads[pi.roadIdx].path[s],
                                         m_roads[pi.roadIdx].path[s + 1]);
                    if (s == segLo && s == segHi)
                        d += segLen * std::abs(pj.t - pi.t);
                    else if (s == segLo)
                        d += segLen * (1.0 - pi.t);
                    else if (s == segHi)
                        d += segLen * pj.t;
                    else
                        d += segLen;
                }
                m_adj[i].push_back({j, d, pi.roadIdx, segLo});
                m_adj[j].push_back({i, d, pi.roadIdx, segLo});
            } else {
                // Different roads: check if roads intersect (segment-segment)
                bool connected = false;
                double dViaIntersection = 0;
                int roadA = pi.roadIdx, roadB = pj.roadIdx;

                QPointF intersectionPt;
                for (int sa = 0; sa + 1 < m_roads[roadA].path.size() && !connected; ++sa) {
                    for (int sb = 0; sb + 1 < m_roads[roadB].path.size() && !connected; ++sb) {
                        if (segmentsIntersect(m_roads[roadA].path[sa], m_roads[roadA].path[sa + 1],
                                              m_roads[roadB].path[sb], m_roads[roadB].path[sb + 1],
                                              intersectionPt)) {
                            double dA = dist(pi.pt, intersectionPt) + pi.dist;
                            double dB = dist(pj.pt, intersectionPt) + pj.dist;
                            dViaIntersection = dA + dB;
                            connected = true;
                        }
                    }
                }

                // Fallback: vertex proximity (for near-miss endpoints, tolerance 20px)
                if (!connected) {
                    for (const auto& pa : m_roads[roadA].path) {
                        for (const auto& pb : m_roads[roadB].path) {
                            if (dist(pa, pb) < 20.0) {
                                QPointF mid((pa.x() + pb.x()) / 2, (pa.y() + pb.y()) / 2);
                                double dA = dist(pi.pt, mid) + pi.dist;
                                double dB = dist(pj.pt, mid) + pj.dist;
                                dViaIntersection = dA + dB;
                                connected = true;
                                break;
                            }
                        }
                        if (connected) break;
                    }
                }

                if (connected) {
                    m_adj[i].push_back({j, dViaIntersection, -1, -1});
                    m_adj[j].push_back({i, dViaIntersection, -1, -1});
                }
            }
        }
    }

    // Fallback: if a building is isolated, connect to its nearest neighbor
    for (int i = 0; i < buildings.size(); ++i) {
        if (!m_adj[i].isEmpty()) continue;
        int bestJ = -1;
        double bestD = std::numeric_limits<double>::max();
        QPointF ci(buildings[i].x + buildings[i].w / 2,
                   buildings[i].y + buildings[i].h / 2);
        for (int j = 0; j < buildings.size(); ++j) {
            if (i == j) continue;
            QPointF cj(buildings[j].x + buildings[j].w / 2,
                       buildings[j].y + buildings[j].h / 2);
            double d = dist(ci, cj);
            if (d < bestD) { bestD = d; bestJ = j; }
        }
        if (bestJ >= 0) {
            m_adj[i].push_back({bestJ, bestD, -1, -1});
            m_adj[bestJ].push_back({i, bestD, -1, -1});
        }
    }
}

const Building& Graph::building(int id) const {
    return m_buildings[m_idToIndex.value(id)];
}

int Graph::toIndex(int buildingId) const {
    return m_idToIndex.value(buildingId, -1);
}

int Graph::toId(int index) const {
    return (index >= 0 && index < m_buildings.size()) ? m_buildings[index].id : -1;
}

const QVector<Graph::Edge>& Graph::neighbors(int index) const {
    return m_adj[index];
}
