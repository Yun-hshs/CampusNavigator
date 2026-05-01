#include "Graph.h"
#include <QtMath>

void Graph::addNode(int id, const QString& name, double x, double y) {
    m_nodes[id] = {id, name, x, y};
}

void Graph::addEdge(int from, int to, double weight) {
    m_adj[from].push_back({from, to, weight});
    m_adj[to].push_back({to, from, weight});
}

QVector<Edge> Graph::getEdges(int nodeId) const {
    return m_adj.value(nodeId);
}

QVector<Node> Graph::allNodes() const {
    return m_nodes.values();
}
