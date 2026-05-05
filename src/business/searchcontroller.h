#ifndef SEARCHCONTROLLER_H
#define SEARCHCONTROLLER_H

#include <QObject>
#include <QString>
#include <QVector>
#include "building.h"
#include "spatialindex.h"

class DatabaseManager;

struct SearchResult {
    int id;            // buildingId 或 nodeId
    QString name;
    QString type;
    double distance;   // 距搜索中心距离 (-1 = 不适用)
};

class SearchController : public QObject {
    Q_OBJECT
public:
    explicit SearchController(QObject *parent = nullptr);

    void setBuildings(const QVector<Building> &buildings);
    void setSpatialIndex(SpatialIndex *index);

    QVector<SearchResult> searchByName(const QString &keyword);
    QVector<SearchResult> searchNearby(const QPointF &center, double radius);

signals:
    void searchResultsReady(const QVector<SearchResult> &results);

private:
    QVector<Building> m_buildings;
    SpatialIndex *m_spatialIndex = nullptr;
};

#endif // SEARCHCONTROLLER_H