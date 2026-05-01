#pragma once
#include <QString>
#include <QVector>
#include <QMap>

struct Node {
    int id;
    QString name;
    double x;
    double y;
};

struct Edge {
    int from;
    int to;
    double weight;
};

class Graph {
public:
    void addNode(int id, const QString& name, double x, double y);
    void addEdge(int from, int to, double weight);

    QVector<Edge> getEdges(int nodeId) const;

    int nodeCount() const { return m_nodes.size(); }
    bool hasNode(int id) const { return m_nodes.contains(id); }
    Node node(int id) const { return m_nodes.value(id); }
    QVector<Node> allNodes() const;

private:
    QMap<int, Node> m_nodes;
    QMap<int, QVector<Edge>> m_adj;
};
