#include "spatialindex.h"
#include <QtMath>
#include <algorithm>

void SpatialIndex::insert(int id, const QPointF &coord)
{
    m_items[id] = coord;
}

void SpatialIndex::clear()
{
    m_items.clear();
}

QVector<int> SpatialIndex::searchNearby(const QPointF &center, double radius) const
{
    QVector<int> result;
    for (auto it = m_items.constBegin(); it != m_items.constEnd(); ++it) {
        if (distance(center, it.value()) <= radius) {
            result.append(it.key());
        }
    }
    return result;
}

QVector<int> SpatialIndex::searchNearest(const QPointF &center, int count) const
{
    QVector<std::pair<double, int>> sorted;
    for (auto it = m_items.constBegin(); it != m_items.constEnd(); ++it) {
        sorted.append({distance(center, it.value()), it.key()});
    }
    std::sort(sorted.begin(), sorted.end());

    QVector<int> result;
    for (int i = 0; i < std::min(count, static_cast<int>(sorted.size())); ++i) {
        result.append(sorted[i].second);
    }
    return result;
}

double SpatialIndex::distance(const QPointF &a, const QPointF &b)
{
    double dx = a.x() - b.x();
    double dy = a.y() - b.y();
    return qSqrt(dx * dx + dy * dy);
}