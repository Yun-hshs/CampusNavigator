#ifndef SPATIALINDEX_H
#define SPATIALINDEX_H

#include <QPointF>
#include <QVector>
#include <QHash>

class SpatialIndex {
public:
    void insert(int id, const QPointF &coord);
    void clear();

    // 返回中心点 radius 半径范围内的所有对象ID
    QVector<int> searchNearby(const QPointF &center, double radius) const;

    // 返回最近的N个对象ID
    QVector<int> searchNearest(const QPointF &center, int count) const;

private:
    QHash<int, QPointF> m_items;

    static double distance(const QPointF &a, const QPointF &b);
};

#endif // SPATIALINDEX_H