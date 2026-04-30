#pragma once
#include <QVector>
#include <QHash>
#include <QPointF>
#include <QString>
#include "src/models/building.h"
#include "src/models/road.h"

class Graph {
public:
    struct Edge { int to; double weight; int roadIdx; int segIdx; };

    void build(const QVector<Building>& buildings, const QVector<Road>& roads);

    int buildingCount() const { return m_buildings.size(); }
    const Building& building(int id) const;
    Building buildingAt(int index) const { return m_buildings[index]; }

    int toIndex(int buildingId) const;
    int toId(int index) const;

    const QVector<Edge>& neighbors(int index) const;

    const QVector<Road>& roads() const { return m_roads; }
    const QVector<QVector<QPointF>>& roadSegments() const { return m_roadSegments; }

private:
    QVector<Building> m_buildings;
    QVector<Road> m_roads;
    QVector<QVector<QPointF>> m_roadSegments;  // per-road: list of consecutive point pairs
    QVector<QVector<Edge>> m_adj;
    QHash<int, int> m_idToIndex;
};
