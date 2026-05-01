#include "MapView.h"
#include "graph/Graph.h"

#include <QGraphicsScene>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QWheelEvent>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QtMath>
#include <QSet>
#include <QRandomGenerator>

// ═══════════════════════════════════════════════════════════════════════════
// Building style configuration
// ═══════════════════════════════════════════════════════════════════════════

struct BldgCfg {
    qreal w, h, depth;
    QColor wall, roof, window;
    int winRows, winCols;
};

static BldgCfg cfgFor(const QString& name) {
    // Teaching buildings — tall, blue-gray
    if (name.contains("主教"))
        return {50, 70, 40, QColor(200,210,225), QColor(120,150,185),
                QColor(140,180,220), 5, 4};
    if (name.contains("教学") || name.contains("3栋"))
        return {42, 55, 35, QColor(195,205,220), QColor(115,140,175),
                QColor(135,175,215), 4, 3};
    // Library — wide, distinctive
    if (name.contains("图书馆"))
        return {55, 50, 45, QColor(185,195,210), QColor(100,130,170),
                QColor(130,170,210), 4, 5};
    // Museum
    if (name.contains("博物"))
        return {42, 40, 35, QColor(210,200,185), QColor(170,155,135),
                QColor(180,190,200), 3, 3};
    // Labs
    if (name.contains("实验") || name.contains("实训"))
        return {38, 42, 30, QColor(190,205,195), QColor(120,155,135),
                QColor(150,190,170), 3, 3};
    // Research
    if (name.contains("研究生"))
        return {40, 48, 32, QColor(200,200,215), QColor(130,135,165),
                QColor(155,165,200), 4, 3};
    // Dorms — residential feel
    if (name.contains("公寓") || name.contains("宿舍"))
        return {30, 55, 25, QColor(215,205,190), QColor(175,160,140),
                QColor(185,195,210), 5, 2};
    // Canteen
    if (name.contains("餐厅") || name.contains("食堂"))
        return {38, 22, 30, QColor(215,205,185), QColor(180,165,140),
                QColor(195,200,180), 2, 3};
    // Sports
    if (name.contains("体育") || name.contains("操场") || name.contains("游泳"))
        return {55, 18, 45, QColor(195,210,200), QColor(145,170,155),
                QColor(170,195,180), 1, 4};
    if (name.contains("篮球"))
        return {42, 8, 35, QColor(200,195,185), QColor(170,160,145),
                QColor(185,185,175), 0, 0};
    if (name.contains("田径"))
        return {58, 6, 48, QColor(195,210,195), QColor(155,180,155),
                QColor(175,200,175), 0, 0};
    // Colleges
    if (name.contains("学院") || name.contains("楼"))
        return {38, 45, 32, QColor(200,205,215), QColor(130,140,165),
                QColor(155,170,200), 4, 3};
    // Gates
    if (name.contains("门"))
        return {32, 16, 18, QColor(195,185,170), QColor(165,150,130),
                QColor(175,180,170), 1, 2};
    // Default
    return {36, 35, 28, QColor(195,200,210), QColor(135,145,165),
            QColor(160,175,200), 3, 3};
}

// ═══════════════════════════════════════════════════════════════════════════
// IsometricBuilding
// ═══════════════════════════════════════════════════════════════════════════

IsometricBuilding::IsometricBuilding(int nodeId, const QString& name,
                                     qreal w, qreal h, qreal depth,
                                     const QColor& wallColor,
                                     const QColor& roofColor,
                                     QGraphicsItem* parent)
    : QGraphicsObject(parent)
    , m_nodeId(nodeId), m_name(name)
    , m_w(w), m_h(h), m_depth(depth)
    , m_wallColor(wallColor), m_roofColor(roofColor)
{
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable);
    setZValue(10);

    auto* shadow = new QGraphicsDropShadowEffect;
    shadow->setBlurRadius(25);
    shadow->setOffset(8, 8);
    shadow->setColor(QColor(0, 0, 0, 60));
    setGraphicsEffect(shadow);
}

QRectF IsometricBuilding::boundingRect() const {
    return {-m_w - 6, -m_h - m_depth - 6,
            m_w * 2 + 12, m_h + m_depth * 2 + 12};
}

void IsometricBuilding::paint(QPainter* painter,
                              const QStyleOptionGraphicsItem*, QWidget*) {
    painter->setRenderHint(QPainter::Antialiasing);

    qreal hw = m_w / 2, hd = m_depth / 2;

    QColor wall = m_highlighted ? m_hlColor : m_wallColor;
    QColor roof = m_highlighted ? m_hlColor.lighter(140) : m_roofColor;
    QColor leftWall  = wall.darker(120);
    QColor rightWall = wall.darker(108);
    QColor outline   = wall.darker(150);

    if (m_hovered && !m_highlighted) {
        wall = wall.lighter(112);
        leftWall = leftWall.lighter(112);
        rightWall = rightWall.lighter(112);
        roof = roof.lighter(112);
    }

    // ── Left wall ──
    QPolygonF leftFace;
    leftFace << QPointF(0, 0) << QPointF(-hw, hd * 0.5)
             << QPointF(-hw, hd * 0.5 - m_h) << QPointF(0, -m_h);
    painter->setPen(QPen(outline, 0.8));
    painter->setBrush(QBrush(leftWall));
    painter->drawPolygon(leftFace);

    // ── Right wall ──
    QPolygonF rightFace;
    rightFace << QPointF(0, 0) << QPointF(hw, hd * 0.5)
              << QPointF(hw, hd * 0.5 - m_h) << QPointF(0, -m_h);
    painter->setBrush(QBrush(rightWall));
    painter->drawPolygon(rightFace);

    // ── Roof ──
    QPolygonF roofFace;
    roofFace << QPointF(0, -m_h)
             << QPointF(-hw, hd * 0.5 - m_h)
             << QPointF(0, hd - m_h)
             << QPointF(hw, hd * 0.5 - m_h);
    painter->setBrush(QBrush(roof));
    painter->drawPolygon(roofFace);

    // Roof edge highlight
    painter->setPen(QPen(roof.lighter(140), 1.5));
    painter->setBrush(Qt::NoBrush);
    painter->drawPolygon(roofFace);

    // Roof inner line
    QPolygonF roofInner;
    qreal inset = 3;
    roofInner << QPointF(0, -m_h + inset * 1.5)
              << QPointF(-hw + inset, hd * 0.5 - m_h + inset * 0.5)
              << QPointF(0, hd - m_h - inset * 0.5)
              << QPointF(hw - inset, hd * 0.5 - m_h + inset * 0.5);
    painter->setPen(QPen(roof.lighter(120), 0.5));
    painter->drawPolygon(roofInner);

    // ── Windows ──
    const auto cfg = cfgFor(m_name);
    if (cfg.winRows > 0 && cfg.winCols > 0) {
        QColor winColor = m_highlighted
            ? QColor(200, 220, 255, 180)
            : QColor(140, 180, 220, 160);
        QColor winFrame = winColor.darker(130);

        // Left wall windows
        painter->setPen(QPen(winFrame, 0.6));
        painter->setBrush(QBrush(winColor));
        qreal winW = (hw * 0.7) / cfg.winCols;
        qreal winH = (m_h * 0.6) / cfg.winRows;
        for (int r = 0; r < cfg.winRows; ++r) {
            for (int c = 0; c < cfg.winCols; ++c) {
                qreal wx = -hw + hw * 0.15 + c * winW + winW * 0.15;
                qreal wy = hd * 0.5 - m_h + m_h * 0.2 + r * winH;
                painter->drawRect(QRectF(wx, wy, winW * 0.7, winH * 0.65));
            }
        }

        // Right wall windows
        winW = (hw * 0.7) / qMax(cfg.winCols - 1, 1);
        for (int r = 0; r < cfg.winRows; ++r) {
            for (int c = 0; c < cfg.winCols - 1; ++c) {
                qreal wx = hw * 0.15 + c * winW + winW * 0.15;
                qreal wy = hd * 0.5 - m_h + m_h * 0.2 + r * winH;
                painter->drawRect(QRectF(wx, wy, winW * 0.7, winH * 0.65));
            }
        }
    }

    // ── Entrance canopy ──
    if (m_h > 20 && !m_name.contains("田径") && !m_name.contains("篮球")) {
        QColor canopyColor = roof.darker(115);
        painter->setPen(QPen(canopyColor.darker(120), 0.8));
        painter->setBrush(QBrush(canopyColor));
        QPolygonF canopy;
        canopy << QPointF(-hw * 0.3, hd * 0.25)
               << QPointF(hw * 0.3, hd * 0.25)
               << QPointF(hw * 0.3, hd * 0.25 + 4)
               << QPointF(-hw * 0.3, hd * 0.25 + 4);
        painter->drawPolygon(canopy);
    }

    // ── Label ──
    painter->setPen(QColor(40, 40, 40));
    QFont f("Microsoft YaHei", 7, QFont::Bold);
    painter->setFont(f);
    QRectF labelRect(-hw - 12, hd * 0.5 + 6, m_w + 24, 16);
    painter->drawText(labelRect, Qt::AlignCenter, m_name);

    // ── Highlight glow ──
    if (m_highlighted) {
        QRadialGradient glow(0, -m_h / 2, m_w * 0.8);
        glow.setColorAt(0.0, QColor(255, 255, 200, 80));
        glow.setColorAt(1.0, QColor(255, 255, 200, 0));
        painter->setPen(Qt::NoPen);
        painter->setBrush(glow);
        painter->drawEllipse(QPointF(0, -m_h / 2), m_w * 0.8, m_h * 0.6);
    }
}

void IsometricBuilding::setHighlighted(bool on, const QColor& color) {
    m_highlighted = on;
    m_hlColor = color;
    if (on) {
        setGraphicsEffect(nullptr);
    } else {
        auto* shadow = new QGraphicsDropShadowEffect;
        shadow->setBlurRadius(25);
        shadow->setOffset(8, 8);
        shadow->setColor(QColor(0, 0, 0, 60));
        setGraphicsEffect(shadow);
    }
    update();
}

void IsometricBuilding::mousePressEvent(QGraphicsSceneMouseEvent* e) {
    QGraphicsObject::mousePressEvent(e);
    emit clicked(m_nodeId);
}

void IsometricBuilding::hoverEnterEvent(QGraphicsSceneHoverEvent*) {
    m_hovered = true;
    update();
}

void IsometricBuilding::hoverLeaveEvent(QGraphicsSceneHoverEvent*) {
    m_hovered = false;
    update();
}

// ═══════════════════════════════════════════════════════════════════════════
// IsometricTree — 3 styles: round, conical, bushy
// ═══════════════════════════════════════════════════════════════════════════

IsometricTree::IsometricTree(qreal size, int style, QGraphicsItem* parent)
    : QGraphicsObject(parent), m_size(size), m_style(style)
{
    setZValue(5);
}

QRectF IsometricTree::boundingRect() const {
    return {-m_size * 1.2, -m_size * 3, m_size * 2.4, m_size * 4};
}

void IsometricTree::paint(QPainter* painter,
                          const QStyleOptionGraphicsItem*, QWidget*) {
    painter->setRenderHint(QPainter::Antialiasing);

    // Ground shadow
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(0, 0, 0, 22));
    painter->drawEllipse(QPointF(2, 3), m_size * 0.5, m_size * 0.25);

    // Trunk
    QColor trunk(110, 85, 55);
    painter->setBrush(trunk);
    painter->setPen(QPen(trunk.darker(120), 0.8));
    painter->drawRect(-1.5, -m_size * 0.9, 3, m_size * 0.9);

    if (m_style == 0) {
        // Round tree — layered spheres
        struct L { qreal y, rx, ry; QColor c; };
        L layers[] = {
            {-m_size * 0.9, m_size * 0.5, m_size * 0.4, QColor(70, 145, 75)},
            {-m_size * 1.4, m_size * 0.42, m_size * 0.35, QColor(85, 165, 85)},
            {-m_size * 1.8, m_size * 0.3, m_size * 0.25, QColor(105, 180, 100)},
        };
        for (auto& l : layers) {
            painter->setPen(QPen(l.c.darker(125), 0.8));
            painter->setBrush(l.c);
            painter->drawEllipse(QPointF(0, l.y), l.rx, l.ry);
        }
        // Highlights
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(255, 255, 220, 35));
        painter->drawEllipse(QPointF(-m_size * 0.1, -m_size * 1.2), 2, 1.5);
    } else if (m_style == 1) {
        // Conical tree — pine/cedar
        painter->setPen(QPen(QColor(50, 120, 60).darker(120), 0.8));
        painter->setBrush(QColor(60, 130, 65));
        QPolygonF tri;
        tri << QPointF(0, -m_size * 2.2)
            << QPointF(-m_size * 0.45, -m_size * 0.6)
            << QPointF(m_size * 0.45, -m_size * 0.6);
        painter->drawPolygon(tri);
        painter->setBrush(QColor(75, 150, 80));
        tri = QPolygonF();
        tri << QPointF(0, -m_size * 1.8)
            << QPointF(-m_size * 0.5, -m_size * 0.4)
            << QPointF(m_size * 0.5, -m_size * 0.4);
        painter->drawPolygon(tri);
    } else {
        // Bushy — multiple small circles
        painter->setPen(Qt::NoPen);
        QColor bush(80, 155, 80);
        struct P { qreal x, y, r; };
        P pts[] = {
            {0, -m_size * 1.0, m_size * 0.35},
            {-m_size * 0.3, -m_size * 0.8, m_size * 0.28},
            {m_size * 0.3, -m_size * 0.8, m_size * 0.28},
            {-m_size * 0.15, -m_size * 1.3, m_size * 0.3},
            {m_size * 0.15, -m_size * 1.3, m_size * 0.3},
            {0, -m_size * 1.5, m_size * 0.22},
        };
        for (auto& p : pts) {
            painter->setBrush(bush);
            painter->drawEllipse(QPointF(p.x, p.y), p.r, p.r * 0.8);
            bush = bush.lighter(105);
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// AnimBall — glowing animated ball
// ═══════════════════════════════════════════════════════════════════════════

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

    // Outer glow
    QRadialGradient glow(0, 0, m_radius * 2.5);
    glow.setColorAt(0.0, QColor(255, 120, 50, 160));
    glow.setColorAt(0.3, QColor(255, 90, 30, 80));
    glow.setColorAt(0.7, QColor(255, 60, 20, 20));
    glow.setColorAt(1.0, QColor(255, 50, 10, 0));
    painter->setPen(Qt::NoPen);
    painter->setBrush(glow);
    painter->drawEllipse(QPointF(0, 0), m_radius * 2.5, m_radius * 2.5);

    // Ball body
    QRadialGradient fill(-m_radius * 0.3, -m_radius * 0.3, m_radius * 1.2);
    fill.setColorAt(0.0, m_color.lighter(170));
    fill.setColorAt(0.5, m_color);
    fill.setColorAt(1.0, m_color.darker(140));
    painter->setBrush(fill);
    painter->setPen(QPen(m_color.darker(160), 1.5));
    painter->drawEllipse(QPointF(0, 0), m_radius, m_radius);

    // Specular
    painter->setBrush(QColor(255, 255, 255, 140));
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(QPointF(-m_radius * 0.25, -m_radius * 0.3),
                         m_radius * 0.28, m_radius * 0.18);
}

// ═══════════════════════════════════════════════════════════════════════════
// MapView — constructor & setup
// ═══════════════════════════════════════════════════════════════════════════

MapView::MapView(QWidget* parent) : QGraphicsView(parent) {
    setRenderHint(QPainter::Antialiasing);
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameStyle(QFrame::NoFrame);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    setupScene();
}

void MapView::setupScene() {
    m_scene = new QGraphicsScene(this);
    m_scene->setBackgroundBrush(QColor(215, 225, 200));
    setScene(m_scene);
    setStyleSheet("border: none;");
}

void MapView::setGraph(Graph* graph) { m_graph = graph; }

// Isometric projection — north (small y) at top, south (large y) at bottom
QPointF MapView::iso(qreal x, qreal y) const {
    return QPointF((x + y) * SX * SCALE, -(x - y) * SY * SCALE);
}

qreal MapView::depthSort(qreal x, qreal y) const {
    return (x + y) * 0.01;
}

// ═══════════════════════════════════════════════════════════════════════════
// drawMap — main entry
// ═══════════════════════════════════════════════════════════════════════════

void MapView::drawMap() {
    if (!m_graph) return;
    m_scene->clear();
    m_buildings.clear();
    m_pathItems.clear();
    m_pathIds.clear();
    m_ball = nullptr;
    m_animGroup = nullptr;

    drawGround();
    drawGrassPatches();
    drawRoads();
    drawBuildings();
    drawTrees();

    fitInView(m_scene->sceneRect().adjusted(-50, -50, 50, 50), Qt::KeepAspectRatio);
}

// ═══════════════════════════════════════════════════════════════════════════
// drawGround — isometric grass plane
// ═══════════════════════════════════════════════════════════════════════════

void MapView::drawGround() {
    const auto nodes = m_graph->allNodes();
    double minX = 1e9, minY = 1e9, maxX = -1e9, maxY = -1e9;
    for (const auto& n : nodes) {
        minX = qMin(minX, n.x); minY = qMin(minY, n.y);
        maxX = qMax(maxX, n.x); maxY = qMax(maxY, n.y);
    }

    qreal pad = 100;
    QPointF tl = iso(minX - pad, minY - pad);
    QPointF tr = iso(maxX + pad, minY - pad);
    QPointF br = iso(maxX + pad, maxY + pad);
    QPointF bl = iso(minX - pad, maxY + pad);

    QPolygonF ground;
    ground << tl << tr << br << bl;

    QLinearGradient grad(tl.x(), tl.y(), br.x(), br.y());
    grad.setColorAt(0.0, QColor(190, 210, 170));
    grad.setColorAt(0.3, QColor(180, 200, 160));
    grad.setColorAt(0.7, QColor(175, 195, 155));
    grad.setColorAt(1.0, QColor(170, 190, 150));

    auto* item = m_scene->addPolygon(ground, QPen(QColor(155, 175, 140), 1.5), QBrush(grad));
    item->setZValue(-100);
}

// ═══════════════════════════════════════════════════════════════════════════
// drawGrassPatches — dark green patches for texture
// ═══════════════════════════════════════════════════════════════════════════

void MapView::drawGrassPatches() {
    const auto nodes = m_graph->allNodes();
    auto rng = QRandomGenerator::global();

    // Scatter grass patches between nodes
    QSet<QPair<int,int>> seen;
    for (const auto& n : nodes) {
        for (const auto& e : m_graph->getEdges(n.id)) {
            if (e.from >= e.to) continue;
            QPair<int,int> key(e.from, e.to);
            if (seen.contains(key)) continue;
            seen.insert(key);

            const Node& a = m_graph->node(e.from);
            const Node& b = m_graph->node(e.to);

            for (int i = 0; i < 2; ++i) {
                qreal frac = 0.2 + rng->bounded(60) / 100.0;
                qreal gx = a.x + (b.x - a.x) * frac;
                qreal gy = a.y + (b.y - a.y) * frac;

                qreal dx = b.x - a.x, dy = b.y - a.y;
                qreal len = std::sqrt(dx * dx + dy * dy);
                if (len < 1) continue;
                qreal px = -dy / len, py = dx / len;
                qreal side = (rng->bounded(2) == 0) ? 1.0 : -1.0;
                qreal offset = 25 + rng->bounded(30);
                gx += px * offset * side;
                gy += py * offset * side;

                QPointF pos = iso(gx, gy);
                qreal rx = 8 + rng->bounded(12);
                qreal ry = rx * 0.5;

                QColor gc = QColor(
                    130 + rng->bounded(25),
                    170 + rng->bounded(25),
                    100 + rng->bounded(20),
                    80 + rng->bounded(40));
                auto* patch = m_scene->addEllipse(
                    pos.x() - rx, pos.y() - ry, rx * 2, ry * 2,
                    Qt::NoPen, QBrush(gc));
                patch->setZValue(-90);
            }
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// drawRoads — curved roads with shadow, width, gradient
// ═══════════════════════════════════════════════════════════════════════════

void MapView::drawRoads() {
    const auto nodes = m_graph->allNodes();

    for (const auto& n : nodes) {
        for (const auto& e : m_graph->getEdges(n.id)) {
            if (e.from >= e.to) continue;
            const Node& a = m_graph->node(e.from);
            const Node& b = m_graph->node(e.to);

            QPointF pa = iso(a.x, a.y);
            QPointF pb = iso(b.x, b.y);

            // Smooth curve with control point offset
            QPointF mid = (pa + pb) / 2;
            QPointF perp(pa.y() - pb.y(), pb.x() - pa.x());
            qreal len = std::sqrt(perp.x() * perp.x() + perp.y() * perp.y());
            if (len > 0) perp /= len;
            QPointF cp = mid + perp * 6.0;

            QPainterPath path(pa);
            path.quadTo(cp, pb);

            // 1. Road shadow (widest)
            QPen shadowPen(QColor(0, 0, 0, 25), 14);
            shadowPen.setCapStyle(Qt::RoundCap);
            auto* s1 = m_scene->addPath(path, shadowPen);
            s1->setZValue(-55);

            // 2. Road base (dark gray edge)
            QPen basePen(QColor(165, 160, 150), 11);
            basePen.setCapStyle(Qt::RoundCap);
            auto* s2 = m_scene->addPath(path, basePen);
            s2->setZValue(-50);

            // 3. Road surface
            QPen surfPen(QColor(215, 210, 200), 7);
            surfPen.setCapStyle(Qt::RoundCap);
            auto* s3 = m_scene->addPath(path, surfPen);
            s3->setZValue(-49);

            // 4. Road center highlight
            QPen centerPen(QColor(230, 225, 218), 3);
            centerPen.setCapStyle(Qt::RoundCap);
            auto* s4 = m_scene->addPath(path, centerPen);
            s4->setZValue(-48);

            // 5. Dashed center line
            QPen dashPen(QColor(255, 255, 230, 100), 1, Qt::DashLine);
            dashPen.setDashPattern({5, 8});
            dashPen.setCapStyle(Qt::RoundCap);
            auto* s5 = m_scene->addPath(path, dashPen);
            s5->setZValue(-47);
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// drawBuildings — 3D isometric buildings with auto-style
// ═══════════════════════════════════════════════════════════════════════════

void MapView::drawBuildings() {
    const auto nodes = m_graph->allNodes();

    for (const auto& n : nodes) {
        auto c = cfgFor(n.name);
        auto* bldg = new IsometricBuilding(
            n.id, n.name, c.w, c.h, c.depth, c.wall, c.roof);

        QPointF pos = iso(n.x, n.y);
        bldg->setPos(pos);
        bldg->setZValue(depthSort(n.x, n.y) + 10);

        connect(bldg, &IsometricBuilding::clicked, this, &MapView::nodeClicked);
        m_scene->addItem(bldg);
        m_buildings[n.id] = bldg;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// drawTrees — decorative 3D trees scattered along roads
// ═══════════════════════════════════════════════════════════════════════════

void MapView::drawTrees() {
    const auto nodes = m_graph->allNodes();
    auto rng = QRandomGenerator::global();
    QSet<QPair<int,int>> seen;

    for (const auto& n : nodes) {
        for (const auto& e : m_graph->getEdges(n.id)) {
            if (e.from >= e.to) continue;
            QPair<int,int> key(e.from, e.to);
            if (seen.contains(key)) continue;
            seen.insert(key);

            const Node& a = m_graph->node(e.from);
            const Node& b = m_graph->node(e.to);

            int count = 1 + rng->bounded(3);  // 1-3 trees per edge
            for (int t = 0; t < count; ++t) {
                qreal frac = 0.15 + rng->bounded(70) / 100.0;
                qreal tx = a.x + (b.x - a.x) * frac;
                qreal ty = a.y + (b.y - a.y) * frac;

                qreal dx = b.x - a.x, dy = b.y - a.y;
                qreal len = std::sqrt(dx * dx + dy * dy);
                if (len < 1) continue;
                qreal px = -dy / len, py = dx / len;
                qreal side = (rng->bounded(2) == 0) ? 1.0 : -1.0;
                qreal offset = 18 + rng->bounded(25);
                tx += px * offset * side;
                ty += py * offset * side;

                QPointF pos = iso(tx, ty);
                qreal size = 5 + rng->bounded(7);
                int style = rng->bounded(3);

                auto* tree = new IsometricTree(size, style);
                tree->setPos(pos);
                tree->setZValue(depthSort(tx, ty) + 5);
                m_scene->addItem(tree);
            }
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// drawPath — gradient path with glow
// ═══════════════════════════════════════════════════════════════════════════

void MapView::drawPath(const QVector<int>& path) {
    if (path.size() < 2 || !m_graph) return;

    QPainterPath isoPath;
    QPointF p0 = iso(m_graph->node(path.first()).x,
                     m_graph->node(path.first()).y);
    isoPath.moveTo(p0);

    for (int i = 1; i < path.size(); ++i) {
        const Node& n = m_graph->node(path[i]);
        QPointF pi = iso(n.x, n.y);
        if (i + 1 < path.size()) {
            const Node& nx = m_graph->node(path[i + 1]);
            QPointF pn = iso(nx.x, nx.y);
            QPointF cp = pi + (pn - pi) * 0.12 + (pi - p0) * 0.12;
            isoPath.quadTo(cp, pi);
        } else {
            isoPath.lineTo(pi);
        }
        p0 = pi;
    }

    // Layer 1: shadow
    QPen sp(QColor(200, 50, 30, 40), 14);
    sp.setCapStyle(Qt::RoundCap); sp.setJoinStyle(Qt::RoundJoin);
    auto* l1 = m_scene->addPath(isoPath, sp);
    l1->setZValue(30); m_pathItems.append(l1);

    // Layer 2: outer glow
    QPen gp(QColor(255, 100, 60, 70), 10);
    gp.setCapStyle(Qt::RoundCap); gp.setJoinStyle(Qt::RoundJoin);
    auto* l2 = m_scene->addPath(isoPath, gp);
    l2->setZValue(31); m_pathItems.append(l2);

    // Layer 3: main
    QPen mp(QColor(230, 50, 30), 5);
    mp.setCapStyle(Qt::RoundCap); mp.setJoinStyle(Qt::RoundJoin);
    auto* l3 = m_scene->addPath(isoPath, mp);
    l3->setZValue(32); m_pathItems.append(l3);

    // Layer 4: center highlight
    QPen cp(QColor(255, 180, 100, 140), 2);
    cp.setCapStyle(Qt::RoundCap);
    auto* l4 = m_scene->addPath(isoPath, cp);
    l4->setZValue(33); m_pathItems.append(l4);

    for (int id : path) m_pathIds.append(id);
}

// ═══════════════════════════════════════════════════════════════════════════
// animatePath
// ═══════════════════════════════════════════════════════════════════════════

void MapView::animatePath(const QVector<int>& path) {
    if (path.size() < 2 || !m_graph) return;
    if (m_animGroup) { m_animGroup->stop(); delete m_animGroup; m_animGroup = nullptr; }
    if (m_ball) { m_scene->removeItem(m_ball); delete m_ball; m_ball = nullptr; }

    QPointF startPos = iso(m_graph->node(path.first()).x,
                           m_graph->node(path.first()).y);
    m_ball = new AnimBall(BALL_RADIUS, QColor(255, 90, 30));
    m_ball->setPos(startPos);
    m_scene->addItem(m_ball);

    m_animGroup = new QSequentialAnimationGroup(this);
    for (int i = 0; i + 1 < path.size(); ++i) {
        QPointF pf = iso(m_graph->node(path[i]).x,
                         m_graph->node(path[i]).y);
        QPointF pt = iso(m_graph->node(path[i + 1]).x,
                         m_graph->node(path[i + 1]).y);
        qreal dist = std::sqrt(std::pow(pt.x() - pf.x(), 2) +
                               std::pow(pt.y() - pf.y(), 2));
        auto* seg = new QPropertyAnimation(m_ball, "pos");
        seg->setDuration(qBound(200, static_cast<int>(dist * 4), 800));
        seg->setStartValue(pf);
        seg->setEndValue(pt);
        seg->setEasingCurve(QEasingCurve::InOutQuad);
        m_animGroup->addAnimation(seg);
    }
    m_animGroup->start();
}

// ═══════════════════════════════════════════════════════════════════════════
// clearPath
// ═══════════════════════════════════════════════════════════════════════════

void MapView::clearPath() {
    if (m_animGroup) { m_animGroup->stop(); delete m_animGroup; m_animGroup = nullptr; }
    if (m_ball) { m_scene->removeItem(m_ball); delete m_ball; m_ball = nullptr; }
    for (auto* item : m_pathItems) { m_scene->removeItem(item); delete item; }
    m_pathItems.clear();
    for (int id : m_pathIds) resetBuildingStyle(id);
    m_pathIds.clear();
    if (m_startNodeId >= 0) { resetBuildingStyle(m_startNodeId); m_startNodeId = -1; }
    if (m_endNodeId >= 0)   { resetBuildingStyle(m_endNodeId);   m_endNodeId = -1; }
}

void MapView::resetBuildingStyle(int id) {
    if (auto* b = m_buildings.value(id)) b->setHighlighted(false);
}

void MapView::highlightStartEnd(int startId, int endId) {
    if (m_startNodeId >= 0) resetBuildingStyle(m_startNodeId);
    if (m_endNodeId >= 0)   resetBuildingStyle(m_endNodeId);
    m_startNodeId = startId;
    m_endNodeId = endId;
    if (auto* b = m_buildings.value(startId))
        b->setHighlighted(true, QColor(50, 200, 80));
    if (auto* b = m_buildings.value(endId))
        b->setHighlighted(true, QColor(220, 50, 50));
}

void MapView::centerOnNode(int id) {
    if (!m_graph || !m_graph->hasNode(id)) return;
    const Node& n = m_graph->node(id);
    centerOn(iso(n.x, n.y));
}

void MapView::highlightNode(int id, const QColor& color) {
    if (auto* b = m_buildings.value(id)) b->setHighlighted(true, color);
}

// ═══════════════════════════════════════════════════════════════════════════
// Zoom
// ═══════════════════════════════════════════════════════════════════════════

void MapView::zoomIn() {
    if (transform().m11() * ZOOM_FACTOR <= MAX_SCALE)
        scale(ZOOM_FACTOR, ZOOM_FACTOR);
}

void MapView::zoomOut() {
    if (transform().m11() / ZOOM_FACTOR >= MIN_SCALE)
        scale(1.0 / ZOOM_FACTOR, 1.0 / ZOOM_FACTOR);
}

void MapView::fitMap() {
    if (m_scene)
        fitInView(m_scene->sceneRect().adjusted(-50, -50, 50, 50), Qt::KeepAspectRatio);
}

void MapView::wheelEvent(QWheelEvent* event) {
    double factor = (event->angleDelta().y() > 0) ? ZOOM_FACTOR : (1.0 / ZOOM_FACTOR);
    if (transform().m11() * factor < MIN_SCALE || transform().m11() * factor > MAX_SCALE)
        return;
    scale(factor, factor);
}
