#pragma once
#include <QGraphicsView>

class MapView : public QGraphicsView {
    Q_OBJECT
public:
    explicit MapView(QWidget* parent = nullptr);
    void setMapScene(class MapScene* scene);
    void fitMap();

protected:
    void wheelEvent(QWheelEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    bool m_firstShow = true;
    static constexpr double ZOOM_FACTOR = 1.15;
    static constexpr double MIN_SCALE   = 0.05;
    static constexpr double MAX_SCALE   = 8.0;
};
