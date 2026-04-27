#include "mapview.h"
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QPen>
#include <QBrush>
#include <QTimer>

MapView::MapView(QWidget *parent) : QGraphicsView(parent) {
    scene = new QGraphicsScene(this);
    setScene(scene);
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setDragMode(QGraphicsView::ScrollHandDrag);  // 启用拖动
}

void MapView::drawNodes(const Graph& graph) {
    scene->clear();  // 清除所有节点和路径
    for (int i = 0; i < graph.size(); ++i) {
        const Node& node = graph.getNode(i);

        // 绘制节点
        QGraphicsEllipseItem* nodeItem = scene->addEllipse(node.x - 10, node.y - 10, 20, 20, QPen(Qt::black), QBrush(Qt::blue));
        
        // 显示节点名称
        QGraphicsTextItem* textItem = scene->addText(QString::fromStdString(node.name));
        textItem->setPos(node.x + 10, node.y - 10);
    }
}

void MapView::drawEdges(const Graph& graph) {
    for (int i = 0; i < graph.size(); ++i) {
        const Node& fromNode = graph.getNode(i);
        for (const Edge& edge : graph.getNeighbors(i)) {
            const Node& toNode = graph.getNode(edge.to);
            scene->addLine(fromNode.x, fromNode.y, toNode.x, toNode.y, QPen(Qt::gray));
        }
    }
}

void MapView::drawPath(const std::vector<int>& path, const Graph& graph) {
    clearPath();
    QPen pathPen(Qt::red);
    pathPen.setWidth(3);
    
    // 高亮路径上的节点和边
    for (size_t i = 1; i < path.size(); ++i) {
        const Node& fromNode = graph.getNode(path[i - 1]);
        const Node& toNode = graph.getNode(path[i]);

        // 绘制高亮路径
        scene->addLine(fromNode.x, fromNode.y, toNode.x, toNode.y, pathPen);
    }

    // 高亮路径上的节点
    for (int id : path) {
        const Node& node = graph.getNode(id);
        QGraphicsEllipseItem* nodeItem = scene->addEllipse(node.x - 10, node.y - 10, 20, 20, QPen(Qt::black), QBrush(Qt::yellow));
    }
}

void MapView::clearPath() {
    for (auto item : scene->items()) {
        if (item->pen().color() == Qt::red || item->brush().color() == Qt::yellow) {
            scene->removeItem(item);
            delete item;
        }
    }
}