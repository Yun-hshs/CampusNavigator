#include "graph.h"
#include <QDebug>

void Graph::addNode(int id, const QPointF &coord, int buildingId, bool isEntrance)
{
    m_coords[id] = coord;
    m_buildingIds[id] = buildingId;
    m_entrances[id] = isEntrance;
    if (!m_adjacency.contains(id)) {
        m_adjacency[id] = QVector<EdgeInfo>();
    }
}

void Graph::addEdge(int from, int to, double distance, double weight, int roadId)
{
    EdgeInfo e{to, distance, weight, roadId};
    m_adjacency[from].append(e);
    // 双向边
    EdgeInfo e2{from, distance, weight, roadId};
    m_adjacency[to].append(e2);
}

void Graph::removeNode(int nodeId)
{
    m_coords.remove(nodeId);
    m_buildingIds.remove(nodeId);
    m_entrances.remove(nodeId);
    m_adjacency.remove(nodeId);
    for (auto it = m_adjacency.begin(); it != m_adjacency.end(); ++it) {
        auto &edges = it.value();
        for (int i = edges.size() - 1; i >= 0; --i) {
            if (edges[i].toNode == nodeId)
                edges.removeAt(i);
        }
    }
}

void Graph::clear()
{
    m_coords.clear();
    m_buildingIds.clear();
    m_entrances.clear();
    m_adjacency.clear();
}

const QVector<EdgeInfo> &Graph::edgesFrom(int nodeId) const
{
    static const QVector<EdgeInfo> empty;
    auto it = m_adjacency.constFind(nodeId);
    return it != m_adjacency.constEnd() ? *it : empty;
}

QPointF Graph::nodeCoord(int nodeId) const
{
    return m_coords.value(nodeId, QPointF());
}

int Graph::nodeBuildingId(int nodeId) const
{
    return m_buildingIds.value(nodeId, -1);
}

bool Graph::nodeIsEntrance(int nodeId) const
{
    return m_entrances.value(nodeId, false);
}

bool Graph::hasNode(int nodeId) const
{
    return m_coords.contains(nodeId);
}

QList<int> Graph::allNodeIds() const
{
    return m_coords.keys();
}

int Graph::nodeCount() const
{
    return m_coords.size();
}