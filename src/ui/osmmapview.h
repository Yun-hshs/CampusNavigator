#pragma once
#include <QGraphicsView>
#include <QHash>
#include <QPixmap>

class OsmMapView : public QGraphicsView {
    Q_OBJECT
public:
    explicit OsmMapView(QGraphicsScene* scene, QWidget* parent = nullptr);

    void setViewCenter(double x, double y);
    void setViewZoom(double factor);

protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    bool m_dragging = false;
    QPoint m_lastMousePos;
};
