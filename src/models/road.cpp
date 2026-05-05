#include "road.h"

QString Road::typeString() const {
    switch (type) {
    case RoadType::Footpath:     return QStringLiteral("步行道");
    case RoadType::Cyclepath:    return QStringLiteral("自行车道");
    case RoadType::VehicleRoad:  return QStringLiteral("车行道");
    case RoadType::Indoor:       return QStringLiteral("室内通道");
    default:                     return QStringLiteral("其他");
    }
}

RoadType Road::typeFromString(const QString &s) {
    if (s == "footpath" || s == "步行道")    return RoadType::Footpath;
    if (s == "cyclepath" || s == "自行车道") return RoadType::Cyclepath;
    if (s == "vehicle" || s == "车行道")     return RoadType::VehicleRoad;
    if (s == "indoor" || s == "室内通道")    return RoadType::Indoor;
    return RoadType::Other;
}