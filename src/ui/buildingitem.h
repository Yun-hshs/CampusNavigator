#pragma once
#include <QGraphicsRectItem>
#include <QColor>
#include <QString>
#include <QHash>

class QGraphicsSimpleTextItem;

class BuildingItem : public QGraphicsRectItem {
public:
    BuildingItem(int id, const QString& name, const QString& type,
                 const QString& desc, double x, double y, double w, double h,
                 QGraphicsItem* parent = nullptr);

    int buildingId() const { return m_id; }
    QString buildingName() const { return m_name; }
    QString buildingType() const { return m_type; }
    QString buildingDesc() const { return m_desc; }

    void setHighlighted(bool on);
    QColor typeColor() const { return m_fillColor; }

    static QColor colorForType(const QString& type);

    QGraphicsSimpleTextItem* label() const { return m_label; }

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    void updateLabelPos();

    int m_id;
    QString m_name;
    QString m_type;
    QString m_desc;
    QColor m_fillColor;
    QGraphicsSimpleTextItem* m_label = nullptr;
    bool m_highlighted = false;
};
