#include "roaditem.h"
#include <QPainterPath>

QPen RoadItem::penForType(const QString& type) {
    QPen pen;
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    if (type == "main") {
        pen.setColor(QColor(100, 100, 105));
        pen.setWidthF(7.0);
    } else if (type == "sub") {
        pen.setColor(QColor(145, 145, 150));
        pen.setWidthF(5.0);
    } else {
        pen.setColor(QColor(175, 175, 180));
        pen.setWidthF(3.0);
    }
    return pen;
}

RoadItem::RoadItem(const QVector<QPointF>& points, const QString& roadType,
                   const QString& name, QGraphicsItem* parent)
    : QGraphicsPathItem(parent), m_name(name)
{
    if (points.size() >= 2) {
        QPainterPath pp;
        pp.moveTo(points[0]);
        for (int i = 1; i < points.size(); ++i)
            pp.lineTo(points[i]);
        setPath(pp);
    }
    m_normalPen = penForType(roadType);
    setPen(m_normalPen);
    setBrush(Qt::NoBrush);
    setZValue(-2);
}

void RoadItem::setPathHighlight(bool on) {
    QPen p = m_normalPen;
    if (on) {
        p.setColor(QColor(240, 40, 40));
        p.setWidthF(p.widthF() + 3);
        setZValue(-1);
    }
    setPen(p);
}

void RoadItem::resetHighlight() {
    setPen(m_normalPen);
    setZValue(-2);
}
