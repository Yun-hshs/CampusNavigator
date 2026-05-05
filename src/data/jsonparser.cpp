#include "jsonparser.h"
#include <QJsonDocument>
#include <QPointF>
#include <QDebug>

JsonParser::JsonParser(QObject *parent)
    : QObject(parent)
{
}

bool JsonParser::parseMapData(const QJsonObject &root)
{
    m_buildings.clear();
    m_roads.clear();
    m_nodes.clear();
    m_environments.clear();

    const QJsonArray buildingsArr = root["buildings"].toArray();
    for (const QJsonValue &v : buildingsArr) {
        m_buildings.append(parseBuilding(v.toObject()));
    }

    const QJsonArray roadsArr = root["roads"].toArray();
    for (const QJsonValue &v : roadsArr) {
        m_roads.append(parseRoad(v.toObject()));
    }

    const QJsonArray nodesArr = root["nodes"].toArray();
    for (const QJsonValue &v : nodesArr) {
        m_nodes.append(parseNode(v.toObject()));
    }

    const QJsonArray envArr = root["environment"].toArray();
    for (const QJsonValue &v : envArr) {
        m_environments.append(parseEnvironment(v.toObject()));
    }

    if (m_nodes.isEmpty()) {
        emit parseError("No nodes found in map data");
        return false;
    }

    emit parseCompleted();
    return true;
}

QVector<Building> JsonParser::buildings() const { return m_buildings; }
QVector<Road> JsonParser::roads() const { return m_roads; }
QVector<Node> JsonParser::nodes() const { return m_nodes; }
QVector<EnvironmentArea> JsonParser::environments() const { return m_environments; }

Building JsonParser::parseBuilding(const QJsonObject &obj)
{
    Building b;
    b.id = obj["id"].toInt();
    b.name = obj["name"].toString();
    b.type = Building::typeFromString(obj["type"].toString());
    b.description = obj["description"].toString();

    b.center = QPointF(obj["center_x"].toDouble(), obj["center_y"].toDouble());

    const QJsonArray polyArr = obj["polygon"].toArray();
    for (const QJsonValue &pv : polyArr) {
        const QJsonObject pt = pv.toObject();
        b.polygon.append(QPointF(pt["x"].toDouble(), pt["y"].toDouble()));
    }

    const QJsonArray aliasArr = obj["aliases"].toArray();
    for (const QJsonValue &av : aliasArr) {
        b.aliases.append(av.toString());
    }

    return b;
}

Road JsonParser::parseRoad(const QJsonObject &obj)
{
    Road r;
    r.id = obj["id"].toInt();
    r.name = obj["name"].toString();
    r.type = Road::typeFromString(obj["type"].toString());
    r.weightFactor = obj["weight_factor"].toDouble(1.0);

    const QJsonArray nodeArr = obj["node_ids"].toArray();
    for (const QJsonValue &nv : nodeArr) {
        r.nodeIds.append(nv.toInt());
    }

    const QJsonArray ptsArr = obj["points"].toArray();
    for (const QJsonValue &pv : ptsArr) {
        const QJsonObject pt = pv.toObject();
        r.points.append(QPointF(pt["x"].toDouble(), pt["y"].toDouble()));
    }

    return r;
}

Node JsonParser::parseNode(const QJsonObject &obj)
{
    Node n;
    n.id = obj["id"].toInt();
    n.coord = QPointF(obj["x"].toDouble(), obj["y"].toDouble());
    n.buildingId = obj["building_id"].toInt(-1);
    n.isEntrance = obj["is_entrance"].toInt(0) != 0;
    return n;
}

EnvironmentArea JsonParser::parseEnvironment(const QJsonObject &obj)
{
    EnvironmentArea env;
    env.id = obj["id"].toInt();
    env.name = obj["name"].toString();
    env.type = EnvironmentArea::typeFromString(obj["type"].toString());

    const QJsonArray polyArr = obj["polygon"].toArray();
    for (const QJsonValue &pv : polyArr) {
        const QJsonObject pt = pv.toObject();
        env.polygon.append(QPointF(pt["x"].toDouble(), pt["y"].toDouble()));
    }

    return env;
}