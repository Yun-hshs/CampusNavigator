#include "searchcontroller.h"
#include <QLineF>
#include <QDebug>

SearchController::SearchController(QObject *parent)
    : QObject(parent)
{
}

void SearchController::setBuildings(const QVector<Building> &buildings)
{
    m_buildings = buildings;
}

void SearchController::setSpatialIndex(SpatialIndex *index)
{
    m_spatialIndex = index;
}

QVector<SearchResult> SearchController::searchByName(const QString &keyword)
{
    QVector<SearchResult> results;
    QString lower = keyword.toLower();

    for (const Building &b : m_buildings) {
        bool match = b.name.toLower().contains(lower);
        if (!match) {
            for (const QString &alias : b.aliases) {
                if (alias.toLower().contains(lower)) {
                    match = true;
                    break;
                }
            }
        }
        if (match) {
            results.append({b.id, b.name, b.typeString(), -1.0});
        }
    }

    return results;
}

QVector<SearchResult> SearchController::searchNearby(const QPointF &center, double radius)
{
    QVector<SearchResult> results;
    if (!m_spatialIndex) return results;

    QVector<int> nearbyIds = m_spatialIndex->searchNearby(center, radius);
    for (int id : nearbyIds) {
        for (const Building &b : m_buildings) {
            if (b.id == id) {
                double dist = QLineF(center, b.center).length();
                results.append({b.id, b.name, b.typeString(), dist});
                break;
            }
        }
    }

    // Sort by distance
    std::sort(results.begin(), results.end(),
              [](const SearchResult &a, const SearchResult &b) {
                  return a.distance < b.distance;
              });

    return results;
}