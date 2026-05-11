#pragma once
#include <QGraphicsView>
#include <QGraphicsObject>
#include <QMap>
#include <QPixmap>
#include <QVector>
#include <QTimer>
#include "data/GeoTransform.h"
#include "view/LabelManager.h"
#include "view/RenderContext.h"

class QGraphicsScene;
class Graph;
class PathVisualizer;

// ── 2.5D Isometric Building (pixmap-based) ─────────────────────────────────

class IsometricBuilding : public QGraphicsObject {
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
public:
    explicit IsometricBuilding(int nodeId, const QString& name,
                               const QPixmap& basePixmap,
                               const QPixmap& overlayPixmap,
                               QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override;

    void setHighlighted(bool on, const QColor& color = QColor());
    int nodeId() const { return m_nodeId; }

signals:
    void clicked(int nodeId);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

private:
    int m_nodeId;
    QString m_name;
    QPixmap m_basePixmap;
    QPixmap m_overlayPixmap;
    bool m_highlighted = false;
    QColor m_hlColor;
    bool m_hovered = false;
};

// ── 2D Vector Building (shape-based) ─────────────────────────────────────

class VectorBuilding : public QGraphicsObject {
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
public:
    explicit VectorBuilding(int nodeId, const QString& name,
                            QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override;

    void setHighlighted(bool on, const QColor& color = QColor());
    int nodeId() const { return m_nodeId; }

signals:
    void clicked(int nodeId);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

private:
    int m_nodeId;
    QString m_name;
    bool m_highlighted = false;
    QColor m_hlColor;
    bool m_hovered = false;
};

// ── MapView ──────────────────────────────────────────────────────────────

class MapView : public QGraphicsView {
    Q_OBJECT
public:
    explicit MapView(QWidget* parent = nullptr);

    void setGraph(Graph* graph);
    void drawMap();

    void drawPath(const QVector<int>& path);
    void animatePath(const QVector<int>& path);
    void clearPath();

    void highlightStartEnd(int startId, int endId);
    void centerOnNode(int id);
    void highlightNode(int id, const QColor& color);

    QPointF nodeScreenPos(int id) const;

    void setRenderMode(RenderMode mode);
    RenderMode renderMode() const { return m_renderMode; }

    void zoomIn();
    void zoomOut();
    void fitMap();

    QPointF iso(qreal x, qreal y) const;

signals:
    void nodeClicked(int id);

protected:
    void wheelEvent(QWheelEvent* event) override;

private:
    void setupScene();
    void initPixmaps();
    bool drawRealMapBackground(double minX, double minY, double maxX, double maxY);
    void updateLabelsAndLOD();

    QGraphicsScene* m_scene = nullptr;
    Graph* m_graph = nullptr;

    QMap<int, IsometricBuilding*> m_buildings;
    QMap<int, VectorBuilding*> m_vectorBuildings;
    LabelManager* m_labelMgr = nullptr;
    PathVisualizer* m_pathViz = nullptr;

    QMap<QString, QPixmap> m_pixCache;

    GeoTransform m_geo;
    RenderMode m_renderMode = RenderMode::Isometric;
    double m_logicScale = 0.3;
    static constexpr double ZOOM_FACTOR = 1.15;
    static constexpr double MIN_SCALE   = 0.05;
    static constexpr double MAX_SCALE   = 8.0;
    static constexpr double BALL_RADIUS = 8.0;

    // LOD debounce
    qreal m_lastLabelZoom = -1;
    QTimer* m_lodTimer = nullptr;
};
