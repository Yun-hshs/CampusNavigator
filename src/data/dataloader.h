#pragma once
#include <QVector>
#include "src/models/building.h"
#include "src/models/road.h"

struct CampusData {
    QVector<Building> buildings;
    QVector<Road> roads;
};

class DataLoader {
public:
    static CampusData loadFromJson(const QString& filePath);
};
