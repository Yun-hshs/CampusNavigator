#include "MapView.h"
#include "PathVisualizer.h"
#include "RoadRenderer.h"

#include "RenderContext.h"
#include "graph/Graph.h"

#include <QGraphicsScene>
#include <QGraphicsPathItem>
#include <QWheelEvent>
#include <QPainter>
#include <algorithm>

// ═══════════════════════════════════════════════════════════════════════════
// IsometricBuilding — pixmap-based 2.5D building
// ═══════════════════════════════════════════════════════════════════════════

IsometricBuilding::IsometricBuilding(int nodeId, const QString& name,
                                     const QPixmap& basePixmap,
                                     const QPixmap& overlayPixmap,
                                     QGraphicsItem* parent)
    : QGraphicsObject(parent)
    , m_nodeId(nodeId), m_name(name)
    , m_basePixmap(basePixmap), m_overlayPixmap(overlayPixmap)
{
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable);
    setZValue(10);
}

QRectF IsometricBuilding::boundingRect() const {
    if (!m_basePixmap.isNull())
        return {0, 0, static_cast<qreal>(m_basePixmap.width()),
                     static_cast<qreal>(m_basePixmap.height())};
    return {0, 0, 60, 80};
}

void IsometricBuilding::paint(QPainter* painter,
                              const QStyleOptionGraphicsItem*, QWidget*) {
    painter->setRenderHint(QPainter::SmoothPixmapTransform);

    if (!m_basePixmap.isNull())
        painter->drawPixmap(0, 0, m_basePixmap);
    else {
        painter->setBrush(QColor(180, 180, 180));
        painter->setPen(QPen(QColor(120, 120, 120), 1));
        painter->drawRect(boundingRect());
    }

    if (!m_overlayPixmap.isNull())
        painter->drawPixmap(0, 0, m_overlayPixmap);

    if (m_highlighted) {
        painter->setOpacity(0.3);
        painter->fillRect(boundingRect(), m_hlColor);
        painter->setOpacity(1.0);
    }

    if (m_hovered && !m_highlighted) {
        painter->setPen(QPen(QColor(255, 255, 255, 120), 2));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(boundingRect().adjusted(1, 1, -1, -1));
    }
}

void IsometricBuilding::setHighlighted(bool on, const QColor& color) {
    m_highlighted = on;
    m_hlColor = color;
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
// VectorBuilding — shape-based 2D building
// ═══════════════════════════════════════════════════════════════════════════

VectorBuilding::VectorBuilding(int nodeId, const QString& name,
                               QGraphicsItem* parent)
    : QGraphicsObject(parent), m_nodeId(nodeId), m_name(name)
{
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable);
    setZValue(10);
}

QRectF VectorBuilding::boundingRect() const {
    return {0, 0, 36, 28};
}

void VectorBuilding::paint(QPainter* painter,
                           const QStyleOptionGraphicsItem*, QWidget*) {
    QRectF r = boundingRect();
    QColor fill = m_highlighted ? m_hlColor : QColor(100, 149, 237);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(fill.darker(130), 1.2));
    painter->setBrush(fill.lighter(120));
    painter->drawRoundedRect(r, 5, 5);

    QFont f("Microsoft YaHei", 7, QFont::Bold);
    painter->setFont(f);
    painter->setPen(Qt::white);
    QString label = m_name.length() > 4 ? m_name.left(4) : m_name;
    painter->drawText(r, Qt::AlignCenter, label);

    if (m_hovered && !m_highlighted) {
        painter->setPen(QPen(Qt::white, 2));
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(r.adjusted(-1, -1, 1, 1), 5, 5);
    }
}

void VectorBuilding::setHighlighted(bool on, const QColor& color) {
    m_highlighted = on;
    m_hlColor = color;
    update();
}

void VectorBuilding::mousePressEvent(QGraphicsSceneMouseEvent* e) {
    QGraphicsObject::mousePressEvent(e);
    emit clicked(m_nodeId);
}

void VectorBuilding::hoverEnterEvent(QGraphicsSceneHoverEvent*) {
    m_hovered = true;
    update();
}

void VectorBuilding::hoverLeaveEvent(QGraphicsSceneHoverEvent*) {
    m_hovered = false;
    update();
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
    initPixmaps();
    m_pathViz = new PathVisualizer(this);
}

void MapView::setupScene() {
    m_scene = new QGraphicsScene(this);
    m_scene->setBackgroundBrush(QColor(215, 225, 200));
    setScene(m_scene);
    setStyleSheet("border: none;");
    m_labelMgr = new LabelManager(m_scene);

    m_lodTimer = new QTimer(this);
    m_lodTimer->setSingleShot(true);
    m_lodTimer->setInterval(80);
    connect(m_lodTimer, &QTimer::timeout, this, [this]() {
        qreal zoom = transform().m11();
        if (qAbs(zoom - m_lastLabelZoom) >= 0.02) {
            m_lastLabelZoom = zoom;
            if (m_labelMgr) m_labelMgr->updateLabels(zoom);
        }
    });
}

// ═══════════════════════════════════════════════════════════════════════════
// initPixmaps — preload all sprites from Qt resources
// ═══════════════════════════════════════════════════════════════════════════

void MapView::initPixmaps() {
    m_pixCache["main_teach"]  = QPixmap(":/buildings/buildingTiles_020.png");
    m_pixCache["lab"]         = QPixmap(":/buildings/buildingTiles_060.png");
    m_pixCache["dormitory"]   = QPixmap(":/buildings/buildingTiles_040.png");
    m_pixCache["gate"]        = QPixmap(":/buildings/buildingTiles_120.png");
    m_pixCache["default"]     = QPixmap(":/buildings/buildingTiles_000.png");
    m_pixCache["decoration"]  = QPixmap(":/buildings/buildingTiles_128.png");
}

// ═══════════════════════════════════════════════════════════════════════════
// setGraph, iso
// ═══════════════════════════════════════════════════════════════════════════

void MapView::setGraph(Graph* graph) { m_graph = graph; }

void MapView::setRenderMode(RenderMode mode) {
    if (m_renderMode == mode) return;
    m_renderMode = mode;
    drawMap();
}

QPointF MapView::iso(qreal x, qreal y) const {
    return m_geo.toScreen(x * m_logicScale, y * m_logicScale);
}

// ═══════════════════════════════════════════════════════════════════════════
// drawMap — main entry with Y-sorting
// ═══════════════════════════════════════════════════════════════════════════

void MapView::drawMap() {
    if (!m_graph) return;
    clearPath();
    m_scene->clear();
    m_buildings.clear();
    m_vectorBuildings.clear();
    m_labelMgr = new LabelManager(m_scene);

    // Configure GeoTransform
    const auto nodes = m_graph->allNodes();
    double lxMin = 1e9, lyMin = 1e9, lxMax = -1e9, lyMax = -1e9;
    for (const auto& n : nodes) {
        lxMin = qMin(lxMin, n.x); lyMin = qMin(lyMin, n.y);
        lxMax = qMax(lxMax, n.x); lyMax = qMax(lyMax, n.y);
    }
    double logicW = (lxMax - lxMin) * m_logicScale;
    double mpp = logicW / 800.0;
    if (mpp < 0.01) mpp = 0.5;

    if (m_renderMode == RenderMode::Flat2D) {
        // Origin places north (max logical y) at screen top, campus centered horizontally
        double originY = lyMax * m_logicScale;
        double originX = -logicW / 2.0;
        m_geo.configureFlat2D(mpp, QPointF(originX, originY));
        m_scene->setBackgroundBrush(QColor(235, 240, 230));
    } else {
        m_geo.configure(mpp, QPointF(0, 0));
    }

    // Build render context
    RenderContext ctx;
    ctx.scene = m_scene;
    ctx.graph = m_graph;
    ctx.iso = [this](qreal x, qreal y) { return iso(x, y); };
    ctx.mode = m_renderMode;

    // Draw layers
    if (m_renderMode == RenderMode::Flat2D)
        drawRealMapBackground(lxMin, lyMin, lxMax, lyMax);
    RoadRenderer::draw(ctx);

    // Draw buildings — Y-sorted
    struct Drawable { int nodeId; QPointF sp; };
    QVector<Drawable> drawables;
    for (const auto& n : nodes)
        drawables.append({n.id, iso(n.x, n.y)});
    std::sort(drawables.begin(), drawables.end(),
        [](const Drawable& a, const Drawable& b){ return a.sp.y() < b.sp.y(); });

    if (m_renderMode == RenderMode::Isometric) {
        QPixmap basePix = m_pixCache.value("default");
        for (const auto& d : drawables) {
            const auto& node = m_graph->node(d.nodeId);
            QPixmap overlayPix = m_pixCache.value(node.sprite);
            auto* bldg = new IsometricBuilding(d.nodeId, node.name, basePix, overlayPix);
            qreal bw = bldg->boundingRect().width();
            qreal bh = bldg->boundingRect().height();
            bldg->setPos(d.sp.x() - bw / 2, d.sp.y() - bh);
            bldg->setZValue(10 + d.sp.y() * 0.01);
            m_scene->addItem(bldg);
            m_buildings[d.nodeId] = bldg;
            m_labelMgr->registerLabel(d.nodeId, node.name,
                bldg->pos() + QPointF(bw / 2, -12), 1.0, bh);
            connect(bldg, &IsometricBuilding::clicked, this, &MapView::nodeClicked);
        }
    } else {
        for (const auto& d : drawables) {
            const auto& node = m_graph->node(d.nodeId);
            auto* bldg = new VectorBuilding(d.nodeId, node.name);
            qreal bw = bldg->boundingRect().width();
            qreal bh = bldg->boundingRect().height();
            bldg->setPos(d.sp.x() - bw / 2, d.sp.y() - bh / 2);
            bldg->setZValue(10 + d.sp.y() * 0.01);
            m_scene->addItem(bldg);
            m_vectorBuildings[d.nodeId] = bldg;
            m_labelMgr->registerLabel(d.nodeId, node.name,
                bldg->pos() + QPointF(bw / 2, -12), 1.0, bh);
            connect(bldg, &VectorBuilding::clicked, this, &MapView::nodeClicked);
        }
    }

    updateLabelsAndLOD();
    fitInView(m_scene->itemsBoundingRect().adjusted(-100, -100, 100, 100), Qt::KeepAspectRatio);
}



bool MapView::drawRealMapBackground(double minX, double minY, double maxX, double maxY) {
    QPixmap realMap(":/assets/campus_map.png");
    if (realMap.isNull()) return false;

    constexpr qreal pad = 120.0;
    QPointF tl = iso(minX, minY) + QPointF(-pad, -pad);
    QPointF br = iso(maxX, maxY) + QPointF(pad, pad);
    QRectF targetRect(tl, br);

    auto* mapItem = m_scene->addPixmap(realMap.scaled(targetRect.size().toSize(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    mapItem->setPos(targetRect.topLeft());
    mapItem->setOpacity(0.72);
    mapItem->setZValue(-120);

    auto* mask = m_scene->addRect(targetRect, Qt::NoPen, QColor(255, 255, 255, 36));
    mask->setZValue(-119);
    return true;
}

// ═══════════════════════════════════════════════════════════════════════════
// Path delegation
// ═══════════════════════════════════════════════════════════════════════════

void MapView::drawPath(const QVector<int>& path) {
    RenderContext ctx{m_scene, m_graph,
                      [this](qreal x, qreal y) { return iso(x, y); },
                      m_renderMode};
    m_pathViz->drawPath(ctx, path, BALL_RADIUS);
}

void MapView::animatePath(const QVector<int>& path) {
    RenderContext ctx{m_scene, m_graph,
                      [this](qreal x, qreal y) { return iso(x, y); },
                      m_renderMode};
    m_pathViz->animatePath(ctx, path, BALL_RADIUS);
}

void MapView::clearPath() {
    m_pathViz->clearPath(m_scene, m_buildings, m_vectorBuildings);
}

// ═══════════════════════════════════════════════════════════════════════════
// highlight / center / nodeScreenPos
// ═══════════════════════════════════════════════════════════════════════════

void MapView::highlightStartEnd(int startId, int endId) {
    // Helper to un-highlight a node in whichever building map it belongs to
    auto unhighlight = [&](int id) {
        if (auto* b = m_buildings.value(id)) b->setHighlighted(false);
        if (auto* b = m_vectorBuildings.value(id)) b->setHighlighted(false);
    };
    auto highlight = [&](int id, const QColor& color) {
        if (auto* b = m_buildings.value(id)) b->setHighlighted(true, color);
        if (auto* b = m_vectorBuildings.value(id)) b->setHighlighted(true, color);
    };

    if (m_pathViz->startNodeId() >= 0) unhighlight(m_pathViz->startNodeId());
    if (m_pathViz->endNodeId() >= 0) unhighlight(m_pathViz->endNodeId());
    m_pathViz->setStartNodeId(startId);
    m_pathViz->setEndNodeId(endId);
    highlight(startId, QColor(50, 200, 80));
    highlight(endId, QColor(220, 50, 50));
}

void MapView::centerOnNode(int id) {
    if (!m_graph || !m_graph->hasNode(id)) return;
    const Node& n = m_graph->node(id);
    centerOn(iso(n.x, n.y));
}

void MapView::highlightNode(int id, const QColor& color) {
    if (auto* b = m_buildings.value(id)) b->setHighlighted(true, color);
    if (auto* b = m_vectorBuildings.value(id)) b->setHighlighted(true, color);
}

QPointF MapView::nodeScreenPos(int id) const {
    if (!m_graph || !m_graph->hasNode(id)) return {};
    const Node& n = m_graph->node(id);
    QPointF scenePos = iso(n.x, n.y);
    if (auto* bldg = m_buildings.value(id)) {
        scenePos = bldg->pos() + QPointF(bldg->boundingRect().width() / 2, -20);
    } else if (auto* bldg = m_vectorBuildings.value(id)) {
        scenePos = bldg->pos() + QPointF(bldg->boundingRect().width() / 2, -12);
    }
    return mapFromScene(scenePos);
}

// ═══════════════════════════════════════════════════════════════════════════
// updateLabelsAndLOD / Zoom
// ═══════════════════════════════════════════════════════════════════════════

void MapView::updateLabelsAndLOD() {
    if (!m_labelMgr) return;
    qreal zoom = transform().m11();
    if (qAbs(zoom - m_lastLabelZoom) < 0.02) return;
    m_lastLabelZoom = zoom;
    m_labelMgr->updateLabels(zoom);
}

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
    m_lodTimer->start();
}
