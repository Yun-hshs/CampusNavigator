#pragma once
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QPen>
#include "graph.h"

class MapView : public QGraphicsView {
    Q_OBJECT
public:
    MapView(QWidget *parent = nullptr);
    void drawNodes(const Graph& graph);
    void drawEdges(const Graph& graph);
    void drawPath(const std::vector<int>& path, const Graph& graph);

private:
    QGraphicsScene* scene;
    void clearPath();
};