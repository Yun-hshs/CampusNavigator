#pragma once
#include <QGraphicsView>
#include <QGraphicsObject>
#include <QGraphicsDropShadowEffect>
#include <QMap>

class QGraphicsScene;
class QGraphicsPathItem;
class QSequentialAnimationGroup;
class Graph;

// ── 3D Isometric Building ────────────────────────────────────────────────

class IsometricBuilding : public QGraphicsObject {
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
public:
    explicit IsometricBuilding(int nodeId, const QString& name,
                               qreal w, qreal h, qreal depth,
                               const QColor& wallColor,
                               const QColor& roofColor,
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
    void drawWindows(QPainter* painter, const QPolygonF& face,
                     int rows, int cols, bool isLeft);
    void drawRoofEdge(QPainter* painter);

    int m_nodeId;
    QString m_name;
    qreal m_w, m_h, m_depth;
    QColor m_wallColor, m_roofColor;
    bool m_highlighted = false;
    QColor m_hlColor;
    bool m_hovered = false;
};

// ── Isometric Tree ───────────────────────────────────────────────────────

class IsometricTree : public QGraphicsObject {
public:
    explicit IsometricTree(qreal size, int style = 0,
                           QGraphicsItem* parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;
private:
    qreal m_size;
    int m_style;
};

// ── Animated ball ────────────────────────────────────────────────────────

class AnimBall : public QGraphicsObject {
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
public:
    explicit AnimBall(qreal radius, const QColor& color,
                      QGraphicsItem* parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem*,
               QWidget*) override;
private:
    qreal  m_radius;
    QColor m_color;
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

    void zoomIn();
    void zoomOut();
    void fitMap();

signals:
    void nodeClicked(int id);

protected:
    void wheelEvent(QWheelEvent* event) override;

private:
    void setupScene();

    // Isometric helpers
    QPointF iso(qreal x, qreal y) const;
    qreal depthSort(qreal x, qreal y) const;
    void drawGround();
    void drawGrassPatches();
    void drawRoads();
    void drawBuildings();
    void drawTrees();
    void resetBuildingStyle(int id);

    QGraphicsScene* m_scene = nullptr;
    Graph* m_graph = nullptr;

    QMap<int, IsometricBuilding*> m_buildings;

    // Path state
    QVector<int>                  m_pathIds;
    QVector<QGraphicsPathItem*>   m_pathItems;
    int m_startNodeId = -1;
    int m_endNodeId   = -1;
    AnimBall*                   m_ball      = nullptr;
    QSequentialAnimationGroup*  m_animGroup = nullptr;

    // Isometric projection parameters
    static constexpr double SX = 0.866;          // cos(30°)
    static constexpr double SY = 0.5;            // sin(30°)
    static constexpr double SCALE = 0.5;         // overall scale
    static constexpr double ZOOM_FACTOR = 1.15;
    static constexpr double MIN_SCALE   = 0.05;
    static constexpr double MAX_SCALE   = 8.0;
    static constexpr double BALL_RADIUS = 8.0;
};
