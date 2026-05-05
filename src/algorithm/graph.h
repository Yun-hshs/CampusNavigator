#ifndef GRAPH_H
#define GRAPH_H

#include <QVector>
#include <QPointF>
#include <QHash>
#include <limits>

struct EdgeInfo {
    int toNode;
    double distance;      // 边距离 (米)
    double weight;        // 加权距离 = distance * roadTypeFactor
    int roadId;
};

class Graph {
public:
    void addNode(int id, const QPointF &coord, int buildingId = -1, bool isEntrance = false);
    void addEdge(int from, int to, double distance, double weight, int roadId);
    void removeNode(int nodeId);
    void clear();

    const QVector<EdgeInfo> &edgesFrom(int nodeId) const;
    QPointF nodeCoord(int nodeId) const;
    int nodeBuildingId(int nodeId) const;
    bool nodeIsEntrance(int nodeId) const;
    bool hasNode(int nodeId) const;
    QList<int> allNodeIds() const;
    int nodeCount() const;

private:
    QHash<int, QPointF> m_coords;
    QHash<int, int> m_buildingIds;
    QHash<int, bool> m_entrances;
    QHash<int, QVector<EdgeInfo>> m_adjacency;
};

#endif // GRAPH_H