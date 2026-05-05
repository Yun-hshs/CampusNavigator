#include "RoadRenderer.h"
#include "graph/Graph.h"
#include <QGraphicsScene>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <cmath>

static QPointF bezierControlPoint(const Node& a, const Node& b,
                                  double weight, double maxWeight) {
    double mx = (a.x + b.x) / 2.0;
    double my = (a.y + b.y) / 2.0;
    double dx = b.x - a.x;
    double dy = b.y - a.y;
    double len = std::sqrt(dx * dx + dy * dy);
    if (len < 1e-6) return QPointF(mx, my);

    double nx = -dy / len;
    double ny =  dx / len;
    double curvature = 12.0 * (1.0 - 0.7 * weight / maxWeight);
    double sign = ((a.id + b.id) % 2 == 0) ? 1.0 : -1.0;

    return QPointF(mx + nx * curvature * sign,
                   my + ny * curvature * sign);
}

void RoadRenderer::draw(const RenderContext& ctx) {
    const auto nodes = ctx.graph->allNodes();

    double maxW = 0;
    for (const auto& n : nodes)
        for (const auto& e : ctx.graph->getEdges(n.id))
            maxW = qMax(maxW, e.weight);
    if (maxW < 1) maxW = 1;

    for (const auto& n : nodes) {
        for (const auto& e : ctx.graph->getEdges(n.id)) {
            if (e.from >= e.to) continue;
            const Node& a = ctx.graph->node(e.from);
            const Node& b = ctx.graph->node(e.to);

            QPointF pa = ctx.iso(a.x, a.y);
            QPointF pb = ctx.iso(b.x, b.y);

            QPointF cpLogic = bezierControlPoint(a, b, e.weight, maxW);
            QPointF cp = ctx.iso(cpLogic.x(), cpLogic.y());

            QPainterPath path(pa);
            path.quadTo(cp, pb);

            bool isFootpath = (e.type == "footpath");
            bool isMain = !isFootpath && (e.weight >= 150);

            qreal wShadow, wBase, wSurf, wCenter;
            QColor baseColor, surfColor, centerClr;
            int baseZ;

            if (isFootpath) {
                wShadow = 6;  wBase = 4;  wSurf = 2.5;  wCenter = 1;
                baseColor = QColor(185, 175, 155);
                surfColor = QColor(210, 200, 180);
                centerClr = QColor(225, 218, 200);
                baseZ = -53;
            } else if (isMain) {
                wShadow = 16; wBase = 12; wSurf = 8;    wCenter = 3;
                baseColor = QColor(155, 150, 140);
                surfColor = QColor(210, 205, 195);
                centerClr = QColor(228, 224, 216);
                baseZ = -55;
            } else {
                wShadow = 10; wBase = 7;  wSurf = 4;    wCenter = 1.5;
                baseColor = QColor(175, 170, 162);
                surfColor = QColor(220, 218, 210);
                centerClr = QColor(232, 230, 225);
                baseZ = -55;
            }

            QPen shadowPen(QColor(0, 0, 0, 28), wShadow);
            shadowPen.setCapStyle(Qt::RoundCap);
            auto* s1 = ctx.scene->addPath(path, shadowPen);
            s1->setZValue(baseZ);

            QPen basePen(baseColor, wBase);
            basePen.setCapStyle(Qt::RoundCap);
            auto* s2 = ctx.scene->addPath(path, basePen);
            s2->setZValue(baseZ + 5);

            QPen surfPen(surfColor, wSurf);
            surfPen.setCapStyle(Qt::RoundCap);
            auto* s3 = ctx.scene->addPath(path, surfPen);
            s3->setZValue(baseZ + 6);

            QPen centerPen(centerClr, wCenter);
            centerPen.setCapStyle(Qt::RoundCap);
            auto* s4 = ctx.scene->addPath(path, centerPen);
            s4->setZValue(baseZ + 7);

            QPen dashPen(QColor(255, 255, 230, 100), 1,
                         isFootpath ? Qt::DashLine : Qt::SolidLine);
            dashPen.setDashPattern(isFootpath ? QVector<qreal>{3, 6}
                                              : QVector<qreal>{5, 8});
            dashPen.setCapStyle(Qt::RoundCap);
            auto* s5 = ctx.scene->addPath(path, dashPen);
            s5->setZValue(baseZ + 8);
        }
    }
}
