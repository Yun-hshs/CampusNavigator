#include "mapcontroller.h"
#include "jsonparser.h"
#include "databasemanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QSqlQuery>
#include <QSqlError>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsPathItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>
#include <QPainterPath>
#include <QImage>
#include <QDebug>

MapController::MapController(QObject *parent)
    : QObject(parent)
{
}

void MapController::setBackgroundImage(const QString &path)
{
    m_bgImagePath = path;
    QImage img(path);
    if (!img.isNull()) {
        m_bgImageSize = img.size();
    }
}

bool MapController::loadFromJson(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit mapLoadError("Cannot open file: " + filePath);
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (doc.isNull()) {
        emit mapLoadError("Invalid JSON in file: " + filePath);
        return false;
    }

    JsonParser parser;
    if (!parser.parseMapData(doc.object())) {
        emit mapLoadError("Failed to parse map data");
        return false;
    }

    m_buildings = parser.buildings();
    m_roads = parser.roads();
    m_nodes = parser.nodes();
    m_environments = parser.environments();
    buildGraphFromData();

    emit mapLoaded();
    return true;
}

bool MapController::loadFromDatabase(DatabaseManager *dbManager)
{
    if (!dbManager || !dbManager->isOpen()) {
        emit mapLoadError("Database not open");
        return false;
    }

    QSqlQuery q(dbManager->database());

    // Load buildings
    m_buildings.clear();
    if (q.exec("SELECT id, name, type, center_x, center_y, polygon, description, aliases FROM buildings")) {
        while (q.next()) {
            Building b;
            b.id = q.value(0).toInt();
            b.name = q.value(1).toString();
            b.type = Building::typeFromString(q.value(2).toString());
            b.center = QPointF(q.value(3).toDouble(), q.value(4).toDouble());
            b.description = q.value(6).toString();

            // Parse polygon JSON
            QJsonDocument polyDoc = QJsonDocument::fromJson(q.value(5).toString().toUtf8());
            QJsonArray polyArr = polyDoc.array();
            for (const QJsonValue &pv : polyArr) {
                b.polygon.append(QPointF(pv["x"].toDouble(), pv["y"].toDouble()));
            }

            b.aliases = q.value(7).toString().split(",", Qt::SkipEmptyParts);
            m_buildings.append(b);
        }
    }

    // Load nodes
    m_nodes.clear();
    if (q.exec("SELECT id, x, y, building_id, is_entrance FROM nodes")) {
        while (q.next()) {
            Node n;
            n.id = q.value(0).toInt();
            n.coord = QPointF(q.value(1).toDouble(), q.value(2).toDouble());
            n.buildingId = q.value(3).toInt();
            n.isEntrance = q.value(4).toInt() != 0;
            m_nodes.append(n);
        }
    }

    // Load roads & edges
    m_roads.clear();
    if (q.exec("SELECT id, name, type, node_ids, points, weight_factor FROM roads")) {
        while (q.next()) {
            Road r;
            r.id = q.value(0).toInt();
            r.name = q.value(1).toString();
            r.type = Road::typeFromString(q.value(2).toString());
            bool ok = false;
            double wf = q.value(5).toDouble(&ok);
            r.weightFactor = ok ? wf : 1.0;

            QJsonDocument nodesDoc = QJsonDocument::fromJson(q.value(3).toString().toUtf8());
            for (const QJsonValue &nv : nodesDoc.array()) {
                r.nodeIds.append(nv.toInt());
            }

            QJsonDocument ptsDoc = QJsonDocument::fromJson(q.value(4).toString().toUtf8());
            for (const QJsonValue &pv : ptsDoc.array()) {
                r.points.append(QPointF(pv["x"].toDouble(), pv["y"].toDouble()));
            }
            m_roads.append(r);
        }
    }

    buildGraphFromData();
    emit mapLoaded();
    return true;
}

bool MapController::saveToDatabase(DatabaseManager *dbManager)
{
    if (!dbManager || !dbManager->isOpen()) return false;
    QSqlQuery q(dbManager->database());

    // Clear existing data
    q.exec("DELETE FROM buildings");
    q.exec("DELETE FROM nodes");
    q.exec("DELETE FROM roads");
    q.exec("DELETE FROM edges");

    // Save buildings
    for (const Building &b : m_buildings) {
        QJsonArray polyArr;
        for (const QPointF &pt : b.polygon) {
            QJsonObject ptObj;
            ptObj["x"] = pt.x();
            ptObj["y"] = pt.y();
            polyArr.append(ptObj);
        }
        q.prepare("INSERT INTO buildings (id, name, type, center_x, center_y, polygon, description, aliases) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
        q.addBindValue(b.id);
        q.addBindValue(b.name);
        q.addBindValue(b.typeString());
        q.addBindValue(b.center.x());
        q.addBindValue(b.center.y());
        q.addBindValue(QJsonDocument(polyArr).toJson(QJsonDocument::Compact));
        q.addBindValue(b.description);
        q.addBindValue(b.aliases.join(","));
        q.exec();
    }

    // Save nodes
    for (const Node &n : m_nodes) {
        q.prepare("INSERT INTO nodes (id, x, y, building_id, is_entrance) VALUES (?, ?, ?, ?, ?)");
        q.addBindValue(n.id);
        q.addBindValue(n.coord.x());
        q.addBindValue(n.coord.y());
        q.addBindValue(n.buildingId);
        q.addBindValue(n.isEntrance ? 1 : 0);
        q.exec();
    }

    // Save roads & edges
    for (const Road &r : m_roads) {
        QJsonArray nodeArr;
        for (int nid : r.nodeIds) nodeArr.append(nid);
        QJsonArray ptsArr;
        for (const QPointF &pt : r.points) {
            QJsonObject ptObj;
            ptObj["x"] = pt.x();
            ptObj["y"] = pt.y();
            ptsArr.append(ptObj);
        }
        q.prepare("INSERT INTO roads (id, name, type, node_ids, points, weight_factor) "
                  "VALUES (?, ?, ?, ?, ?, ?)");
        q.addBindValue(r.id);
        q.addBindValue(r.name);
        q.addBindValue(r.typeString());
        q.addBindValue(QJsonDocument(nodeArr).toJson(QJsonDocument::Compact));
        q.addBindValue(QJsonDocument(ptsArr).toJson(QJsonDocument::Compact));
        q.addBindValue(r.weightFactor);
        q.exec();

        // Generate edges from consecutive nodes
        for (int i = 0; i < r.nodeIds.size() - 1; ++i) {
            int from = r.nodeIds[i], to = r.nodeIds[i + 1];
            QPointF p1 = nodeCoord(from), p2 = nodeCoord(to);
            double dist = QLineF(p1, p2).length();
            q.prepare("INSERT INTO edges (from_node, to_node, road_id, distance) VALUES (?, ?, ?, ?)");
            q.addBindValue(from);
            q.addBindValue(to);
            q.addBindValue(r.id);
            q.addBindValue(dist);
            q.exec();
        }
    }

    return true;
}

void MapController::buildScene(QGraphicsScene *scene)
{
    scene->clear();

    drawBackground(scene);

    // Roads
    for (const Road &r : m_roads) {
        drawRoad(scene, r);
    }

    // Buildings — polygon-based with drop shadow
    for (const Building &b : m_buildings) {
        drawBuildingItem(scene, b);
    }

    // Labels on top
    for (const Building &b : m_buildings) {
        drawBuildingLabel(scene, b);
    }

    // Nodes (interaction dots)
    drawNodes(scene);
}

// ========== private helpers ==========

void MapController::drawBackground(QGraphicsScene *scene)
{
    if (!m_bgImagePath.isEmpty()) {
        QImage img(m_bgImagePath);
        if (!img.isNull()) {
            QPixmap pixmap = QPixmap::fromImage(img);
            QGraphicsPixmapItem *bg = scene->addPixmap(pixmap);
            bg->setZValue(static_cast<int>(MapLayer::Background));
            bg->setTransformationMode(Qt::SmoothTransformation);

            // Scene rect = image dimensions exactly
            scene->setSceneRect(0, 0, pixmap.width(), pixmap.height());
            return;
        }
    }

    // Fallback: solid background computed from all data
    QRectF bounds;
    auto extend = [&](const QPointF &pt) {
        if (bounds.isNull()) bounds = QRectF(pt, QSizeF(0, 0));
        else {
            if (pt.x() < bounds.left()) bounds.setLeft(pt.x());
            if (pt.x() > bounds.right()) bounds.setRight(pt.x());
            if (pt.y() < bounds.top()) bounds.setTop(pt.y());
            if (pt.y() > bounds.bottom()) bounds.setBottom(pt.y());
        }
    };
    for (const Building &b : m_buildings) {
        extend(b.center);
        for (const QPointF &pt : b.polygon) extend(pt);
    }
    for (const Node &n : m_nodes) extend(n.coord);
    for (const Road &r : m_roads) {
        for (const QPointF &pt : r.points) extend(pt);
    }
    bounds.adjust(-40, -40, 40, 40);
    scene->addRect(bounds, Qt::NoPen, QColor(248, 248, 245))
         ->setZValue(static_cast<int>(MapLayer::Background));
    scene->setSceneRect(bounds);
}

void MapController::drawEnvironment(QGraphicsScene *scene)
{
    for (const EnvironmentArea &env : m_environments) {
        if (env.polygon.size() < 3) continue;
        if (env.type == EnvironmentType::Grass) continue; // skip grass

        QColor fill, border;
        if (env.type == EnvironmentType::Water) {
            fill = QColor(179, 217, 255);    // light blue
            border = QColor(153, 194, 230);
        }

        QGraphicsPolygonItem *item = new QGraphicsPolygonItem(env.polygon);
        item->setPen(QPen(border, 1.0));
        item->setBrush(fill);
        item->setZValue(static_cast<int>(MapLayer::Environment));

        scene->addItem(item);
    }
}

void MapController::drawRoad(QGraphicsScene *scene, const Road &road)
{
    if (road.points.size() < 2) return;

    QPainterPath path;
    path.moveTo(road.points.first());
    for (int i = 1; i < road.points.size(); ++i)
        path.lineTo(road.points[i]);

    double baseW, topW;
    switch (road.type) {
    case RoadType::VehicleRoad: baseW = 6.0; topW = 4.0; break;
    case RoadType::Cyclepath:   baseW = 4.5; topW = 2.8; break;
    case RoadType::Footpath:    baseW = 3.5; topW = 2.0; break;
    case RoadType::Indoor:      baseW = 2.5; topW = 1.2; break;
    default:                    baseW = 3.5; topW = 2.0; break;
    }

    // Bottom layer — darker purple border
    QPen basePen(QColor(80, 40, 120), baseW,
                 Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QGraphicsPathItem *baseItem = scene->addPath(path, basePen);
    baseItem->setZValue(static_cast<int>(MapLayer::RoadBase));

    // Top layer — lighter purple road surface
    QPen topPen(QColor(140, 100, 190), topW,
                Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QGraphicsPathItem *topItem = scene->addPath(path, topPen);
    topItem->setZValue(static_cast<int>(MapLayer::RoadTop));
}

void MapController::drawBuildingItem(QGraphicsScene *scene, const Building &b)
{
    // Color by building type
    QColor fill, border;
    switch (b.type) {
    case BuildingType::Teaching:
    case BuildingType::Library:
    case BuildingType::Lab:
        fill = QColor(100, 149, 237, 180);    // cornflower blue — academic
        border = QColor(70, 110, 190);
        break;
    case BuildingType::Dormitory:
        fill = QColor(144, 208, 144, 180);    // light green — dorms
        border = QColor(100, 170, 100);
        break;
    case BuildingType::Dining:
        fill = QColor(255, 182, 108, 180);    // orange — dining
        border = QColor(210, 140, 70);
        break;
    case BuildingType::Sports:
        fill = QColor(135, 206, 235, 180);    // sky blue — sports
        border = QColor(90, 160, 200);
        break;
    case BuildingType::Admin:
        fill = QColor(200, 180, 140, 180);    // tan — admin
        border = QColor(160, 140, 100);
        break;
    default:
        fill = QColor(210, 200, 215, 180);    // light purple — other
        border = QColor(160, 140, 180);
        break;
    }

    if (b.polygon.size() < 3) {
        QRectF fallback(b.center.x() - 20, b.center.y() - 15, 40, 30);
        QGraphicsRectItem *rect = scene->addRect(fallback, QPen(border, 1.5), fill);
        rect->setZValue(static_cast<int>(MapLayer::Building));
        rect->setData(0, b.id);
        return;
    }

    QGraphicsPolygonItem *item = new QGraphicsPolygonItem(b.polygon);
    item->setBrush(fill);
    item->setPen(QPen(border, 1.8));

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect;
    shadow->setBlurRadius(6);
    shadow->setOffset(1, 2);
    shadow->setColor(QColor(0, 0, 0, 50));
    item->setGraphicsEffect(shadow);

    item->setZValue(static_cast<int>(MapLayer::Building));
    item->setData(0, b.id);

    scene->addItem(item);
}

void MapController::drawBuildingLabel(QGraphicsScene *scene, const Building &b)
{
    QGraphicsTextItem *label = scene->addText(b.name);
    label->setDefaultTextColor(QColor(80, 80, 80));

    QFont font = label->font();
    font.setPointSize(8);
    label->setFont(font);

    // Center label on building center
    label->setPos(b.center.x() - label->boundingRect().width() / 2,
                  b.center.y() - label->boundingRect().height() / 2);
    label->setZValue(static_cast<int>(MapLayer::LabelIcon));
}

void MapController::drawNodes(QGraphicsScene *scene)
{
    for (const Node &n : m_nodes) {
        // Purple dots — slightly larger and brighter for entrance nodes
        QColor color = n.isEntrance ? QColor(180, 120, 230) : QColor(130, 70, 190);
        double r = n.isEntrance ? 5.5 : 4.0;
        QGraphicsEllipseItem *item = scene->addEllipse(
            n.coord.x() - r, n.coord.y() - r, r * 2, r * 2,
            QPen(QColor(100, 50, 150), 1.5), QBrush(color));
        item->setZValue(static_cast<int>(MapLayer::LabelIcon));
        item->setData(0, n.id);
    }
}

Building* MapController::findBuilding(int id)
{
    for (int i = 0; i < m_buildings.size(); ++i) {
        if (m_buildings[i].id == id) return &m_buildings[i];
    }
    return nullptr;
}

Building* MapController::findBuildingByName(const QString &name)
{
    for (int i = 0; i < m_buildings.size(); ++i) {
        if (m_buildings[i].name == name ||
            m_buildings[i].aliases.contains(name)) {
            return &m_buildings[i];
        }
    }
    return nullptr;
}

Node* MapController::findNode(int id)
{
    for (int i = 0; i < m_nodes.size(); ++i) {
        if (m_nodes[i].id == id) return &m_nodes[i];
    }
    return nullptr;
}

const QVector<Building> &MapController::buildings() const { return m_buildings; }
const QVector<Node> &MapController::nodes() const { return m_nodes; }
const QVector<EnvironmentArea> &MapController::environments() const { return m_environments; }
const Graph &MapController::graph() const { return m_graph; }

QPointF MapController::nodeCoord(int nodeId)
{
    for (const Node &n : m_nodes) {
        if (n.id == nodeId) return n.coord;
    }
    return QPointF();
}

void MapController::buildGraphFromData()
{
    m_graph.clear();
    for (const Node &n : m_nodes) {
        m_graph.addNode(n.id, n.coord, n.buildingId, n.isEntrance);
    }
    for (const Road &r : m_roads) {
        for (int i = 0; i < r.nodeIds.size() - 1; ++i) {
            int from = r.nodeIds[i], to = r.nodeIds[i + 1];
            double dist = QLineF(nodeCoord(from), nodeCoord(to)).length();
            double weight = dist * r.weightFactor;
            m_graph.addEdge(from, to, dist, weight, r.id);
        }
    }
}

// ---- calibration ----

static QPointF transformPoint(const QPointF &p,
                               double sx, double sy, double ox, double oy)
{
    return QPointF(p.x() * sx + ox, p.y() * sy + oy);
}

void MapController::applyCalibration(QPointF old1, QPointF new1,
                                      QPointF old2, QPointF new2)
{
    double dx = old2.x() - old1.x();
    double dy = old2.y() - old1.y();
    double oldDist = std::hypot(dx, dy);
    double newDist = std::hypot(new2.x() - new1.x(), new2.y() - new1.y());
    if (qFuzzyIsNull(oldDist)) {
        qWarning() << "Calibration: reference points too close together";
        return;
    }
    // Uniform scale — preserves aspect ratio, no distortion
    double s = newDist / oldDist;
    double ox = new1.x() - old1.x() * s;
    double oy = new1.y() - old1.y() * s;

    // Transform nodes only — buildings/roads/environments stay put
    for (Node &n : m_nodes) {
        n.coord = transformPoint(n.coord, s, s, ox, oy);
    }

    // Rebuild graph with new coordinates
    buildGraphFromData();

    qDebug().noquote()
        << QStringLiteral("[CALIB] scale=%1 offset=(%2,%3)")
               .arg(s, 0, 'f', 4).arg(ox, 0, 'f', 1).arg(oy, 0, 'f', 1);
}

QString MapController::exportCalibratedJson() const
{
    QJsonObject root;

    QJsonArray buildingsArr;
    for (const Building &b : m_buildings) {
        QJsonObject obj;
        obj["id"] = b.id;
        obj["name"] = b.name;
        // Type: map BuildingType enum back to string
        switch (b.type) {
        case BuildingType::Teaching:   obj["type"] = "academic"; break;
        case BuildingType::Dormitory:  obj["type"] = "dorm"; break;
        case BuildingType::Library:    obj["type"] = "academic"; break;
        case BuildingType::Dining:     obj["type"] = "dining"; break;
        case BuildingType::Admin:      obj["type"] = "academic"; break;
        case BuildingType::Sports:     obj["type"] = "sports"; break;
        case BuildingType::Lab:        obj["type"] = "academic"; break;
        default:                       obj["type"] = "other"; break;
        }
        obj["center_x"] = int(b.center.x());
        obj["center_y"] = int(b.center.y());
        obj["description"] = b.description;

        QJsonArray polyArr;
        for (const QPointF &pt : b.polygon) {
            QJsonObject ptObj;
            ptObj["x"] = int(pt.x());
            ptObj["y"] = int(pt.y());
            polyArr.append(ptObj);
        }
        obj["polygon"] = polyArr;

        QJsonArray aliasArr;
        for (const QString &a : b.aliases)
            aliasArr.append(a);
        obj["aliases"] = aliasArr;

        buildingsArr.append(obj);
    }
    root["buildings"] = buildingsArr;

    QJsonArray nodesArr;
    for (const Node &n : m_nodes) {
        QJsonObject obj;
        obj["id"] = n.id;
        obj["x"] = int(n.coord.x());
        obj["y"] = int(n.coord.y());
        obj["building_id"] = n.buildingId;
        obj["is_entrance"] = n.isEntrance ? 1 : 0;
        nodesArr.append(obj);
    }
    root["nodes"] = nodesArr;

    QJsonArray roadsArr;
    for (const Road &r : m_roads) {
        QJsonObject obj;
        obj["id"] = r.id;
        obj["name"] = r.name;
        obj["type"] = r.typeString().toLower();
        obj["weight_factor"] = r.weightFactor;

        QJsonArray nodeArr;
        for (int nid : r.nodeIds) nodeArr.append(nid);
        obj["node_ids"] = nodeArr;

        QJsonArray ptsArr;
        for (const QPointF &pt : r.points) {
            QJsonObject ptObj;
            ptObj["x"] = int(pt.x());
            ptObj["y"] = int(pt.y());
            ptsArr.append(ptObj);
        }
        obj["points"] = ptsArr;

        roadsArr.append(obj);
    }
    root["roads"] = roadsArr;

    QJsonDocument doc(root);
    return doc.toJson(QJsonDocument::Indented);
}