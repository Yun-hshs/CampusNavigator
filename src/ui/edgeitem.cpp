#include "edgeitem.h"
#include <QPen>

EdgeItem::EdgeItem(int fromId, int toId, double weight,
                   const QLineF& line, QGraphicsItem* parent)
    : QGraphicsLineItem(line, parent)
    , m_fromId(fromId)
    , m_toId(toId)
    , m_weight(weight)
{
    QPen pen(COLOR_NORMAL);
    pen.setWidth(2);
    pen.setCapStyle(Qt::RoundCap);
    setPen(pen);
}

void EdgeItem::setPathHighlight(bool on) {
    QPen p = pen();
    if (on) {
        p.setColor(COLOR_HIGHLIGHT);
        p.setWidth(4);
        setZValue(1);
    } else {
        p.setColor(COLOR_NORMAL);
        p.setWidth(2);
        setZValue(-1);
    }
    setPen(p);
}
