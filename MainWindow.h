#pragma once
#include <QMainWindow>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QDockWidget>
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

private:
    void setupUi();
    void setupToolBar();
    void setupInfoPanel();
    void loadMapData();
    void navigate(int startId, int endId);

    Graph m_graph;
    MapView* m_mapView = nullptr;

    int m_startId = -1;
    int m_endId   = -1;
    int m_clickCount = 0;

    QLineEdit*   m_searchEdit  = nullptr;
    QComboBox*   m_startCombo  = nullptr;
    QComboBox*   m_endCombo    = nullptr;
    QPushButton* m_navigateBtn = nullptr;
    QPushButton* m_clearBtn    = nullptr;

    QLabel* m_infoTitle  = nullptr;
    QLabel* m_infoDetail = nullptr;
    QLabel* m_pathInfo   = nullptr;
    QLabel* m_pathLabel  = nullptr;
    QLabel* m_statusLabel = nullptr;
};
