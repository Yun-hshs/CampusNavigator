#ifndef NODE_H
#define NODE_H

#include <QPointF>

class Node {
public:
    int id = 0;
    QPointF coord;               // 场景坐标
    int buildingId = -1;         // 所属建筑ID (-1 = 独立路口)
    bool isEntrance = false;     // 是否建筑入口
};

#endif // NODE_H