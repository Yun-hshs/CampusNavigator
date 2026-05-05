#ifndef BUILDING_H
#define BUILDING_H

#include <QString>
#include <QStringList>
#include <QPointF>
#include <QPolygonF>

enum class BuildingType {
    Teaching,      // 教学楼
    Dormitory,     // 宿舍
    Library,       // 图书馆
    Dining,        // 餐厅
    Admin,         // 行政楼
    Sports,        // 体育设施
    Lab,           // 实验楼
    Other
};

class Building {
public:
    int id = 0;
    QString name;
    BuildingType type = BuildingType::Other;
    QPointF center;              // 中心坐标 (x, y) in scene coords
    QPolygonF polygon;           // 建筑轮廓多边形
    QString description;
    QStringList aliases;         // 搜索别名

    QString typeString() const;
    static BuildingType typeFromString(const QString &s);
};

#endif // BUILDING_H