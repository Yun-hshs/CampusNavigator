#include "PathVisualizer.h"
#include "MapView.h"
#include "graph/Graph.h"

#include <QGraphicsScene>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QPainterPath>
#include <QTimer>
#include <cmath>

// ── AnimBall — glowing animated ball ─────────────────────────────────────

AnimBall::AnimBall(qreal radius, const QColor& color, QGraphicsItem* parent)
    : QGraphicsObject(parent), m_radius(radius), m_color(color)
{
    setZValue(200);
}

QRectF AnimBall::boundingRect() const {
    return {-m_radius * 2.5, -m_radius * 2.5,
            m_radius * 5, m_radius * 5};
}

void AnimBall::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    painter->setRenderHint(QPainter::Antialiasing);

    QRadialGradient glow(0, 0, m_radius * 2.5);
    glow.setColorAt(0.0, QColor(255, 120, 50, 160));
    glow.setColorAt(0.3, QColor(255, 90, 30, 80));
    glow.setColorAt(0.7, QColor(255, 60, 20, 20));
    glow.setColorAt(1.0, QColor(255, 50, 10, 0));
    painter->setPen(Qt::NoPen);
    painter->setBrush(glow);
    painter->drawEllipse(QPointF(0, 0), m_radius * 2.5, m_radius * 2.5);

    QRadialGradient fill(-m_radius * 0.3, -m_radius * 0.3, m_radius * 1.2);
    fill.setColorAt(0.0, m_color.lighter(170));
    fill.setColorAt(0.5, m_color);
    fill.setColorAt(1.0, m_color.darker(140));
    painter->setBrush(fill);
    painter->setPen(QPen(m_color.darker(160), 1.5));
    painter->drawEllipse(QPointF(0, 0), m_radius, m_radius);

    painter->setBrush(QColor(255, 255, 255, 140));
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(QPointF(-m_radius * 0.25, -m_radius * 0.3),
                         m_radius * 0.28, m_radius * 0.18);
}

// ── bezier helper (shared with RoadRenderer) ─────────────────────────────

static QPointF bezierControlPoint(const Node& a, const Node& b,
                                  double weight, double maxWeight) {
    double mx = (a.x + b.x) / 2.0;
    double my = (a.y + b.y) / 2.0;
    double dx = b.x - a.x;
    double dy = b.y - a.y;
    double len = std::sqrt(dx * dx + dy * dy);
    if (len < 1e-6) return QPointF(mx, my);

    double nx = -dy / len;
    double ny =  dx / len;
    double curvature = 12.0 * (1.0 - 0.7 * weight / maxWeight);
    double sign = ((a.id + b.id) % 2 == 0) ? 1.0 : -1.0;

    return QPointF(mx + nx * curvature * sign,
                   my + ny * curvature * sign);
}

// ── PathVisualizer ───────────────────────────────────────────────────────

PathVisualizer::PathVisualizer(QObject* parent)
    : QObject(parent)
{
    m_antsTimer = new QTimer(this);
    m_antsTimer->setInterval(60);
    connect(m_antsTimer, &QTimer::timeout, this, [this]() {
        if (m_antsItem) {
            m_antsOffset += 1.5;
            QPen p = m_antsItem->pen();
            p.setDashOffset(m_antsOffset);
            m_antsItem->setPen(p);
        }
    });
}

void PathVisualizer::drawPath(const RenderContext& ctx,
                              const QVector<int>& path,
                              qreal ballRadius) {
    Q_UNUSED(ballRadius);
    if (path.size() < 2 || !ctx.graph) return;

    double maxW = 0;
    for (int i = 0; i + 1 < path.size(); ++i) {
        for (const auto& e : ctx.graph->getEdges(path[i])) {
            if (e.to == path[i + 1]) { maxW = qMax(maxW, e.weight); break; }
        }
    }
    if (maxW < 1) maxW = 1;

    QPainterPath isoPath;
    QPointF p0 = ctx.iso(ctx.graph->node(path.first()).x,
                         ctx.graph->node(path.first()).y);
    isoPath.moveTo(p0);

    for (int i = 1; i < path.size(); ++i) {
        const Node& cur = ctx.graph->node(path[i]);
        QPointF pi = ctx.iso(cur.x, cur.y);

        if (i + 1 < path.size()) {
            double w = 100;
            for (const auto& e : ctx.graph->getEdges(path[i - 1])) {
                if (e.to == path[i]) { w = e.weight; break; }
            }
            const Node& prev = ctx.graph->node(path[i - 1]);
            QPointF cpLogic = bezierControlPoint(prev, cur, w, maxW);
            QPointF cp = ctx.iso(cpLogic.x(), cpLogic.y());
            isoPath.quadTo(cp, pi);
        } else {
            isoPath.lineTo(pi);
        }
        p0 = pi;
    }

    // Layer 0: aura
    QPainterPathStroker auraStroker;
    auraStroker.setWidth(22);
    auraStroker.setCapStyle(Qt::RoundCap);
    QPainterPath auraPath = auraStroker.createStroke(isoPath);
    QLinearGradient auraGrad(auraPath.boundingRect().topLeft(),
                             auraPath.boundingRect().bottomRight());
    auraGrad.setColorAt(0.0, QColor(255, 120, 50, 25));
    auraGrad.setColorAt(0.5, QColor(255, 80, 30, 45));
    auraGrad.setColorAt(1.0, QColor(255, 120, 50, 25));
    auto* l0 = ctx.scene->addPath(auraPath, Qt::NoPen, QBrush(auraGrad));
    l0->setZValue(28); m_pathItems.append(l0);

    // Layer 1: shadow
    QPen sp(QColor(200, 50, 30, 40), 14);
    sp.setCapStyle(Qt::RoundCap); sp.setJoinStyle(Qt::RoundJoin);
    auto* l1 = ctx.scene->addPath(isoPath, sp);
    l1->setZValue(30); m_pathItems.append(l1);

    // Layer 2: outer glow
    QPen gp(QColor(255, 100, 60, 70), 10);
    gp.setCapStyle(Qt::RoundCap); gp.setJoinStyle(Qt::RoundJoin);
    auto* l2 = ctx.scene->addPath(isoPath, gp);
    l2->setZValue(31); m_pathItems.append(l2);

    // Layer 3: main
    QPen mp(QColor(230, 50, 30), 5);
    mp.setCapStyle(Qt::RoundCap); mp.setJoinStyle(Qt::RoundJoin);
    auto* l3 = ctx.scene->addPath(isoPath, mp);
    l3->setZValue(32); m_pathItems.append(l3);

    // Layer 4: center highlight
    QPen cp(QColor(255, 180, 100, 140), 2);
    cp.setCapStyle(Qt::RoundCap);
    auto* l4 = ctx.scene->addPath(isoPath, cp);
    l4->setZValue(33); m_pathItems.append(l4);

    // Marching ants
    QPainterPathStroker antsStroker;
    antsStroker.setWidth(18);
    antsStroker.setCapStyle(Qt::RoundCap);
    QPainterPath antsOutline = antsStroker.createStroke(isoPath);

    QPen antsPen(QColor(255, 255, 255, 160), 1.5, Qt::CustomDashLine);
    antsPen.setDashPattern({8, 10});
    antsPen.setDashOffset(0);
    m_antsItem = ctx.scene->addPath(antsOutline, antsPen);
    m_antsItem->setZValue(34);
    m_pathItems.append(m_antsItem);
    m_antsOffset = 0;
    m_antsTimer->start();

    // Non-focus mask
    QRectF sceneR = ctx.scene->sceneRect();
    QPainterPath maskPath;
    maskPath.addRect(sceneR);

    QPainterPathStroker maskStroker;
    maskStroker.setWidth(60);
    maskStroker.setCapStyle(Qt::RoundCap);
    QPainterPath pathZone = maskStroker.createStroke(isoPath);

    QPainterPath maskWithHole = maskPath.subtracted(pathZone);

    m_maskItem = ctx.scene->addPath(
        maskWithHole, Qt::NoPen,
        QBrush(QColor(0, 0, 0, 100)));
    m_maskItem->setZValue(25);
    m_pathItems.append(m_maskItem);

    for (int id : path) m_pathIds.append(id);
}

void PathVisualizer::animatePath(const RenderContext& ctx,
                                 const QVector<int>& path,
                                 qreal ballRadius) {
    if (path.size() < 2 || !ctx.graph) return;
    if (m_animGroup) { m_animGroup->stop(); delete m_animGroup; m_animGroup = nullptr; }
    if (m_ball) { ctx.scene->removeItem(m_ball); delete m_ball; m_ball = nullptr; }

    QPointF startPos = ctx.iso(ctx.graph->node(path.first()).x,
                               ctx.graph->node(path.first()).y);
    m_ball = new AnimBall(ballRadius, QColor(255, 90, 30));
    m_ball->setPos(startPos);
    ctx.scene->addItem(m_ball);

    QVector<QPointF> waypoints;
    for (int i = 0; i + 1 < path.size(); ++i) {
        const Node& a = ctx.graph->node(path[i]);
        const Node& b = ctx.graph->node(path[i + 1]);
        QPointF pa = ctx.iso(a.x, a.y);
        QPointF pb = ctx.iso(b.x, b.y);

        double w = 100;
        for (const auto& e : ctx.graph->getEdges(path[i])) {
            if (e.to == path[i + 1]) { w = e.weight; break; }
        }
        double maxW = 1;
        for (int j = 0; j + 1 < path.size(); ++j)
            for (const auto& e : ctx.graph->getEdges(path[j]))
                if (e.to == path[j + 1]) maxW = qMax(maxW, e.weight);

        QPointF cpLogic = bezierControlPoint(a, b, w, maxW);
        QPointF cp = ctx.iso(cpLogic.x(), cpLogic.y());

        int samples = 10;
        for (int s = 1; s <= samples; ++s) {
            qreal t = static_cast<qreal>(s) / samples;
            qreal u = 1.0 - t;
            QPointF pt = u * u * pa + 2 * u * t * cp + t * t * pb;
            waypoints.append(pt);
        }
    }

    m_animGroup = new QSequentialAnimationGroup(this);
    for (int i = 0; i < waypoints.size(); ++i) {
        QPointF from = (i == 0) ? startPos : waypoints[i - 1];
        QPointF to = waypoints[i];
        qreal dist = std::sqrt(std::pow(to.x() - from.x(), 2) +
                               std::pow(to.y() - from.y(), 2));
        auto* seg = new QPropertyAnimation(m_ball, "pos");
        seg->setDuration(qBound(30, static_cast<int>(dist * 3), 120));
        seg->setStartValue(from);
        seg->setEndValue(to);
        seg->setEasingCurve(QEasingCurve::Linear);
        m_animGroup->addAnimation(seg);
    }
    m_animGroup->start();
}

void PathVisualizer::clearPath(QGraphicsScene* scene,
                               const QMap<int, IsometricBuilding*>& buildings) {
    if (m_antsTimer) { m_antsTimer->stop(); }
    m_antsItem = nullptr;

    if (m_animGroup) { m_animGroup->stop(); delete m_animGroup; m_animGroup = nullptr; }
    if (m_ball) { scene->removeItem(m_ball); delete m_ball; m_ball = nullptr; }
    for (auto* item : m_pathItems) { scene->removeItem(item); delete item; }
    m_pathItems.clear();
    m_maskItem = nullptr;
    for (int id : m_pathIds)
        if (auto* b = buildings.value(id)) b->setHighlighted(false);
    m_pathIds.clear();
    if (m_startNodeId >= 0) {
        if (auto* b = buildings.value(m_startNodeId)) b->setHighlighted(false);
        m_startNodeId = -1;
    }
    if (m_endNodeId >= 0) {
        if (auto* b = buildings.value(m_endNodeId)) b->setHighlighted(false);
        m_endNodeId = -1;
    }
}
