#pragma once
#include <QGraphicsScene>
#include <QHash>
#include <QVector>
#include "src/algorithm/graph.h"
#include "src/ui/buildingitem.h"
#include "src/ui/roaditem.h"

class MapScene : public QGraphicsScene {
    Q_OBJECT
public:
    explicit MapScene(QObject* parent = nullptr);

    void loadMap(const QVector<Building>& buildings, const QVector<Road>& roads);

    void highlightPath(const QVector<int>& buildingIds, const QVector<QPointF>& pathPoints);
    void clearHighlight();
    void highlightBuilding(int id, bool on);
    void selectAndCenterBuilding(int id);

    const Graph& graph() const { return m_graph; }
    BuildingItem* buildingItem(int id) const { return m_buildingItems.value(id); }

signals:
    void buildingClicked(int id, const QString& name, const QString& desc);

private:
    void rebuildItems();

    QVector<Building> m_buildings;
    QVector<Road>     m_roads;

    Graph m_graph;

    QHash<int, BuildingItem*> m_buildingItems;
    QVector<RoadItem*> m_roadItems;
    QGraphicsPathItem* m_pathOverlay = nullptr;
};
