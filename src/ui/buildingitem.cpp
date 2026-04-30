#include "buildingitem.h"
#include <QGraphicsSimpleTextItem>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QCursor>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QPainter>

QColor BuildingItem::colorForType(const QString& type) {
    static const QHash<QString, QColor> map = {
        {"teaching", QColor(70,  130, 210)},
        {"dorm",     QColor(235, 120, 45)},
        {"dining",   QColor(220, 55,  50)},
        {"sports",   QColor(46,  160, 90)},
        {"service",  QColor(28,  170, 150)},
        {"gate",     QColor(140, 150, 160)},
        {"other",    QColor(142, 68,  170)},
    };
    return map.value(type, QColor(70, 130, 210));
}

BuildingItem::BuildingItem(int id, const QString& name, const QString& type,
                           const QString& desc, double x, double y, double w, double h,
                           QGraphicsItem* parent)
    : QGraphicsRectItem(x, y, w, h, parent)
    , m_id(id), m_name(name), m_type(type), m_desc(desc)
    , m_fillColor(colorForType(type))
{
    setAcceptHoverEvents(true);
    setFlags(QGraphicsItem::ItemIsSelectable);
    setPen(QPen(m_fillColor.darker(130), 2));
    setBrush(QBrush(m_fillColor.lighter(110)));
    setZValue(0);

    m_label = new QGraphicsSimpleTextItem(m_name, this);
    QFont f("Microsoft YaHei", 9);
    f.setBold(true);
    m_label->setFont(f);
    m_label->setBrush(Qt::white);
    m_label->setPen(QPen(QColor(0, 0, 0, 60), 1));
    m_label->setZValue(1);

    QString tip = m_desc.isEmpty() ? m_name : m_name + "\n" + m_desc;
    setToolTip(tip);

    updateLabelPos();
}

void BuildingItem::updateLabelPos() {
    QRectF r  = rect();
    QRectF lr = m_label->boundingRect();
    m_label->setPos(r.x() + (r.width()  - lr.width())  / 2,
                    r.y() + (r.height() - lr.height()) / 2);
}

void BuildingItem::setHighlighted(bool on) {
    m_highlighted = on;
    if (on) {
        setPen(QPen(QColor(255, 60, 30), 3.5));
        setBrush(QBrush(m_fillColor));
        setZValue(10);
    } else {
        setPen(QPen(m_fillColor.darker(130), 2));
        setBrush(QBrush(m_fillColor.lighter(110)));
        setZValue(0);
    }
}

void BuildingItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
    Q_UNUSED(event);
    if (!m_highlighted) {
        setPen(QPen(QColor(255, 180, 40), 3));
        setBrush(QBrush(m_fillColor.lighter(140)));
    }
    setCursor(QCursor(Qt::PointingHandCursor));
}

void BuildingItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
    Q_UNUSED(event);
    if (!m_highlighted) {
        setPen(QPen(m_fillColor.darker(130), 2));
        setBrush(QBrush(m_fillColor.lighter(110)));
    }
    setCursor(QCursor(Qt::ArrowCursor));
}

void BuildingItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    QGraphicsRectItem::mousePressEvent(event);
}
