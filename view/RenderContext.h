#pragma once
#include <QPointF>
#include <functional>

class QGraphicsScene;
class Graph;

struct RenderContext {
    QGraphicsScene* scene;
    const Graph* graph;
    std::function<QPointF(qreal, qreal)> iso;
};
