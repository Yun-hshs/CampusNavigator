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
#include <algorithm>

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

    // ── 标签已移至 LabelManager 统一管理（碰撞检测 + LOD）──

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
// VegetationLayer — batch-painted procedural trees & flowers
// ═══════════════════════════════════════════════════════════════════════════

struct TreeData {
    QPointF pos;      // 屏幕坐标
    qreal   size;     // 树冠半径
    int     style;    // 0=圆冠, 1=尖冠, 2=灌木
    QColor  crownColor;
};

class VegetationLayer : public QGraphicsObject {
public:
    explicit VegetationLayer(const QVector<TreeData>& trees,
                             QGraphicsItem* parent = nullptr)
        : QGraphicsObject(parent), m_trees(trees)
    {
        // Compute bounding rect from all tree positions
        m_bounds = QRectF();
        for (const auto& t : trees) {
            qreal r = t.size * 2;
            m_bounds |= QRectF(t.pos.x() - r, t.pos.y() - r * 1.8, r * 2, r * 2.5);
        }
    }

    QRectF boundingRect() const override { return m_bounds; }

    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override {
        painter->setPen(Qt::NoPen);

        for (const auto& t : m_trees) {
            qreal r = t.size;

            // Shadow
            painter->setBrush(QColor(0, 0, 0, 22));
            painter->drawEllipse(t.pos.x() - r * 0.8, t.pos.y() + r * 0.2,
                                 r * 1.6, r * 0.7);

            if (t.style == 0) {
                // Round crown — 3 layered circles
                painter->setBrush(t.crownColor.darker(115));
                painter->drawEllipse(t.pos.x() - r, t.pos.y() - r * 1.6,
                                     r * 2, r * 1.5);
                painter->setBrush(t.crownColor);
                painter->drawEllipse(t.pos.x() - r * 0.85, t.pos.y() - r * 2.0,
                                     r * 1.7, r * 1.4);
                painter->setBrush(t.crownColor.lighter(115));
                painter->drawEllipse(t.pos.x() - r * 0.6, t.pos.y() - r * 2.3,
                                     r * 1.2, r * 1.0);
            } else if (t.style == 1) {
                // Conical crown (pine)
                QPolygonF tri;
                tri << QPointF(t.pos.x(), t.pos.y() - r * 2.8)
                    << QPointF(t.pos.x() - r * 1.1, t.pos.y() - r * 0.3)
                    << QPointF(t.pos.x() + r * 1.1, t.pos.y() - r * 0.3);
                painter->setBrush(t.crownColor);
                painter->drawPolygon(tri);
                painter->setBrush(t.crownColor.lighter(110));
                QPolygonF tri2;
                tri2 << QPointF(t.pos.x(), t.pos.y() - r * 3.4)
                     << QPointF(t.pos.x() - r * 0.8, t.pos.y() - r * 1.5)
                     << QPointF(t.pos.x() + r * 0.8, t.pos.y() - r * 1.5);
                painter->drawPolygon(tri2);
            } else {
                // Bushy — multiple small circles
                painter->setBrush(t.crownColor);
                painter->drawEllipse(t.pos.x() - r * 0.9, t.pos.y() - r * 0.9,
                                     r * 1.3, r * 1.0);
                painter->setBrush(t.crownColor.lighter(108));
                painter->drawEllipse(t.pos.x() + r * 0.1, t.pos.y() - r * 1.1,
                                     r * 1.1, r * 0.9);
                painter->setBrush(t.crownColor.darker(105));
                painter->drawEllipse(t.pos.x() - r * 0.4, t.pos.y() - r * 1.5,
                                     r * 1.0, r * 0.8);
            }
        }
    }

private:
    QVector<TreeData> m_trees;
    QRectF m_bounds;
};

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
    m_labelMgr = new LabelManager(m_scene);

    // 蚂蚁线定时器（每 60ms 刷新一次虚线偏移）
    m_antsTimer = new QTimer(this);
    m_antsTimer->setInterval(60);
    connect(m_antsTimer, &QTimer::timeout, this, [this]() {
        if (m_antsItem) {
            m_antsOffset += 1.5;
            // 动态更新虚线偏移
            QPen pen = m_antsItem->pen();
            pen.setDashOffset(m_antsOffset);
            m_antsItem->setPen(pen);
        }
    });
}

void MapView::setGraph(Graph* graph) { m_graph = graph; }

// Isometric projection via GeoTransform affine matrix
// 逻辑坐标（经 m_logicScale 缩放后）→ 屏幕像素
QPointF MapView::iso(qreal x, qreal y) const {
    return m_geo.toScreen(x * m_logicScale, y * m_logicScale);
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

    // ── 配置仿射变换 ──
    // 计算逻辑坐标包围盒
    const auto nodes = m_graph->allNodes();
    double lxMin = 1e9, lyMin = 1e9, lxMax = -1e9, lyMax = -1e9;
    for (const auto& n : nodes) {
        lxMin = qMin(lxMin, n.x); lyMin = qMin(lyMin, n.y);
        lxMax = qMax(lxMax, n.x); lyMax = qMax(lyMax, n.y);
    }
    // 将逻辑中心映射到屏幕原点 (0,0)，后续场景自动居中
    // 比例尺：逻辑范围 → 屏幕 800px 宽
    double logicW = (lxMax - lxMin) * m_logicScale;
    double mpp = logicW / 800.0;  // 米/像素
    if (mpp < 0.01) mpp = 0.5;
    m_geo.configure(mpp, QPointF(0, 0));

    // 计算屏幕包围盒以设置场景 rect
    QPointF sMin = iso(lxMin, lyMin);
    QPointF sMax = iso(lxMax, lyMax);
    // 等轴测下还需要考虑四个角
    QPointF s1 = iso(lxMin, lyMax);
    QPointF s2 = iso(lxMax, lyMin);
    double sxMin = std::min({sMin.x(), sMax.x(), s1.x(), s2.x()});
    double sxMax = std::max({sMin.x(), sMax.x(), s1.x(), s2.x()});
    double syMin = std::min({sMin.y(), sMax.y(), s1.y(), s2.y()});
    double syMax = std::max({sMin.y(), sMax.y(), s1.y(), s2.y()});

    double pad = 80;
    m_scene->setSceneRect(sxMin - pad, syMin - pad,
                          sxMax - sxMin + 2 * pad,
                          syMax - syMin + 2 * pad);

    drawGround();
    drawGrassPatches();
    drawAreas();
    drawRoads();
    drawShadows();
    drawBuildings();

    // ── Procedural vegetation (single batch-painted item) ──
    {
        QVector<TreeData> trees;
        auto rng = QRandomGenerator::global();

        // Build exclusion path from buildings
        QPainterPath exclusion;
        for (auto* b : m_buildings) {
            QRectF br = b->boundingRect().translated(b->pos()).adjusted(-25, -25, 25, 25);
            exclusion.addRect(br);
        }

        // Add road corridors to exclusion
        QSet<QPair<int,int>> seenEdges;
        for (const auto& n : m_graph->allNodes()) {
            for (const auto& e : m_graph->getEdges(n.id)) {
                QPair<int,int> key(qMin(e.from, e.to), qMax(e.from, e.to));
                if (seenEdges.contains(key)) continue;
                seenEdges.insert(key);
                const Node& a = m_graph->node(e.from);
                const Node& b = m_graph->node(e.to);
                QPointF pa = iso(a.x, a.y);
                QPointF pb = iso(b.x, b.y);
                QPainterPath road(pa);
                road.lineTo(pb);
                QPainterPathStroker stroker;
                stroker.setWidth(14);
                exclusion.addPath(stroker.createStroke(road));
            }
        }

        // Add area polygons to exclusion
        for (const auto& area : m_graph->allAreas()) {
            QPolygonF sp;
            for (const QPointF& pt : area.polygon)
                sp << iso(pt.x(), pt.y());
            exclusion.addPolygon(sp);
        }

        // Scene rect in scene coordinates
        QRectF sceneR = m_scene->sceneRect().adjusted(20, 20, -20, -20);
        int treeCount = 120;
        int attempts = 0;
        int maxAttempts = treeCount * 8;

        while (trees.size() < treeCount && attempts < maxAttempts) {
            ++attempts;
            qreal x = sceneR.left() + rng->bounded(static_cast<int>(sceneR.width()));
            qreal y = sceneR.top()  + rng->bounded(static_cast<int>(sceneR.height()));

            if (exclusion.contains(QPointF(x, y)))
                continue;

            TreeData td;
            td.pos = QPointF(x, y);
            td.size = 4 + rng->bounded(6);
            td.style = rng->bounded(3);
            td.crownColor = QColor(
                55 + rng->bounded(40),
                120 + rng->bounded(50),
                45 + rng->bounded(35));
            trees.append(td);
        }

        if (!trees.isEmpty()) {
            auto* vegLayer = new VegetationLayer(trees);
            vegLayer->setZValue(5);
            m_scene->addItem(vegLayer);
        }
    }

    updateLabelsAndLOD();

    fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
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

    // pad 在逻辑坐标中（像素单位 / logicScale）
    qreal pad = 100.0 / m_logicScale;
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

    // Subtle isometric grid lines for texture
    QPen gridPen(QColor(160, 180, 145, 35), 0.5);
    qreal step = 40;
    for (int i = -5; i <= 30; ++i) {
        // Lines along x-axis direction
        QPointF ga = iso(minX - pad + i * step, minY - pad);
        QPointF gb = iso(minX - pad + i * step, maxY + pad);
        auto* gl = m_scene->addLine(ga.x(), ga.y(), gb.x(), gb.y(), gridPen);
        gl->setZValue(-99);
        // Lines along y-axis direction
        QPointF gc = iso(minX - pad, minY - pad + i * step);
        QPointF gd = iso(maxX + pad, minY - pad + i * step);
        auto* gl2 = m_scene->addLine(gc.x(), gc.y(), gd.x(), gd.y(), gridPen);
        gl2->setZValue(-99);
    }
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
// drawAreas — terrain polygons (lake, plaza, sports field, garden)
// ═══════════════════════════════════════════════════════════════════════════

void MapView::drawAreas() {
    const auto areas = m_graph->allAreas();
    if (areas.isEmpty()) return;

    for (const auto& area : areas) {
        // Convert logical polygon to screen polygon
        QPolygonF screenPoly;
        for (const QPointF& pt : area.polygon)
            screenPoly << iso(pt.x(), pt.y());

        QRectF bounds = screenPoly.boundingRect();

        if (area.type == "lake") {
            // ── Lake: blue gradient + ripple lines ──
            QLinearGradient grad(bounds.topLeft(), bounds.bottomRight());
            grad.setColorAt(0.0, QColor(100, 170, 220, 180));
            grad.setColorAt(0.5, QColor(80, 150, 210, 200));
            grad.setColorAt(1.0, QColor(110, 180, 225, 180));
            auto* item = m_scene->addPolygon(screenPoly, Qt::NoPen, QBrush(grad));
            item->setZValue(area.zOrder);

            // Ripple lines
            QPen ripplePen(QColor(140, 195, 235, 80), 0.8);
            qreal step = 15;
            for (int i = -3; i <= 8; ++i) {
                qreal x1 = bounds.left() + i * step;
                qreal y1 = bounds.top() + bounds.height() * 0.3;
                qreal x2 = x1 + step * 0.6;
                qreal y2 = y1 + bounds.height() * 0.4;
                auto* line = m_scene->addLine(x1, y1, x2, y2, ripplePen);
                line->setZValue(area.zOrder + 1);
            }
            // Shore highlight
            auto* shore = m_scene->addPolygon(screenPoly,
                QPen(QColor(150, 200, 240, 100), 1.5), Qt::NoBrush);
            shore->setZValue(area.zOrder + 2);

        } else if (area.type == "plaza") {
            // ── Plaza: light gray + tile grid ──
            QLinearGradient grad(bounds.topLeft(), bounds.bottomRight());
            grad.setColorAt(0.0, QColor(225, 222, 215, 200));
            grad.setColorAt(1.0, QColor(215, 212, 205, 200));
            auto* item = m_scene->addPolygon(screenPoly, Qt::NoPen, QBrush(grad));
            item->setZValue(area.zOrder);

            // Tile grid
            QPen gridPen(QColor(200, 195, 188, 60), 0.5);
            qreal step = 20;
            for (int i = -5; i <= 30; ++i) {
                qreal x1 = bounds.left() + i * step;
                auto* l1 = m_scene->addLine(x1, bounds.top(), x1, bounds.bottom(), gridPen);
                l1->setZValue(area.zOrder + 1);
                qreal y1 = bounds.top() + i * step;
                auto* l2 = m_scene->addLine(bounds.left(), y1, bounds.right(), y1, gridPen);
                l2->setZValue(area.zOrder + 1);
            }
            // Border
            auto* border = m_scene->addPolygon(screenPoly,
                QPen(QColor(190, 185, 178, 120), 1.2), Qt::NoBrush);
            border->setZValue(area.zOrder + 2);

        } else if (area.type == "sports_field") {
            // ── Sports field: dark green + white markings ──
            QLinearGradient grad(bounds.topLeft(), bounds.bottomRight());
            grad.setColorAt(0.0, QColor(80, 140, 70, 200));
            grad.setColorAt(0.5, QColor(70, 130, 60, 200));
            grad.setColorAt(1.0, QColor(90, 150, 80, 200));
            auto* item = m_scene->addPolygon(screenPoly, Qt::NoPen, QBrush(grad));
            item->setZValue(area.zOrder);

            // White lane markings (horizontal lines)
            QPen lanePen(QColor(255, 255, 255, 100), 0.8);
            int lanes = 6;
            for (int i = 0; i <= lanes; ++i) {
                qreal frac = static_cast<qreal>(i) / lanes;
                // Interpolate top and bottom edges
                QPointF leftPt = screenPoly[0] + frac * (screenPoly[3] - screenPoly[0]);
                QPointF rightPt = screenPoly[1] + frac * (screenPoly[2] - screenPoly[1]);
                auto* lane = m_scene->addLine(leftPt.x(), leftPt.y(),
                    rightPt.x(), rightPt.y(), lanePen);
                lane->setZValue(area.zOrder + 1);
            }
            // Border
            auto* border = m_scene->addPolygon(screenPoly,
                QPen(QColor(255, 255, 255, 80), 1.5), Qt::NoBrush);
            border->setZValue(area.zOrder + 2);

        } else if (area.type == "garden") {
            // ── Garden: green gradient + flower dots ──
            QLinearGradient grad(bounds.topLeft(), bounds.bottomRight());
            grad.setColorAt(0.0, QColor(120, 175, 90, 180));
            grad.setColorAt(0.5, QColor(110, 165, 80, 180));
            grad.setColorAt(1.0, QColor(130, 185, 100, 180));
            auto* item = m_scene->addPolygon(screenPoly, Qt::NoPen, QBrush(grad));
            item->setZValue(area.zOrder);

            // Flower dots
            auto rng = QRandomGenerator::global();
            for (int i = 0; i < 12; ++i) {
                qreal fx = bounds.left() + rng->bounded(static_cast<int>(bounds.width()));
                qreal fy = bounds.top() + rng->bounded(static_cast<int>(bounds.height()));
                QColor fc = QColor::fromHsv(rng->bounded(360), 160, 220, 160);
                auto* dot = m_scene->addEllipse(fx - 1.5, fy - 1.5, 3, 3,
                    Qt::NoPen, QBrush(fc));
                dot->setZValue(area.zOrder + 1);
            }
            // Border
            auto* border = m_scene->addPolygon(screenPoly,
                QPen(QColor(100, 150, 75, 80), 1), Qt::NoBrush);
            border->setZValue(area.zOrder + 2);
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// drawRoads — Bézier curve roads with weight-based width
// ═══════════════════════════════════════════════════════════════════════════

/// 计算贝塞尔曲线控制点：在逻辑坐标空间中，根据边的几何关系
/// 生成一个垂直于 AB 连线的偏移量，使道路呈现自然弯曲。
/// 偏移量与边权成反比（主干道更直，支路更弯）。
static QPointF bezierControlPoint(const Node& a, const Node& b,
                                  double weight, double maxWeight) {
    // AB 中点
    double mx = (a.x + b.x) / 2.0;
    double my = (a.y + b.y) / 2.0;

    // AB 方向的法向量（垂直方向）
    double dx = b.x - a.x;
    double dy = b.y - a.y;
    double len = std::sqrt(dx * dx + dy * dy);
    if (len < 1e-6) return QPointF(mx, my);

    double nx = -dy / len;  // 法向量 x
    double ny =  dx / len;  // 法向量 y

    // 弯曲幅度：与边权成反比，主干道几乎直，支路弯曲明显
    // weight/maxWeight ∈ (0,1]，越小越弯
    double curvature = 12.0 * (1.0 - 0.7 * weight / maxWeight);

    // 随机选择弯曲方向（左或右），使用节点 id 作为确定性种子
    double sign = ((a.id + b.id) % 2 == 0) ? 1.0 : -1.0;

    return QPointF(mx + nx * curvature * sign,
                   my + ny * curvature * sign);
}

void MapView::drawRoads() {
    const auto nodes = m_graph->allNodes();

    // 找到最大边权用于归一化
    double maxW = 0;
    for (const auto& n : nodes)
        for (const auto& e : m_graph->getEdges(n.id))
            maxW = qMax(maxW, e.weight);
    if (maxW < 1) maxW = 1;

    for (const auto& n : nodes) {
        for (const auto& e : m_graph->getEdges(n.id)) {
            if (e.from >= e.to) continue;
            const Node& a = m_graph->node(e.from);
            const Node& b = m_graph->node(e.to);

            QPointF pa = iso(a.x, a.y);
            QPointF pb = iso(b.x, b.y);

            // 贝塞尔控制点（在逻辑坐标中计算，再投影到屏幕）
            QPointF cpLogic = bezierControlPoint(a, b, e.weight, maxW);
            QPointF cp = iso(cpLogic.x(), cpLogic.y());

            // 构建二次贝塞尔路径
            QPainterPath path(pa);
            path.quadTo(cp, pb);

            // 道路分级：footpath vs main_road
            bool isFootpath = (e.type == "footpath");
            bool isMain = !isFootpath && (e.weight >= 150);

            qreal wShadow, wBase, wSurf, wCenter;
            QColor baseColor, surfColor, centerClr;
            int baseZ;

            if (isFootpath) {
                // 步行小路：窄，暖色调
                wShadow = 6;  wBase = 4;  wSurf = 2.5;  wCenter = 1;
                baseColor = QColor(185, 175, 155);
                surfColor = QColor(210, 200, 180);
                centerClr = QColor(225, 218, 200);
                baseZ = -53;
            } else if (isMain) {
                // 主干道：宽
                wShadow = 16; wBase = 12; wSurf = 8;    wCenter = 3;
                baseColor = QColor(155, 150, 140);
                surfColor = QColor(210, 205, 195);
                centerClr = QColor(228, 224, 216);
                baseZ = -55;
            } else {
                // 次要道路
                wShadow = 10; wBase = 7;  wSurf = 4;    wCenter = 1.5;
                baseColor = QColor(175, 170, 162);
                surfColor = QColor(220, 218, 210);
                centerClr = QColor(232, 230, 225);
                baseZ = -55;
            }

            // 1. 阴影
            QPen shadowPen(QColor(0, 0, 0, 28), wShadow);
            shadowPen.setCapStyle(Qt::RoundCap);
            auto* s1 = m_scene->addPath(path, shadowPen);
            s1->setZValue(baseZ);

            // 2. 路沿（深色边框）
            QPen basePen(baseColor, wBase);
            basePen.setCapStyle(Qt::RoundCap);
            auto* s2 = m_scene->addPath(path, basePen);
            s2->setZValue(baseZ + 5);

            // 3. 路面
            QPen surfPen(surfColor, wSurf);
            surfPen.setCapStyle(Qt::RoundCap);
            auto* s3 = m_scene->addPath(path, surfPen);
            s3->setZValue(baseZ + 6);

            // 4. 中心高光
            QPen centerPen(centerClr, wCenter);
            centerPen.setCapStyle(Qt::RoundCap);
            auto* s4 = m_scene->addPath(path, centerPen);
            s4->setZValue(baseZ + 7);

            // 5. 中心标线
            QPen dashPen(QColor(255, 255, 230, 100), 1,
                         isFootpath ? Qt::DashLine : Qt::SolidLine);
            dashPen.setDashPattern(isFootpath ? QVector<qreal>{3, 6}
                                              : QVector<qreal>{5, 8});
            dashPen.setCapStyle(Qt::RoundCap);
            auto* s5 = m_scene->addPath(path, dashPen);
            s5->setZValue(baseZ + 8);
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

        // 注册标签到 LabelManager（优先级 = 建筑高度 × 10 + 边数）
        qreal edgeCount = m_graph->getEdges(n.id).size();
        qreal priority = c.h * 10 + edgeCount;
        QPointF labelPos = pos + QPointF(0, c.depth * 0.25 + 10);
        m_labelMgr->registerLabel(n.id, n.name, labelPos, priority, c.h);
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// drawShadows — semi-transparent ellipse shadows under buildings
// ═══════════════════════════════════════════════════════════════════════════

void MapView::drawShadows() {
    const auto nodes = m_graph->allNodes();
    for (const auto& n : nodes) {
        auto c = cfgFor(n.name);
        QPointF pos = iso(n.x, n.y);

        // 等轴测椭圆阴影（扁平，位于建筑底部）
        qreal rx = c.w * 0.6;          // 水平半径
        qreal ry = c.depth * 0.3;      // 垂直半径（等轴测压缩）
        qreal offsetY = c.depth * 0.25; // 向下偏移到建筑底部

        auto* shadow = m_scene->addEllipse(
            pos.x() - rx, pos.y() + offsetY - ry / 2,
            rx * 2, ry,
            Qt::NoPen,
            QBrush(QColor(0, 0, 0, 35)));
        shadow->setZValue(depthSort(n.x, n.y) + 8);
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

    // ── Flower beds near buildings ──
    for (const auto& n : nodes) {
        auto rng2 = QRandomGenerator::global();
        int beds = 1 + rng2->bounded(2);
        for (int b = 0; b < beds; ++b) {
            qreal bx = n.x + (rng2->bounded(40) - 20);
            qreal by = n.y + (rng2->bounded(40) - 20);
            QPointF bp = iso(bx, by);

            // Flower bed — small colored ellipse cluster
            auto* bed = m_scene->addEllipse(
                bp.x() - 6, bp.y() - 3, 12, 6,
                Qt::NoPen,
                QBrush(QColor(120 + rng2->bounded(30),
                              180 + rng2->bounded(30),
                              80 + rng2->bounded(20), 100)));
            bed->setZValue(depthSort(bx, by) + 3);

            // Small colored dots (flowers)
            for (int f = 0; f < 3; ++f) {
                QColor fc = QColor::fromHsv(
                    rng2->bounded(360), 180, 220, 180);
                auto* dot = m_scene->addEllipse(
                    bp.x() - 4 + f * 3 + rng2->bounded(2),
                    bp.y() - 1 + rng2->bounded(3),
                    2.5, 2.5, Qt::NoPen, QBrush(fc));
                dot->setZValue(depthSort(bx, by) + 4);
            }
        }
    }

    // ── Central fountain (near library or main teaching building) ──
    for (const auto& n : nodes) {
        if (n.name.contains("图书馆") || n.name.contains("主教")) {
            QPointF fp = iso(n.x + 25, n.y + 15);

            // Fountain base (isometric diamond)
            QPolygonF base;
            base << QPointF(fp.x(), fp.y() - 8)
                 << QPointF(fp.x() - 12, fp.y())
                 << QPointF(fp.x(), fp.y() + 8)
                 << QPointF(fp.x() + 12, fp.y());
            auto* fb = m_scene->addPolygon(
                base, QPen(QColor(150, 160, 170), 1),
                QBrush(QColor(180, 200, 220, 180)));
            fb->setZValue(depthSort(n.x + 25, n.y + 15) + 2);

            // Water (inner diamond)
            QPolygonF water;
            water << QPointF(fp.x(), fp.y() - 5)
                  << QPointF(fp.x() - 8, fp.y())
                  << QPointF(fp.x(), fp.y() + 5)
                  << QPointF(fp.x() + 8, fp.y());
            auto* wt = m_scene->addPolygon(
                water, Qt::NoPen,
                QBrush(QColor(120, 180, 230, 160)));
            wt->setZValue(depthSort(n.x + 25, n.y + 15) + 3);

            // Spray (small white dots)
            for (int s = 0; s < 5; ++s) {
                auto* sp = m_scene->addEllipse(
                    fp.x() - 2 + s * 1.5, fp.y() - 6 - s * 1.5,
                    1.5, 1.5, Qt::NoPen,
                    QBrush(QColor(255, 255, 255, 150)));
                sp->setZValue(depthSort(n.x + 25, n.y + 15) + 4);
            }
            break;
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// drawPath — gradient path with glow
// ═══════════════════════════════════════════════════════════════════════════

void MapView::drawPath(const QVector<int>& path) {
    if (path.size() < 2 || !m_graph) return;

    // 找到路径上的最大边权
    double maxW = 0;
    for (int i = 0; i + 1 < path.size(); ++i) {
        for (const auto& e : m_graph->getEdges(path[i])) {
            if (e.to == path[i + 1]) { maxW = qMax(maxW, e.weight); break; }
        }
    }
    if (maxW < 1) maxW = 1;

    QPainterPath isoPath;
    QPointF p0 = iso(m_graph->node(path.first()).x,
                     m_graph->node(path.first()).y);
    isoPath.moveTo(p0);

    for (int i = 1; i < path.size(); ++i) {
        const Node& cur = m_graph->node(path[i]);
        QPointF pi = iso(cur.x, cur.y);

        if (i + 1 < path.size()) {
            // 获取当前段的边权
            double w = 100;
            for (const auto& e : m_graph->getEdges(path[i - 1])) {
                if (e.to == path[i]) { w = e.weight; break; }
            }
            const Node& prev = m_graph->node(path[i - 1]);
            const Node& next = m_graph->node(path[i + 1]);
            QPointF cpLogic = bezierControlPoint(prev, cur, w, maxW);
            QPointF cp = iso(cpLogic.x(), cpLogic.y());
            isoPath.quadTo(cp, pi);
        } else {
            isoPath.lineTo(pi);
        }
        p0 = pi;
    }

    // Layer 0: wide gradient aura
    QPainterPathStroker auraStroker;
    auraStroker.setWidth(22);
    auraStroker.setCapStyle(Qt::RoundCap);
    QPainterPath auraPath = auraStroker.createStroke(isoPath);
    QLinearGradient auraGrad(auraPath.boundingRect().topLeft(),
                             auraPath.boundingRect().bottomRight());
    auraGrad.setColorAt(0.0, QColor(255, 120, 50, 25));
    auraGrad.setColorAt(0.5, QColor(255, 80, 30, 45));
    auraGrad.setColorAt(1.0, QColor(255, 120, 50, 25));
    auto* l0 = m_scene->addPath(auraPath, Qt::NoPen, QBrush(auraGrad));
    l0->setZValue(28); m_pathItems.append(l0);

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

    // ── 3A: 蚂蚁线（流动虚线轮廓）──
    QPainterPathStroker antsStroker;
    antsStroker.setWidth(18);
    antsStroker.setCapStyle(Qt::RoundCap);
    QPainterPath antsOutline = antsStroker.createStroke(isoPath);

    QPen antsPen(QColor(255, 255, 255, 160), 1.5, Qt::CustomDashLine);
    antsPen.setDashPattern({8, 10});  // 8px 实线 + 10px 间隔
    antsPen.setDashOffset(0);
    m_antsItem = m_scene->addPath(antsOutline, antsPen);
    m_antsItem->setZValue(34);
    m_pathItems.append(m_antsItem);
    m_antsOffset = 0;
    m_antsTimer->start();

    // ── 3B: 非焦点遮罩（半透明黑色覆盖，路径区域挖孔）──
    // 遮罩 = 全场景矩形 - 路径扩展区域
    QRectF sceneR = m_scene->sceneRect();
    QPainterPath maskPath;
    maskPath.addRect(sceneR);

    // 路径扩展区域（比路径宽一些，保留可视区域）
    QPainterPathStroker maskStroker;
    maskStroker.setWidth(60);
    maskStroker.setCapStyle(Qt::RoundCap);
    QPainterPath pathZone = maskStroker.createStroke(isoPath);

    // 用路径区域"挖孔"：全场景 - 路径区域 = 遮罩
    QPainterPath maskWithHole = maskPath.subtracted(pathZone);

    m_maskItem = m_scene->addPath(
        maskWithHole, Qt::NoPen,
        QBrush(QColor(0, 0, 0, 100)));
    m_maskItem->setZValue(25);  // 低于路径层，高于道路层
    m_pathItems.append(m_maskItem);

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

    // 构建贝塞尔路径上的采样点序列
    QVector<QPointF> waypoints;
    for (int i = 0; i + 1 < path.size(); ++i) {
        const Node& a = m_graph->node(path[i]);
        const Node& b = m_graph->node(path[i + 1]);
        QPointF pa = iso(a.x, a.y);
        QPointF pb = iso(b.x, b.y);

        // 获取边权以计算控制点
        double w = 100;
        for (const auto& e : m_graph->getEdges(path[i])) {
            if (e.to == path[i + 1]) { w = e.weight; break; }
        }
        double maxW = 1;
        for (int j = 0; j + 1 < path.size(); ++j)
            for (const auto& e : m_graph->getEdges(path[j]))
                if (e.to == path[j + 1]) maxW = qMax(maxW, e.weight);

        QPointF cpLogic = bezierControlPoint(a, b, w, maxW);
        QPointF cp = iso(cpLogic.x(), cpLogic.y());

        // 在二次贝塞尔曲线上采样 10 个点
        // B(t) = (1-t)²·P0 + 2(1-t)t·P1 + t²·P2
        int samples = 10;
        for (int s = 1; s <= samples; ++s) {
            qreal t = static_cast<qreal>(s) / samples;
            qreal u = 1.0 - t;
            QPointF pt = u * u * pa + 2 * u * t * cp + t * t * pb;
            waypoints.append(pt);
        }
    }

    // 用 QSequentialAnimationGroup 沿采样点移动
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

// ═══════════════════════════════════════════════════════════════════════════
// clearPath
// ═══════════════════════════════════════════════════════════════════════════

void MapView::clearPath() {
    // 停止蚂蚁线动画
    if (m_antsTimer) { m_antsTimer->stop(); }
    m_antsItem = nullptr;  // 已在 m_pathItems 中，下面统一删除

    if (m_animGroup) { m_animGroup->stop(); delete m_animGroup; m_animGroup = nullptr; }
    if (m_ball) { m_scene->removeItem(m_ball); delete m_ball; m_ball = nullptr; }
    for (auto* item : m_pathItems) { m_scene->removeItem(item); delete item; }
    m_pathItems.clear();
    m_maskItem = nullptr;  // 已在 m_pathItems 中
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

QPointF MapView::nodeScreenPos(int id) const {
    if (!m_graph || !m_graph->hasNode(id)) return {};
    const Node& n = m_graph->node(id);
    QPointF scenePos = iso(n.x, n.y);
    auto* bldg = m_buildings.value(id);
    if (bldg) {
        auto c = cfgFor(n.name);
        scenePos = bldg->pos() + QPointF(0, -c.depth * 0.5 - 20);
    }
    return mapFromScene(scenePos);
}

// ═══════════════════════════════════════════════════════════════════════════
// updateLabelsAndLOD — 标签碰撞检测 + LOD 随缩放联动
// ═══════════════════════════════════════════════════════════════════════════

void MapView::updateLabelsAndLOD() {
    if (!m_labelMgr) return;
    qreal zoom = transform().m11();
    m_labelMgr->updateLabels(zoom);

    // LOD：远处隐藏次要道路的视觉细节（已由 drawRoads 的宽度分级自然实现）
    // 可在此扩展：当 zoom < 0.3 时隐藏树/草等装饰图层
    const auto items = m_scene->items();
    for (auto* item : items) {
        if (item->zValue() == -90) {  // 草地斑块
            item->setVisible(zoom > 0.4);
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Zoom
// ═══════════════════════════════════════════════════════════════════════════

void MapView::zoomIn() {
    if (transform().m11() * ZOOM_FACTOR <= MAX_SCALE) {
        scale(ZOOM_FACTOR, ZOOM_FACTOR);
        updateLabelsAndLOD();
    }
}

void MapView::zoomOut() {
    if (transform().m11() / ZOOM_FACTOR >= MIN_SCALE) {
        scale(1.0 / ZOOM_FACTOR, 1.0 / ZOOM_FACTOR);
        updateLabelsAndLOD();
    }
}

void MapView::fitMap() {
    if (m_scene) {
        fitInView(m_scene->sceneRect().adjusted(-50, -50, 50, 50), Qt::KeepAspectRatio);
        updateLabelsAndLOD();
    }
}

void MapView::wheelEvent(QWheelEvent* event) {
    double factor = (event->angleDelta().y() > 0) ? ZOOM_FACTOR : (1.0 / ZOOM_FACTOR);
    if (transform().m11() * factor < MIN_SCALE || transform().m11() * factor > MAX_SCALE)
        return;
    scale(factor, factor);
    updateLabelsAndLOD();
}
