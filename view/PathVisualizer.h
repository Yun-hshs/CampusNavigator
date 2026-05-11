#pragma once
#include "RenderContext.h"
#include <QObject>
#include <QGraphicsObject>
#include <QVector>
#include <QGraphicsPathItem>

class QTimer;
class QSequentialAnimationGroup;
class IsometricBuilding;
class VectorBuilding;

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

// ── PathVisualizer ───────────────────────────────────────────────────────

class PathVisualizer : public QObject {
    Q_OBJECT
public:
    explicit PathVisualizer(QObject* parent = nullptr);

    void drawPath(const RenderContext& ctx, const QVector<int>& path,
                  qreal ballRadius);
    void animatePath(const RenderContext& ctx, const QVector<int>& path,
                     qreal ballRadius);
    void clearPath(QGraphicsScene* scene,
                   const QMap<int, IsometricBuilding*>& isoBuildings,
                   const QMap<int, VectorBuilding*>& vecBuildings);

    bool hasPath() const { return !m_pathIds.isEmpty(); }
    int startNodeId() const { return m_startNodeId; }
    int endNodeId() const { return m_endNodeId; }
    void setStartNodeId(int id) { m_startNodeId = id; }
    void setEndNodeId(int id) { m_endNodeId = id; }

private:
    QVector<QGraphicsPathItem*>  m_pathItems;
    QGraphicsPathItem*           m_maskItem = nullptr;
    QGraphicsPathItem*           m_antsItem = nullptr;
    qreal                        m_antsOffset = 0;
    QTimer*                      m_antsTimer = nullptr;

    AnimBall*                    m_ball = nullptr;
    QSequentialAnimationGroup*   m_animGroup = nullptr;

    QVector<int> m_pathIds;
    int m_startNodeId = -1;
    int m_endNodeId   = -1;
};
