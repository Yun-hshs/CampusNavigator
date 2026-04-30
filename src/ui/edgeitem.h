// Road edge — black line default, red+bold when part of active path
#pragma once
#include <QGraphicsLineItem>
#include <QPen>

class EdgeItem : public QGraphicsLineItem {
public:
    EdgeItem(int fromId, int toId, double weight,
             const QLineF& line, QGraphicsItem* parent = nullptr);

    int fromId() const { return m_fromId; }
    int toId() const { return m_toId; }
    double weight() const { return m_weight; }

    void setPathHighlight(bool on);

private:
    int m_fromId;
    int m_toId;
    double m_weight;

    static constexpr QColor COLOR_NORMAL    = QColor(50, 50, 50);     // near-black
    static constexpr QColor COLOR_HIGHLIGHT = QColor(220, 30, 30);    // red
};
