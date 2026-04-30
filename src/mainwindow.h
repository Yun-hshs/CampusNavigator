#pragma once
#include <QMainWindow>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QDockWidget>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QHBoxLayout>

class OsmMapView;
class MapScene;
class PathHighlighter;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onNavigate();
    void onClear();
    void onBuildingClicked(int id, const QString& name, const QString& desc);
    void onSearchReturn();

private:
    void setupUi();
    void setupToolBar();
    void setupInfoPanel();
    void loadMapData();

    OsmMapView*      m_osmView     = nullptr;
    MapScene*        m_mapScene    = nullptr;
    PathHighlighter* m_highlighter = nullptr;

    QLineEdit*   m_searchEdit  = nullptr;
    QComboBox*   m_startCombo  = nullptr;
    QComboBox*   m_endCombo    = nullptr;
    QPushButton* m_navigateBtn = nullptr;
    QPushButton* m_clearBtn    = nullptr;

    QLabel* m_infoTitle  = nullptr;
    QLabel* m_infoType   = nullptr;
    QLabel* m_infoDetail = nullptr;
    QLabel* m_pathInfo   = nullptr;
};
