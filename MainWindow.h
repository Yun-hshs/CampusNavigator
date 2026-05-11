#pragma once
#include <QMainWindow>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QDockWidget>
#include <QFrame>
#include <QCompleter>
#include "graph/Graph.h"

class MapView;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onNodeClicked(int id);
    void onNavigate();
    void onClear();
    void onSearchReturn();
    void onTravelModeChanged(int index);

private:
    void setupUi();
    void setupToolBar();
    void setupInfoPanel();
    void loadMapData();
    void navigate(int startId, int endId);
    void showPoiBubble(int nodeId, const QPointF& screenPos);
    void hidePoiBubble();

    Graph m_graph;
    MapView* m_mapView = nullptr;

    int m_startId = -1;
    int m_endId   = -1;
    int m_clickCount = 0;

    QLineEdit*   m_searchEdit  = nullptr;
    QCompleter*  m_completer   = nullptr;
    QComboBox*   m_startCombo  = nullptr;
    QComboBox*   m_endCombo    = nullptr;
    QComboBox*   m_modeCombo   = nullptr;
    QPushButton* m_navigateBtn = nullptr;
    QPushButton* m_clearBtn    = nullptr;
    QPushButton* m_viewModeBtn = nullptr;

    QLabel* m_infoTitle  = nullptr;
    QLabel* m_infoDetail = nullptr;
    QLabel* m_pathInfo   = nullptr;
    QLabel* m_pathLabel  = nullptr;
    QLabel* m_timeLabel  = nullptr;
    QLabel* m_statusLabel = nullptr;

    QFrame* m_poiBubble = nullptr;

    // Travel speeds (m/min)
    static constexpr double WALK_SPEED = 83.3;   // 5 km/h
    static constexpr double BIKE_SPEED = 250.0;   // 15 km/h
};
