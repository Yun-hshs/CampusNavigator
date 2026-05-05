#ifndef ROUTE_H
#define ROUTE_H

#include <QVector>
#include <QPointF>
#include <QString>

class Route {
public:
    QVector<int> nodeIds;        // 途经节点序列
    QVector<QPointF> points;     // 对应场景坐标序列
    double totalDistance = 0.0;  // 总距离 (米)
    double totalTime = 0.0;     // 预计时间 (秒)
    QString description;        // 路线描述
};

#endif // ROUTE_H