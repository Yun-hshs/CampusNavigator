#include "mapwidget.h"
#include <QWheelEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include <QGraphicsScene>
#include <QGraphicsPathItem>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsPolygonItem>
#include <QPainterPath>
#include <QtMath>

MapWidget::MapWidget(QWidget *parent)
    : QGraphicsView(parent)
{
    setDragMode(QGraphicsView::ScrollHandDrag);
    setRenderHint(QPainter::Antialiasing);
    setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    setTransformationAnchor(QGraphicsView::NoAnchor);
    setResizeAnchor(QGraphicsView::NoAnchor);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameStyle(QFrame::NoFrame);
}

// ---- public zoom / navigation ----

void MapWidget::zoomIn()
{
    QPointF center = viewport()->rect().center();
    applyZoom(1.25, center);
}

void MapWidget::zoomOut()
{
    QPointF center = viewport()->rect().center();
    applyZoom(1.0 / 1.25, center);
}

void MapWidget::fitToScene()
{
    if (scene()) {
        fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
        m_zoomFactor = transform().m11();
    }
}

void MapWidget::centerOnPoint(const QPointF &point)
{
    centerOn(point);
}

// ---- route highlight ----

void MapWidget::highlightRoute(const QVector<QPointF> &points)
{
    clearRouteHighlight();
    if (points.size() < 2) return;

    QPainterPath path;
    path.moveTo(points.first());
    for (int i = 1; i < points.size(); ++i)
        path.lineTo(points[i]);

    m_routeItem = scene()->addPath(path,
        QPen(QColor(255, 80, 0), 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    m_routeItem->setZValue(50);
}

void MapWidget::clearRouteHighlight()
{
    if (m_routeItem) {
        scene()->removeItem(m_routeItem);
        delete m_routeItem;
        m_routeItem = nullptr;
    }
}

// ---- event overrides ----

void MapWidget::wheelEvent(QWheelEvent *event)
{
    double angle = event->angleDelta().y();
    double factor = (angle > 0) ? 1.12 : 1.0 / 1.12;

    // Clamp to zoom limits
    double newZoom = m_zoomFactor * factor;
    if (newZoom < kMinZoom || newZoom > kMaxZoom)
        return;

    applyZoom(factor, event->position());
    event->accept();
}

void MapWidget::mousePressEvent(QMouseEvent *event)
{
    QGraphicsView::mousePressEvent(event);

    if (event->button() == Qt::LeftButton && !event->isAccepted())
        return;

    if (event->button() == Qt::LeftButton) {
        QPointF scenePos = mapToScene(event->pos());

        // Debug: always print coordinates on left click
        qDebug().noquote()
            << QStringLiteral("[MAP-CLICK] 坐标: (%1, %2)")
                   .arg(int(scenePos.x()))
                   .arg(int(scenePos.y()));
        emit coordinateDebug(scenePos);

        QList<QGraphicsItem *> items = scene()->items(scenePos);

        for (QGraphicsItem *item : items) {
            if (!item->data(0).isValid())
                continue;

            int id = item->data(0).toInt();
            if (dynamic_cast<QGraphicsEllipseItem *>(item)) {
                emit nodeClicked(id);
                return;
            }
            if (dynamic_cast<QGraphicsRectItem *>(item) ||
                dynamic_cast<QGraphicsPolygonItem *>(item)) {
                emit buildingClicked(id);
                return;
            }
        }

        emit pointClicked(scenePos);
    }
}

void MapWidget::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsView::mouseMoveEvent(event);
    emit mouseMoved(mapToScene(event->pos()));
}

void MapWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);
}

// ---- private helpers ----

void MapWidget::applyZoom(double factor, QPointF cursorPos)
{
    // Scene point under cursor before zoom
    QPointF sceneAnchor = mapToScene(cursorPos.toPoint());

    // Apply scale
    scale(factor, factor);
    m_zoomFactor *= factor;

    // Scene point under cursor after zoom
    QPointF newAnchor = mapToScene(cursorPos.toPoint());

    // Drift correction — keep the original scene point under the cursor
    QPointF drift = newAnchor - sceneAnchor;
    translate(drift.x(), drift.y());
}
