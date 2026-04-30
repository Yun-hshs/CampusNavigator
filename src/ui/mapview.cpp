#include "mapview.h"
#include "mapscene.h"
#include <QWheelEvent>
#include <QShowEvent>
#include <QScrollBar>

MapView::MapView(QWidget* parent)
    : QGraphicsView(parent)
{
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameStyle(QFrame::NoFrame);
    setStyleSheet("QGraphicsView { border: none; background-color: #E8F0F8; }");
}

void MapView::setMapScene(MapScene* scene) {
    setScene(scene);
}

void MapView::fitMap() {
    if (scene())
        fitInView(scene()->sceneRect().adjusted(-20, -20, 20, 20), Qt::KeepAspectRatio);
}

void MapView::showEvent(QShowEvent* event) {
    QGraphicsView::showEvent(event);
    if (m_firstShow) {
        m_firstShow = false;
        fitMap();
    }
}

void MapView::wheelEvent(QWheelEvent* event) {
    double factor = (event->angleDelta().y() > 0) ? ZOOM_FACTOR : (1.0 / ZOOM_FACTOR);
    double newScale = transform().m11() * factor;
    if (newScale < MIN_SCALE || newScale > MAX_SCALE)
        return;
    scale(factor, factor);
}
