#pragma once
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QPen>
#include <QString>

class RoadItem : public QGraphicsPathItem {
public:
    RoadItem(const QVector<QPointF>& points, const QString& roadType,
             const QString& name, QGraphicsItem* parent = nullptr);

    QString roadName() const { return m_name; }

    void setPathHighlight(bool on);
    void resetHighlight();

private:
    QString m_name;
    QPen m_normalPen;
    static QPen penForType(const QString& type);
};
