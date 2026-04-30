#include "osmmapview.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPainter>
#include <QScrollBar>
#include <QtMath>

OsmMapView::OsmMapView(QGraphicsScene* scene, QWidget* parent)
    : QGraphicsView(scene, parent)
{
    setRenderHint(QPainter::Antialiasing, true);
    setOptimizationFlag(QGraphicsView::DontSavePainterState, true);
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameStyle(QFrame::NoFrame);
    setBackgroundBrush(QColor(232, 240, 248));
    setDragMode(QGraphicsView::NoDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);
}

void OsmMapView::setViewCenter(double x, double y) {
    centerOn(x, y);
}

void OsmMapView::setViewZoom(double factor) {
    scale(factor, factor);
}

void OsmMapView::drawBackground(QPainter* painter, const QRectF& rect) {
    painter->fillRect(rect, QColor(232, 240, 248));

    // Draw subtle grid
    painter->setPen(QPen(QColor(200, 210, 220), 0.5));
    double step = 50;
    double x0 = qFloor(rect.left()   / step) * step;
    double y0 = qFloor(rect.top()    / step) * step;
    for (double x = x0; x <= rect.right(); x += step)
        painter->drawLine(QPointF(x, rect.top()), QPointF(x, rect.bottom()));
    for (double y = y0; y <= rect.bottom(); y += step)
        painter->drawLine(QPointF(rect.left(), y), QPointF(rect.right(), y));
}

void OsmMapView::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_lastMousePos = event->pos();
        setCursor(Qt::ClosedHandCursor);
    }
    QGraphicsView::mousePressEvent(event);
}

void OsmMapView::mouseMoveEvent(QMouseEvent* event) {
    if (m_dragging) {
        QPoint delta = event->pos() - m_lastMousePos;
        m_lastMousePos = event->pos();
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
        return;
    }
    QGraphicsView::mouseMoveEvent(event);
}

void OsmMapView::mouseReleaseEvent(QMouseEvent* event) {
    m_dragging = false;
    setCursor(Qt::ArrowCursor);
    QGraphicsView::mouseReleaseEvent(event);
}

void OsmMapView::wheelEvent(QWheelEvent* event) {
    double factor = event->angleDelta().y() > 0 ? 1.15 : 1.0 / 1.15;
    scale(factor, factor);
}
