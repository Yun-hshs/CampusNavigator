#include "Graph.h"
#include <QtMath>

void Graph::addNode(int id, const QString& name, double x, double y,
                    const QString& description, const QString& type) {
    m_nodes[id] = {id, name, x, y, description, type};
}

void Graph::addEdge(int from, int to, double weight, const QString& type) {
    m_adj[from].push_back({from, to, weight, type});
    m_adj[to].push_back({to, from, weight, type});
}

void Graph::addArea(int id, const QString& name, const QString& type,
                    const QPolygonF& polygon, int zOrder) {
    m_areas[id] = {id, name, type, polygon, zOrder};
}

QVector<Edge> Graph::getEdges(int nodeId) const {
    return m_adj.value(nodeId);
}

QVector<Node> Graph::allNodes() const {
    return m_nodes.values();
}
