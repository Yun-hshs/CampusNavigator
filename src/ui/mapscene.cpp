#include "mapscene.h"
#include <QPainterPath>
#include <QPen>
#include <QBrush>

MapScene::MapScene(QObject* parent)
    : QGraphicsScene(parent)
{
    setBackgroundBrush(QBrush(QColor(232, 240, 248)));
}

void MapScene::loadMap(const QVector<Building>& buildings, const QVector<Road>& roads) {
    m_buildings = buildings;
    m_roads     = roads;

    double maxX = 0, maxY = 0;
    for (const auto& b : buildings) {
        maxX = qMax(maxX, b.x + b.w);
        maxY = qMax(maxY, b.y + b.h);
    }
    setSceneRect(-60, -60, maxX + 120, maxY + 120);

    m_graph.build(buildings, roads);
    rebuildItems();
}

void MapScene::highlightPath(const QVector<int>& buildingIds, const QVector<QPointF>& pathPoints) {
    clearHighlight();

    for (int id : buildingIds) {
        if (auto* b = m_buildingItems.value(id))
            b->setHighlighted(true);
    }

    if (pathPoints.size() >= 2) {
        QPainterPath pp;
        pp.moveTo(pathPoints[0]);
        for (int i = 1; i < pathPoints.size(); ++i)
            pp.lineTo(pathPoints[i]);

        m_pathOverlay = new QGraphicsPathItem();
        m_pathOverlay->setPath(pp);
        QPen p(QColor(240, 40, 40), 5);
        p.setCapStyle(Qt::RoundCap);
        p.setJoinStyle(Qt::RoundJoin);
        m_pathOverlay->setPen(p);
        m_pathOverlay->setBrush(Qt::NoBrush);
        m_pathOverlay->setZValue(5);
        addItem(m_pathOverlay);
    }
}

void MapScene::clearHighlight() {
    if (m_pathOverlay) {
        removeItem(m_pathOverlay);
        delete m_pathOverlay;
        m_pathOverlay = nullptr;
    }
    for (auto* r : m_roadItems)
        r->resetHighlight();
    for (auto* b : m_buildingItems)
        b->setHighlighted(false);
}

void MapScene::highlightBuilding(int id, bool on) {
    if (auto* b = m_buildingItems.value(id))
        b->setHighlighted(on);
}

void MapScene::selectAndCenterBuilding(int id) {
    auto* item = m_buildingItems.value(id);
    if (!item) return;
    clearSelection();
    item->setSelected(true);
    item->setHighlighted(true);
    emit buildingClicked(item->buildingId(), item->buildingName(),
                         item->buildingDesc());
}

void MapScene::rebuildItems() {
    clear();
    m_buildingItems.clear();
    m_roadItems.clear();
    m_pathOverlay = nullptr;

    for (const auto& road : m_roads) {
        if (road.path.size() < 2) continue;
        auto* item = new RoadItem(road.path, road.type, road.name);
        addItem(item);
        m_roadItems.append(item);
    }

    for (const auto& b : m_buildings) {
        auto* item = new BuildingItem(b.id, b.name, b.type, b.desc,
                                      b.x, b.y, b.w, b.h);
        addItem(item);
        m_buildingItems[b.id] = item;
    }

    connect(this, &QGraphicsScene::selectionChanged, this, [this]() {
        auto selected = selectedItems();
        for (auto* item : m_buildingItems)
            item->setHighlighted(false);

        if (!selected.isEmpty()) {
            auto* bi = dynamic_cast<BuildingItem*>(selected.first());
            if (bi) {
                bi->setHighlighted(true);
                emit buildingClicked(bi->buildingId(), bi->buildingName(),
                                     bi->buildingDesc());
            }
        }
    });
}
