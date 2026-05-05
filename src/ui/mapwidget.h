#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QGraphicsView>
#include <QPointF>

class MapWidget : public QGraphicsView {
    Q_OBJECT
public:
    explicit MapWidget(QWidget *parent = nullptr);

    void zoomIn();
    void zoomOut();
    void fitToScene();

    void highlightRoute(const QVector<QPointF> &points);
    void clearRouteHighlight();

    void centerOnPoint(const QPointF &point);

signals:
    void buildingClicked(int buildingId);
    void nodeClicked(int nodeId);
    void pointClicked(const QPointF &scenePos);
    void coordinateDebug(const QPointF &scenePos);
    void mouseMoved(const QPointF &scenePos);

protected:
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void applyZoom(double factor, QPointF cursorPos);

    QGraphicsPathItem *m_routeItem = nullptr;
    double m_zoomFactor = 1.0;

    // Min/max zoom relative to initial fit
    static constexpr double kMinZoom = 0.08;
    static constexpr double kMaxZoom = 12.0;
};

#endif // MAPWIDGET_H