#pragma once
#include <QPointF>
#include <functional>

class QGraphicsScene;
class Graph;

enum class RenderMode { Isometric, Flat2D };

struct RenderContext {
    QGraphicsScene* scene;
    const Graph* graph;
    std::function<QPointF(qreal, qreal)> iso;
    RenderMode mode = RenderMode::Isometric;
};
