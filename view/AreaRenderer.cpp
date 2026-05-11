#include "AreaRenderer.h"
#include "graph/Graph.h"
#include <QGraphicsScene>
#include <QGraphicsPolygonItem>
#include <QGraphicsLineItem>
#include <QGraphicsEllipseItem>
#include <QLinearGradient>
#include <QRandomGenerator>

void AreaRenderer::draw(const RenderContext& ctx) {
    const auto areas = ctx.graph->allAreas();
    if (areas.isEmpty()) return;

    // ── Flat 2D mode: simple colored polygons ──
    if (ctx.mode == RenderMode::Flat2D) {
        for (const auto& area : areas) {
            QPolygonF screenPoly;
            for (const QPointF& pt : area.polygon)
                screenPoly << ctx.iso(pt.x(), pt.y());

            QColor fill;
            if (area.type == "plaza")             fill = QColor(220, 218, 210, 200);
            else if (area.type == "sports_field") fill = QColor(90, 160, 80, 200);
            else if (area.type == "garden")       fill = QColor(120, 180, 100, 180);
            else                                  fill = QColor(180, 200, 220, 180);

            auto* item = ctx.scene->addPolygon(screenPoly,
                QPen(fill.darker(120), 1), QBrush(fill));
            item->setZValue(area.zOrder);
        }
        return;
    }

    for (const auto& area : areas) {
        QPolygonF screenPoly;
        for (const QPointF& pt : area.polygon)
            screenPoly << ctx.iso(pt.x(), pt.y());

        QRectF bounds = screenPoly.boundingRect();

        if (area.type == "plaza") {
            QLinearGradient grad(bounds.topLeft(), bounds.bottomRight());
            grad.setColorAt(0.0, QColor(225, 222, 215, 200));
            grad.setColorAt(1.0, QColor(215, 212, 205, 200));
            auto* item = ctx.scene->addPolygon(screenPoly, Qt::NoPen, QBrush(grad));
            item->setZValue(area.zOrder);

            QPen gridPen(QColor(200, 195, 188, 60), 0.5);
            qreal step = 20;
            for (int i = -5; i <= 30; ++i) {
                qreal x1 = bounds.left() + i * step;
                auto* l1 = ctx.scene->addLine(x1, bounds.top(), x1, bounds.bottom(), gridPen);
                l1->setZValue(area.zOrder + 1);
                qreal y1 = bounds.top() + i * step;
                auto* l2 = ctx.scene->addLine(bounds.left(), y1, bounds.right(), y1, gridPen);
                l2->setZValue(area.zOrder + 1);
            }
            auto* border = ctx.scene->addPolygon(screenPoly,
                QPen(QColor(190, 185, 178, 120), 1.2), Qt::NoBrush);
            border->setZValue(area.zOrder + 2);

        } else if (area.type == "sports_field") {
            QLinearGradient grad(bounds.topLeft(), bounds.bottomRight());
            grad.setColorAt(0.0, QColor(80, 140, 70, 200));
            grad.setColorAt(0.5, QColor(70, 130, 60, 200));
            grad.setColorAt(1.0, QColor(90, 150, 80, 200));
            auto* item = ctx.scene->addPolygon(screenPoly, Qt::NoPen, QBrush(grad));
            item->setZValue(area.zOrder);

            QPen lanePen(QColor(255, 255, 255, 100), 0.8);
            int lanes = 6;
            for (int i = 0; i <= lanes; ++i) {
                qreal frac = static_cast<qreal>(i) / lanes;
                QPointF leftPt = screenPoly[0] + frac * (screenPoly[3] - screenPoly[0]);
                QPointF rightPt = screenPoly[1] + frac * (screenPoly[2] - screenPoly[1]);
                auto* lane = ctx.scene->addLine(leftPt.x(), leftPt.y(),
                    rightPt.x(), rightPt.y(), lanePen);
                lane->setZValue(area.zOrder + 1);
            }
            auto* border = ctx.scene->addPolygon(screenPoly,
                QPen(QColor(255, 255, 255, 80), 1.5), Qt::NoBrush);
            border->setZValue(area.zOrder + 2);

        } else if (area.type == "garden") {
            QLinearGradient grad(bounds.topLeft(), bounds.bottomRight());
            grad.setColorAt(0.0, QColor(120, 175, 90, 180));
            grad.setColorAt(0.5, QColor(110, 165, 80, 180));
            grad.setColorAt(1.0, QColor(130, 185, 100, 180));
            auto* item = ctx.scene->addPolygon(screenPoly, Qt::NoPen, QBrush(grad));
            item->setZValue(area.zOrder);

            auto rng = QRandomGenerator::global();
            for (int i = 0; i < 12; ++i) {
                qreal fx = bounds.left() + rng->bounded(static_cast<int>(bounds.width()));
                qreal fy = bounds.top() + rng->bounded(static_cast<int>(bounds.height()));
                QColor fc = QColor::fromHsv(rng->bounded(360), 160, 220, 160);
                auto* dot = ctx.scene->addEllipse(fx - 1.5, fy - 1.5, 3, 3,
                    Qt::NoPen, QBrush(fc));
                dot->setZValue(area.zOrder + 1);
            }
            auto* border = ctx.scene->addPolygon(screenPoly,
                QPen(QColor(100, 150, 75, 80), 1), Qt::NoBrush);
            border->setZValue(area.zOrder + 2);
        }
    }
}
