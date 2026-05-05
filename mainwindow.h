#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QAction>
#include <QLabel>
#include <QDockWidget>
#include <QPushButton>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MapWidget;
class SearchBar;
class RoutePanel;
class CalibrationPanel;
class LoginDialog;
class RouteDialog;
class RouteHistoryDialog;
class MapController;
class RouteController;
class SearchController;
class UserController;
class UpdateController;
class DatabaseManager;
class UserDAO;
class NetworkManager;
class SpatialIndex;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onMapLoaded();
    void onMapLoadError(const QString &msg);

    void onBuildingClicked(int buildingId);
    void onNodeClicked(int nodeId);
    void onMapPointClicked(const QPointF &scenePos);

    void onSearchRequested(const QString &keyword);
    void onSearchResultSelected(int buildingId);

    void onPlanRouteRequested(int startNode, int endNode, int strategy);
    void onRoutePlanned(const class Route &route);
    void onRoutePlanFailed(const QString &msg);

    void onLoginRequested(const QString &username, const QString &password);
    void onRegisterRequested(const QString &username, const QString &password);
    void onLoginSuccess(const QString &username);
    void onLoginFailed(const QString &msg);
    void onRegisterSuccess(const QString &username);
    void onRegisterFailed(const QString &msg);

    void onZoomIn();
    void onZoomOut();
    void onFitToWindow();

    void onActionOpenMap();
    void onActionPlanRoute();
    void onActionLogin();
    void onActionLogout();
    void onActionFavorites();
    void onActionRouteHistory();

    void onUpdateAvailable(const QString &version, const QString &url);
    void onUpdateFailed(const QString &msg);

private:
    void setupUI();
    void setupControllers();
    void setupConnections();
    void setupMenus();
    void setupToolbar();
    void updateStatusBar(const QString &msg);
    void updateUserActions();
    void loadDefaultMap();
    void setupFavoritesPanel();
    void updateFavoritesPanel(int buildingId);

    Ui::MainWindow *ui;

    // UI components
    MapWidget *m_mapWidget = nullptr;
    SearchBar *m_searchBar = nullptr;
    QWidget *m_searchOverlay = nullptr;
    RoutePanel *m_routePanel = nullptr;
    QDockWidget *m_routeDock = nullptr;
    CalibrationPanel *m_calibPanel = nullptr;
    QDockWidget *m_calibDock = nullptr;
    int m_calibPairIndex = 0;     // building-click: which pair (0=A, 1=B)
    bool m_calibFillOld = true;  // building-click: true=fill old, false=fill new
    int m_calibPickIndex = -1;   // manual pick-button mode: which pair
    QLabel *m_coordHud = nullptr;
    LoginDialog *m_loginDialog = nullptr;
    RouteDialog *m_routeDialog = nullptr;
    RouteHistoryDialog *m_routeHistoryDialog = nullptr;
    QGraphicsScene *m_scene = nullptr;

    // Status bar labels
    QLabel *m_statusCoords = nullptr;
    QLabel *m_statusBuilding = nullptr;
    QLabel *m_statusUser = nullptr;

    // Favorites side panel
    QDockWidget *m_favDock = nullptr;
    QLabel *m_favNameLabel = nullptr;
    QLabel *m_favTypeLabel = nullptr;
    QLabel *m_favDescLabel = nullptr;
    QPushButton *m_favToggleBtn = nullptr;

    // Actions
    QAction *m_actionLogin = nullptr;
    QAction *m_actionLogout = nullptr;
    QAction *m_actionFavorites = nullptr;

    // Controllers
    MapController *m_mapController = nullptr;
    RouteController *m_routeController = nullptr;
    SearchController *m_searchController = nullptr;
    UserController *m_userController = nullptr;
    UpdateController *m_updateController = nullptr;

    // Data layer
    DatabaseManager *m_dbManager = nullptr;
    UserDAO *m_userDao = nullptr;
    NetworkManager *m_networkManager = nullptr;
    SpatialIndex *m_spatialIndex = nullptr;

    int m_selectedBuildingId = -1;
    int m_routeStartNode = -1;
    QString m_pendingUpdateUrl;
};
#endif // MAINWINDOW_H
