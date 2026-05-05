#include "building.h"

QString Building::typeString() const {
    switch (type) {
    case BuildingType::Teaching:   return QStringLiteral("教学楼");
    case BuildingType::Dormitory:  return QStringLiteral("宿舍");
    case BuildingType::Library:    return QStringLiteral("图书馆");
    case BuildingType::Dining:     return QStringLiteral("餐厅");
    case BuildingType::Admin:      return QStringLiteral("行政楼");
    case BuildingType::Sports:     return QStringLiteral("体育设施");
    case BuildingType::Lab:        return QStringLiteral("实验楼");
    default:                       return QStringLiteral("其他");
    }
}

BuildingType Building::typeFromString(const QString &s) {
    if (s == "teaching" || s == "教学楼")   return BuildingType::Teaching;
    if (s == "dormitory" || s == "宿舍" || s == "dorm")  return BuildingType::Dormitory;
    if (s == "library" || s == "图书馆")    return BuildingType::Library;
    if (s == "dining" || s == "餐厅" || s == "canteen") return BuildingType::Dining;
    if (s == "admin" || s == "行政楼")      return BuildingType::Admin;
    if (s == "sports" || s == "体育设施")   return BuildingType::Sports;
    if (s == "lab" || s == "实验楼")        return BuildingType::Lab;
    return BuildingType::Other;
}