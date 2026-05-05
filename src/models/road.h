#ifndef ROAD_H
#define ROAD_H

#include <QString>
#include <QVector>
#include <QPointF>

enum class RoadType {
    Footpath,     // 步行道
    Cyclepath,    // 自行车道
    VehicleRoad,  // 车行道
    Indoor,       // 室内通道
    Other
};

class Road {
public:
    int id = 0;
    QString name;
    RoadType type = RoadType::Footpath;
    QVector<int> nodeIds;        // 途经节点ID序列
    QVector<QPointF> points;     // 路径折线点 (scene coords)
    double weightFactor = 1.0;   // 通行权重系数 (1.0=步行, <1=更快, >1=更慢)

    QString typeString() const;
    static RoadType typeFromString(const QString &s);
};

#endif // ROAD_H