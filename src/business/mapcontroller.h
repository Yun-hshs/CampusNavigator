#ifndef MAPCONTROLLER_H
#define MAPCONTROLLER_H

#include <QObject>
#include <QVector>
#include "building.h"
#include "road.h"
#include "node.h"
#include "graph.h"
#include "environment.h"
#include "maplayer.h"

class JsonParser;
class DatabaseManager;
class QGraphicsScene;

class MapController : public QObject {
    Q_OBJECT
public:
    explicit MapController(QObject *parent = nullptr);

    bool loadFromJson(const QString &filePath);
    bool loadFromDatabase(DatabaseManager *dbManager);
    bool saveToDatabase(DatabaseManager *dbManager);

    void setBackgroundImage(const QString &path);
    void buildScene(QGraphicsScene *scene);

    // Calibration: 2-point scale+translate transform
    void applyCalibration(QPointF old1, QPointF new1,
                          QPointF old2, QPointF new2);
    QString exportCalibratedJson() const;

    Building* findBuilding(int id);
    Building* findBuildingByName(const QString &name);
    Node* findNode(int id);
    QPointF nodeCoord(int nodeId);

    const QVector<Building> &buildings() const;
    const QVector<Node> &nodes() const;
    const QVector<EnvironmentArea> &environments() const;
    const Graph &graph() const;

signals:
    void mapLoaded();
    void mapLoadError(const QString &msg);
    void buildingSelected(const Building &b);

private:
    void buildGraphFromData();
    void drawBackground(QGraphicsScene *scene);
    void drawEnvironment(QGraphicsScene *scene);
    void drawRoad(QGraphicsScene *scene, const Road &road);
    void drawBuildingItem(QGraphicsScene *scene, const Building &b);
    void drawBuildingLabel(QGraphicsScene *scene, const Building &b);
    void drawNodes(QGraphicsScene *scene);

    QVector<Building> m_buildings;
    QVector<Road> m_roads;
    QVector<Node> m_nodes;
    QVector<EnvironmentArea> m_environments;
    Graph m_graph;
    QString m_bgImagePath;
    QSizeF m_bgImageSize;
};

#endif // MAPCONTROLLER_H