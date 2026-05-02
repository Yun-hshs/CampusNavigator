#pragma once
#include <QString>
#include <QVector>
#include <QMap>
#include <QPolygonF>

struct Node {
    int id;
    QString name;
    double x;
    double y;
    QString description;
    QString type = "building";  // "building" or "decoration"
};

struct Edge {
    int from;
    int to;
    double weight;
    QString type = "main_road"; // "main_road" or "footpath"
};

struct Area {
    int id;
    QString name;
    QString type;        // "lake", "plaza", "sports_field", "garden"
    QPolygonF polygon;   // 逻辑坐标多边形
    int zOrder = -80;
};

class Graph {
public:
    void addNode(int id, const QString& name, double x, double y,
                 const QString& description = QString(),
                 const QString& type = "building");
    void addEdge(int from, int to, double weight,
                 const QString& type = "main_road");
    void addArea(int id, const QString& name, const QString& type,
                 const QPolygonF& polygon, int zOrder);

    QVector<Edge> getEdges(int nodeId) const;

    int nodeCount() const { return m_nodes.size(); }
    bool hasNode(int id) const { return m_nodes.contains(id); }
    Node node(int id) const { return m_nodes.value(id); }
    QVector<Node> allNodes() const;

    QVector<Area> allAreas() const { return m_areas.values(); }

private:
    QMap<int, Node> m_nodes;
    QMap<int, QVector<Edge>> m_adj;
    QMap<int, Area> m_areas;
};
