#pragma once
#include <QGraphicsScene>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsRectItem>
#include <QGraphicsPathItem>
#include <QGraphicsLineItem>
#include <QPainterPath>
#include <QFont>
#include <QFontMetrics>
#include <QMap>
#include <QVector>
#include <algorithm>

/// @brief 建筑标签碰撞检测与 LOD 管理器
///
/// 设计思路：
///   1. 所有建筑先绘制 3D 体，标签由 LabelManager 统一后处理
///   2. 按优先级（建筑高度 × 边权）排序，贪心布局
///   3. 碰撞检测：QRectF intersects，重叠时引线错开或隐藏
///   4. LOD：根据缩放级别动态调整可见标签数量
///
struct LabelInfo {
    int nodeId;
    QString name;
    QPointF screenPos;    // 标签锚点（屏幕坐标）
    qreal priority;       // 优先级（越高越优先显示）
    qreal buildingHeight; // 建筑高度（用于引线长度）
};

class LabelManager {
public:
    explicit LabelManager(QGraphicsScene* scene)
        : m_scene(scene) {}

    ~LabelManager() { clear(); }

    /// 清除所有标签
    void clear() {
        for (auto* item : m_labelItems) {
            m_scene->removeItem(item);
            delete item;
        }
        m_labelItems.clear();
        for (auto* item : m_leaderLines) {
            m_scene->removeItem(item);
            delete item;
        }
        m_leaderLines.clear();
        for (auto* item : m_bgRects) {
            m_scene->removeItem(item);
            delete item;
        }
        m_bgRects.clear();
        m_labels.clear();
        m_occupiedRects.clear();
    }

    /// 注册一个标签（在 drawBuildings 时调用）
    void registerLabel(int nodeId, const QString& name,
                       QPointF screenPos, qreal priority,
                       qreal buildingHeight) {
        m_labels.append({nodeId, name, screenPos, priority, buildingHeight});
    }

    /// 更新标签可见性（碰撞检测 + LOD）
    /// @param zoomLevel 当前缩放级别（transform().m11()）
    void updateLabels(qreal zoomLevel) {
        // 清除旧标签
        for (auto* item : m_labelItems) { m_scene->removeItem(item); delete item; }
        m_labelItems.clear();
        for (auto* item : m_leaderLines) { m_scene->removeItem(item); delete item; }
        m_leaderLines.clear();
        for (auto* item : m_bgRects) { m_scene->removeItem(item); delete item; }
        m_bgRects.clear();
        m_occupiedRects.clear();

        // LOD 阈值：缩放越小（看越远），显示的标签越少
        // zoomLevel < 0.3: 只显示 top 5
        // zoomLevel < 0.6: 显示 top 10
        // zoomLevel >= 0.6: 显示全部
        int maxLabels;
        if (zoomLevel < 0.3)      maxLabels = 5;
        else if (zoomLevel < 0.6) maxLabels = 10;
        else if (zoomLevel < 1.0) maxLabels = 15;
        else                      maxLabels = 999;

        // 字体大小随缩放调整（近看大字，远看小字）
        int fontSize = qBound(6, static_cast<int>(8 * zoomLevel + 4), 12);
        QFont font("Microsoft YaHei", fontSize, QFont::Bold);
        QFontMetrics fm(font);

        // 按优先级降序排序
        auto sorted = m_labels;
        std::sort(sorted.begin(), sorted.end(),
                  [](const LabelInfo& a, const LabelInfo& b) {
                      return a.priority > b.priority;
                  });

        int shown = 0;
        for (const auto& label : sorted) {
            if (shown >= maxLabels) break;

            // 计算标签矩形
            int tw = fm.horizontalAdvance(label.name) + 12;
            int th = fm.height() + 6;
            QPointF anchor = label.screenPos;
            QRectF desired(anchor.x() - tw / 2.0,
                           anchor.y() - th / 2.0,
                           tw, th);

            // 碰撞检测：尝试原始位置
            QRectF finalRect = desired;
            bool collided = checkCollision(finalRect);

            if (collided) {
                // 尝试向上偏移
                QRectF shifted = desired.translated(0, -th - 4);
                if (!checkCollision(shifted)) {
                    finalRect = shifted;
                    // 画引线
                    auto* leader = m_scene->addLine(
                        anchor.x(), anchor.y(),
                        finalRect.center().x(), finalRect.bottom(),
                        QPen(QColor(150, 160, 180, 120), 0.8, Qt::DashLine));
                    leader->setZValue(50);
                    m_leaderLines.append(leader);
                } else {
                    // 再试向下偏移
                    shifted = desired.translated(0, th + 4);
                    if (!checkCollision(shifted)) {
                        finalRect = shifted;
                        auto* leader = m_scene->addLine(
                            anchor.x(), anchor.y(),
                            finalRect.center().x(), finalRect.top(),
                            QPen(QColor(150, 160, 180, 120), 0.8, Qt::DashLine));
                        leader->setZValue(50);
                        m_leaderLines.append(leader);
                    } else {
                        // 仍然碰撞 → 隐藏此标签
                        continue;
                    }
                }
            }

            // 注册碰撞矩形
            m_occupiedRects.append(finalRect.adjusted(-2, -2, 2, 2));

            // 绘制标签背景卡片
            QPainterPath bgPath;
            bgPath.addRoundedRect(finalRect, 3, 3);
            auto* bg = m_scene->addPath(bgPath);
            QLinearGradient grad(finalRect.topLeft(), finalRect.bottomLeft());
            grad.setColorAt(0.0, QColor(255, 255, 255, 220));
            grad.setColorAt(1.0, QColor(240, 242, 248, 220));
            bg->setBrush(QBrush(grad));
            bg->setPen(QPen(QColor(180, 190, 210, 130), 0.6));
            bg->setZValue(55);
            m_bgRects.append(bg);

            // 绘制标签文字
            auto* text = m_scene->addSimpleText(label.name, font);
            text->setBrush(QColor(40, 45, 60));
            text->setPos(finalRect.x() + 6,
                         finalRect.y() + (th - fm.height()) / 2.0);
            text->setZValue(56);
            m_labelItems.append(text);

            shown++;
        }
    }

private:
    /// 检查矩形是否与已占用矩形碰撞
    bool checkCollision(const QRectF& rect) const {
        for (const auto& r : m_occupiedRects) {
            if (r.intersects(rect))
                return true;
        }
        return false;
    }

    QGraphicsScene* m_scene;
    QVector<LabelInfo> m_labels;
    QVector<QRectF> m_occupiedRects;
    QVector<QGraphicsSimpleTextItem*> m_labelItems;
    QVector<QGraphicsLineItem*> m_leaderLines;
    QVector<QGraphicsPathItem*> m_bgRects;
};
