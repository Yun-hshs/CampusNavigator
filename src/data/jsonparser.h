#ifndef JSONPARSER_H
#define JSONPARSER_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <QVector>

#include "building.h"
#include "road.h"
#include "node.h"
#include "environment.h"

class JsonParser : public QObject {
    Q_OBJECT
public:
    explicit JsonParser(QObject *parent = nullptr);

    bool parseMapData(const QJsonObject &root);

    QVector<Building> buildings() const;
    QVector<Road> roads() const;
    QVector<Node> nodes() const;
    QVector<EnvironmentArea> environments() const;

signals:
    void parseCompleted();
    void parseError(const QString &msg);

private:
    Building parseBuilding(const QJsonObject &obj);
    Road parseRoad(const QJsonObject &obj);
    Node parseNode(const QJsonObject &obj);
    EnvironmentArea parseEnvironment(const QJsonObject &obj);

    QVector<Building> m_buildings;
    QVector<Road> m_roads;
    QVector<Node> m_nodes;
    QVector<EnvironmentArea> m_environments;
};

#endif // JSONPARSER_H