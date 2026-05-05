#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "mapwidget.h"
#include "searchbar.h"
#include "routepanel.h"
#include "calibrationpanel.h"
#include "logindialog.h"
#include "routedialog.h"
#include "routehistorydialog.h"

#include "mapcontroller.h"
#include "routecontroller.h"
#include "searchcontroller.h"
#include "usercontroller.h"
#include "updatecontroller.h"

#include "databasemanager.h"
#include "userdao.h"
#include "networkmanager.h"
#include "spatialindex.h"

#include "building.h"
#include "node.h"
#include "route.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QToolBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGraphicsDropShadowEffect>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("校园导航"));
    resize(1200, 800);

    setupUI();
    setupControllers();
    setupConnections();
    setupMenus();
    setupToolbar();
    updateUserActions();

    loadDefaultMap();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUI()
{
    // ---- central widget: map fills everything ----
    QWidget *central = new QWidget(this);
    QGridLayout *grid = new QGridLayout(central);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setSpacing(0);

    // Map scene + view
    m_scene = new QGraphicsScene(this);
    m_mapWidget = new MapWidget(central);
    m_mapWidget->setScene(m_scene);
    grid->addWidget(m_mapWidget, 0, 0);

    // ---- floating search overlay (top-left) ----
    m_searchOverlay = new QWidget(central);
    m_searchOverlay->setObjectName(QStringLiteral("searchOverlay"));
    m_searchOverlay->setFixedWidth(340);

    QVBoxLayout *overlayInner = new QVBoxLayout(m_searchOverlay);
    overlayInner->setContentsMargins(0, 0, 0, 0);
    overlayInner->setSpacing(0);

    m_searchBar = new SearchBar(m_searchOverlay);
    m_searchBar->setObjectName(QStringLiteral("floatingSearchBar"));
    overlayInner->addWidget(m_searchBar);

    // Shadow effect on the overlay container
    QGraphicsDropShadowEffect *overlayShadow = new QGraphicsDropShadowEffect(m_searchOverlay);
    overlayShadow->setBlurRadius(16);
    overlayShadow->setOffset(0, 2);
    overlayShadow->setColor(QColor(0, 0, 0, 30));
    m_searchOverlay->setGraphicsEffect(overlayShadow);

    grid->addWidget(m_searchOverlay, 0, 0, Qt::AlignTop | Qt::AlignLeft);

    // ---- floating coordinate HUD (bottom-right) ----
    m_coordHud = new QLabel(QStringLiteral("0, 0"), central);
    m_coordHud->setObjectName(QStringLiteral("coordHud"));
    QFont hudFont("Consolas", 9);
    m_coordHud->setFont(hudFont);
    m_coordHud->setFixedWidth(130);
    m_coordHud->setAlignment(Qt::AlignCenter);
    m_coordHud->setAttribute(Qt::WA_TransparentForMouseEvents);
    grid->addWidget(m_coordHud, 0, 0, Qt::AlignBottom | Qt::AlignRight);

    setCentralWidget(central);

    // ---- apply modern QSS ----
    setStyleSheet(QStringLiteral(
        "#searchOverlay {"
        "  background: #FFFFFF;"
        "  border: none;"
        "  border-radius: 10px;"
        "  margin: 14px;"
        "}"
        "#floatingSearchBar {"
        "  background: transparent;"
        "  border: none;"
        "}"
        "#floatingSearchBar QLineEdit {"
        "  border: 1px solid #DDE0E2;"
        "  border-radius: 8px;"
        "  padding: 9px 14px;"
        "  font-size: 14px;"
        "  background: #FFFFFF;"
        "  color: #333333;"
        "  selection-background-color: #4A90D9;"
        "}"
        "#floatingSearchBar QLineEdit:focus {"
        "  border-color: #4A90D9;"
        "}"
        "#floatingSearchBar QLineEdit::placeholder {"
        "  color: #B0B5BA;"
        "}"
        "#floatingSearchBar QListWidget {"
        "  border: 1px solid #E0E3E6;"
        "  border-radius: 6px;"
        "  background: #FFFFFF;"
        "  padding: 4px 0;"
        "  outline: none;"
        "}"
        "#floatingSearchBar QListWidget::item {"
        "  padding: 6px 14px;"
        "  color: #333333;"
        "}"
        "#floatingSearchBar QListWidget::item:selected {"
        "  background: #EDF4FC;"
        "  color: #2A6BB3;"
        "}"
        "#floatingSearchBar QListWidget::item:hover {"
        "  background: #F5F7F9;"
        "}"
        "#coordHud {"
        "  background: rgba(0,0,0,0.55);"
        "  color: #D0FFD0;"
        "  border: none;"
        "  border-radius: 4px;"
        "  padding: 2px 8px;"
        "  margin: 8px;"
        "}"
    ));

    // ---- right-side route planning dock ----
    m_routePanel = new RoutePanel;
    m_routeDock = new QDockWidget(QStringLiteral("路径规划"), this);
    m_routeDock->setWidget(m_routePanel);
    m_routeDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable);
    m_routeDock->setMinimumWidth(260);
    addDockWidget(Qt::RightDockWidgetArea, m_routeDock);

    // ---- calibration dock (below route panel) ----
    m_calibPanel = new CalibrationPanel;
    m_calibDock = new QDockWidget(QStringLiteral("坐标校准"), this);
    m_calibDock->setWidget(m_calibPanel);
    m_calibDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable);
    m_calibDock->setMinimumWidth(260);
    addDockWidget(Qt::RightDockWidgetArea, m_calibDock);

    setupFavoritesPanel();

    // Status bar labels
    m_statusCoords = new QLabel(QStringLiteral("坐标: --, --"), this);
    m_statusBuilding = new QLabel(QStringLiteral("建筑: --"), this);
    m_statusUser = new QLabel(QStringLiteral("未登录"), this);

    statusBar()->addWidget(m_statusCoords);
    statusBar()->addWidget(m_statusBuilding, 1);
    statusBar()->addPermanentWidget(m_statusUser);
}

void MainWindow::setupControllers()
{
    m_mapController = new MapController(this);
    m_routeController = new RouteController(this);
    m_searchController = new SearchController(this);
    m_userController = new UserController(this);
    m_updateController = new UpdateController(this);

    m_dbManager = new DatabaseManager(this);
    m_networkManager = new NetworkManager(this);
    m_spatialIndex = new SpatialIndex();

    // Open local database
    QString dbPath = QApplication::applicationDirPath() + "/campus.db";
    if (m_dbManager->open(dbPath)) {
        m_dbManager->initTables();
        m_userDao = new UserDAO(m_dbManager->database(), this);
        m_userController->setUserDAO(m_userDao);
    }

    m_searchController->setSpatialIndex(m_spatialIndex);

    m_updateController->setNetworkManager(m_networkManager);
    m_updateController->setMapController(m_mapController);
    m_updateController->setDatabaseManager(m_dbManager);
}

void MainWindow::setupConnections()
{
    // Map interactions
    connect(m_mapController, &MapController::mapLoaded,
            this, &MainWindow::onMapLoaded);
    connect(m_mapController, &MapController::mapLoadError,
            this, &MainWindow::onMapLoadError);

    connect(m_mapWidget, &MapWidget::buildingClicked,
            this, &MainWindow::onBuildingClicked);
    connect(m_mapWidget, &MapWidget::nodeClicked,
            this, &MainWindow::onNodeClicked);
    connect(m_mapWidget, &MapWidget::pointClicked,
            this, &MainWindow::onMapPointClicked);
    connect(m_mapWidget, &MapWidget::mouseMoved,
            this, [this](const QPointF &pos) {
        if (m_coordHud)
            m_coordHud->setText(QStringLiteral(" %1 , %2 ")
                .arg(int(pos.x())).arg(int(pos.y())));
    });

    connect(m_mapWidget, &MapWidget::coordinateDebug,
            this, [this](const QPointF &pos) {
        m_statusCoords->setText(QStringLiteral("坐标: %1, %2")
            .arg(int(pos.x())).arg(int(pos.y())));
        // Feed into calibration: free click = new coords after building click
        if (m_calibPickIndex >= 0 && m_calibPanel) {
            m_calibPanel->setPickResult(m_calibPickIndex, pos);
            m_calibPickIndex = -1;
            m_calibFillOld = true;
            if (m_calibPairIndex < 1) m_calibPairIndex++;
        }
    });

    // Search
    connect(m_searchBar, &SearchBar::searchRequested,
            this, &MainWindow::onSearchRequested);
    connect(m_searchBar, &SearchBar::resultSelected,
            this, &MainWindow::onSearchResultSelected);

    // Route — from right-side panel
    connect(m_routePanel, &RoutePanel::searchRouteClicked,
            this, &MainWindow::onPlanRouteRequested);

    // Calibration
    connect(m_calibPanel, &CalibrationPanel::pickRequested,
            this, [this](int idx) { m_calibPickIndex = idx; });
    connect(m_calibPanel, &CalibrationPanel::applyCalibration,
            this, [this](QPointF o1, QPointF n1, QPointF o2, QPointF n2) {
        m_mapController->applyCalibration(o1, n1, o2, n2);
        m_mapController->buildScene(m_scene);
        m_mapWidget->fitToScene();
    });
    connect(m_calibPanel, &CalibrationPanel::exportRequested,
            this, [this]() {
        QString json = m_mapController->exportCalibratedJson();
        QFile f(QApplication::applicationDirPath() + "/campus_map_calibrated.json");
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            f.write(json.toUtf8());
            f.close();
            updateStatusBar(QStringLiteral("已导出: campus_map_calibrated.json"));
        }
    });

    // Route — from controller callbacks
    connect(m_routeController, &RouteController::routePlanned,
            this, &MainWindow::onRoutePlanned);
    connect(m_routeController, &RouteController::routePlanFailed,
            this, &MainWindow::onRoutePlanFailed);

    // User
    connect(m_userController, &UserController::loginSuccess,
            this, &MainWindow::onLoginSuccess);
    connect(m_userController, &UserController::loginFailed,
            this, &MainWindow::onLoginFailed);
    connect(m_userController, &UserController::registerSuccess,
            this, &MainWindow::onRegisterSuccess);
    connect(m_userController, &UserController::registerFailed,
            this, &MainWindow::onRegisterFailed);
    connect(m_userController, &UserController::favoritesChanged,
            this, [this]() {
        updateStatusBar(QStringLiteral("收藏已更新"));
    });

    // Update
    connect(m_updateController, &UpdateController::updateAvailable,
            this, &MainWindow::onUpdateAvailable);
    connect(m_updateController, &UpdateController::updateFailed,
            this, &MainWindow::onUpdateFailed);
}

void MainWindow::setupMenus()
{
    QMenu *fileMenu = menuBar()->addMenu(QStringLiteral("文件"));
    QAction *openAction = new QAction(QStringLiteral("打开地图..."), this);
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::onActionOpenMap);
    fileMenu->addAction(openAction);
    fileMenu->addSeparator();
    QAction *quitAction = new QAction(QStringLiteral("退出"), this);
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(quitAction);

    QMenu *viewMenu = menuBar()->addMenu(QStringLiteral("视图"));
    QAction *zoomInAction = new QAction(QStringLiteral("放大"), this);
    zoomInAction->setShortcut(QKeySequence::ZoomIn);
    connect(zoomInAction, &QAction::triggered, this, &MainWindow::onZoomIn);
    viewMenu->addAction(zoomInAction);
    QAction *zoomOutAction = new QAction(QStringLiteral("缩小"), this);
    zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    connect(zoomOutAction, &QAction::triggered, this, &MainWindow::onZoomOut);
    viewMenu->addAction(zoomOutAction);
    QAction *fitAction = new QAction(QStringLiteral("适应窗口"), this);
    connect(fitAction, &QAction::triggered, this, &MainWindow::onFitToWindow);
    viewMenu->addAction(fitAction);

    QMenu *toolMenu = menuBar()->addMenu(QStringLiteral("工具"));
    QAction *planAction = new QAction(QStringLiteral("路径规划..."), this);
    connect(planAction, &QAction::triggered, this, &MainWindow::onActionPlanRoute);
    toolMenu->addAction(planAction);
    QAction *historyAction = new QAction(QStringLiteral("路线历史..."), this);
    connect(historyAction, &QAction::triggered, this, &MainWindow::onActionRouteHistory);
    toolMenu->addAction(historyAction);
    toolMenu->addSeparator();
    m_actionFavorites = new QAction(QStringLiteral("我的收藏"), this);
    connect(m_actionFavorites, &QAction::triggered, this, &MainWindow::onActionFavorites);
    m_actionFavorites->setEnabled(false);
    toolMenu->addAction(m_actionFavorites);

    QMenu *userMenu = menuBar()->addMenu(QStringLiteral("用户"));
    m_actionLogin = new QAction(QStringLiteral("登录 / 注册"), this);
    connect(m_actionLogin, &QAction::triggered, this, &MainWindow::onActionLogin);
    userMenu->addAction(m_actionLogin);
    m_actionLogout = new QAction(QStringLiteral("退出登录"), this);
    connect(m_actionLogout, &QAction::triggered, this, &MainWindow::onActionLogout);
    userMenu->addAction(m_actionLogout);
}

void MainWindow::setupToolbar()
{
    QToolBar *tb = addToolBar(QStringLiteral("主工具栏"));
    tb->setMovable(false);

    QAction *zoomInTb = new QAction(QStringLiteral("放大"), this);
    connect(zoomInTb, &QAction::triggered, this, &MainWindow::onZoomIn);
    tb->addAction(zoomInTb);
    QAction *zoomOutTb = new QAction(QStringLiteral("缩小"), this);
    connect(zoomOutTb, &QAction::triggered, this, &MainWindow::onZoomOut);
    tb->addAction(zoomOutTb);
    QAction *fitTb = new QAction(QStringLiteral("适应"), this);
    connect(fitTb, &QAction::triggered, this, &MainWindow::onFitToWindow);
    tb->addAction(fitTb);
    tb->addSeparator();
    QAction *planTb = new QAction(QStringLiteral("路径规划"), this);
    connect(planTb, &QAction::triggered, this, &MainWindow::onActionPlanRoute);
    tb->addAction(planTb);
}

void MainWindow::updateStatusBar(const QString &msg)
{
    statusBar()->showMessage(msg, 5000);
}

void MainWindow::updateUserActions()
{
    bool loggedIn = m_userController && m_userController->isLoggedIn();
    m_actionLogin->setVisible(!loggedIn);
    m_actionLogout->setVisible(loggedIn);
    if (m_actionFavorites) {
        m_actionFavorites->setEnabled(loggedIn);
    }

    if (loggedIn) {
        m_statusUser->setText(QStringLiteral("用户: %1").arg(m_userController->currentUsername()));
    } else {
        m_statusUser->setText(QStringLiteral("未登录"));
    }

    if (m_selectedBuildingId >= 0)
        updateFavoritesPanel(m_selectedBuildingId);
}

void MainWindow::loadDefaultMap()
{
    // Use paths relative to the build output directory
    QString binDir = QApplication::applicationDirPath();
    // Project root is 3 levels up from shadow build: debug/ → build/ → project/
    QString projectRoot = binDir + "/../../..";

    // Set background image — prefer source assets over build copy
    QString bgPath = projectRoot + "/assets/campus_map.png.png";
    if (!QFile::exists(bgPath))
        bgPath = binDir + "/campus_map.png";
    if (!QFile::exists(bgPath))
        bgPath = binDir + "/../assets/campus_map.png.png";
    if (QFile::exists(bgPath)) {
        m_mapController->setBackgroundImage(bgPath);
    }

    // Load JSON — prefer the user-editable project-root copy
    QString jsonPath = projectRoot + "/campus_map.json";
    if (QFile::exists(jsonPath)) {
        m_mapController->loadFromJson(jsonPath);
    } else if (QFile::exists(binDir + "/campus_map.json")) {
        // Fallback to the shipped copy in the binary dir
        m_mapController->loadFromJson(binDir + "/campus_map.json");
    } else if (m_dbManager && m_dbManager->isOpen()) {
        m_mapController->loadFromDatabase(m_dbManager);
    } else {
        updateStatusBar(QStringLiteral("未找到地图数据"));
        m_scene->addText(QStringLiteral("请通过 文件 -> 打开地图 加载地图数据"));
    }
}

void MainWindow::onMapLoaded()
{
    m_mapController->buildScene(m_scene);
    m_mapWidget->fitToScene();

    // Update search controller with building data
    m_searchController->setBuildings(m_mapController->buildings());

    // Build spatial index
    m_spatialIndex->clear();
    for (const Building &b : m_mapController->buildings()) {
        m_spatialIndex->insert(b.id, b.center);
    }

    // Populate route panel dropdowns with building-labelled node entries
    if (m_routePanel) {
        QVector<QPair<int, QString>> nodeItems;
        for (const Node &n : m_mapController->nodes()) {
            QString label;
            if (n.buildingId >= 0) {
                Building *b = m_mapController->findBuilding(n.buildingId);
                if (b) {
                    label = QStringLiteral("[%1] %2").arg(b->name).arg(n.id);
                } else {
                    label = QStringLiteral("节点 %1").arg(n.id);
                }
            } else {
                label = QStringLiteral("节点 %1").arg(n.id);
            }
            nodeItems.append({n.id, label});
        }
        m_routePanel->setNodes(nodeItems);
    }

    updateStatusBar(QStringLiteral("地图加载完成"));
}

void MainWindow::onMapLoadError(const QString &msg)
{
    QMessageBox::warning(this, QStringLiteral("地图加载失败"), msg);
}

void MainWindow::onBuildingClicked(int buildingId)
{
    m_selectedBuildingId = buildingId;
    Building *b = m_mapController->findBuilding(buildingId);
    if (b) {
        m_statusBuilding->setText(QStringLiteral("建筑: %1 (%2)  →  [%3, %4]")
            .arg(b->name).arg(b->typeString())
            .arg(int(b->center.x())).arg(int(b->center.y())));
        m_mapWidget->centerOnPoint(b->center);

        // Calibration: building click → 旧坐标, free map click → 新坐标
        if (m_calibPanel && m_calibFillOld) {
            m_calibPanel->setOldCoords(m_calibPairIndex, b->center);
            m_calibFillOld = false;
            m_calibPickIndex = m_calibPairIndex;  // next free click fills new coords
        }

        updateFavoritesPanel(buildingId);
    }
}

void MainWindow::onMapPointClicked(const QPointF &scenePos)
{
    m_statusCoords->setText(QStringLiteral("坐标: %1, %2")
        .arg(int(scenePos.x())).arg(int(scenePos.y())));

    // If route dialog is active, allow picking start/end by clicking map
    if (m_routeDialog && m_routeDialog->isVisible()) {
        // Find nearest node
        QVector<int> nearest = m_spatialIndex->searchNearest(scenePos, 1);
        if (!nearest.isEmpty()) {
            Node *n = m_mapController->findNode(nearest.first());
            if (n) {
                if (m_routeStartNode < 0) {
                    m_routeStartNode = n->id;
                    m_routeDialog->setStartPoint(QStringLiteral("节点 %1").arg(n->id), n->id);
                } else {
                    m_routeDialog->setEndPoint(QStringLiteral("节点 %1").arg(n->id), n->id);
                }
            }
        }
    }
}

void MainWindow::onNodeClicked(int nodeId)
{
    Node *n = m_mapController->findNode(nodeId);
    if (!n) return;

    m_statusCoords->setText(QStringLiteral("坐标: %1, %2")
        .arg(int(n->coord.x())).arg(int(n->coord.y())));

    // If route dialog is active, allow picking start/end by clicking nodes
    if (m_routeDialog && m_routeDialog->isVisible()) {
        if (m_routeStartNode < 0) {
            m_routeStartNode = nodeId;
            m_routeDialog->setStartPoint(QStringLiteral("节点 %1").arg(nodeId), nodeId);
        } else {
            m_routeDialog->setEndPoint(QStringLiteral("节点 %1").arg(nodeId), nodeId);
        }
    }
}

void MainWindow::onSearchRequested(const QString &keyword)
{
    QVector<SearchResult> results = m_searchController->searchByName(keyword);
    m_searchBar->setResults(results);
}

void MainWindow::onSearchResultSelected(int buildingId)
{
    Building *b = m_mapController->findBuilding(buildingId);
    if (b) {
        m_mapWidget->centerOnPoint(b->center);
        onBuildingClicked(buildingId);
    }
}

void MainWindow::onPlanRouteRequested(int startNode, int endNode, int strategy)
{
    RouteStrategy rs = (strategy == 0) ? RouteStrategy::ShortestDistance
                                        : RouteStrategy::ShortestTime;
    Route route = m_routeController->planRoute(m_mapController->graph(), startNode, endNode, rs);

    if (m_userController->isLoggedIn() && !route.nodeIds.isEmpty()) {
        QJsonObject json;
        json["startNode"] = startNode;
        json["endNode"] = endNode;
        json["strategy"] = strategy;
        json["totalDistance"] = route.totalDistance;
        json["totalTime"] = route.totalTime;
        json["description"] = route.description;
        QJsonArray nodeArr;
        for (int nid : route.nodeIds)
            nodeArr.append(nid);
        json["nodeIds"] = nodeArr;
        QJsonArray pointArr;
        for (const QPointF &pt : route.points) {
            QJsonObject ptObj;
            ptObj["x"] = pt.x();
            ptObj["y"] = pt.y();
            pointArr.append(ptObj);
        }
        json["points"] = pointArr;

        m_userController->saveRouteHistory(startNode, endNode,
            QJsonDocument(json).toJson(QJsonDocument::Compact));
    }
}

void MainWindow::onRoutePlanned(const Route &route)
{
    m_mapWidget->highlightRoute(route.points);
    updateStatusBar(route.description);

    if (m_routePanel) {
        QString html = QStringLiteral(
            "<b>距离:</b> %1 m &nbsp;|&nbsp; <b>时间:</b> %2 min<br>"
            "<b>路径:</b> %3")
            .arg(int(route.totalDistance))
            .arg(int(route.totalTime))
            .arg(route.description);
        m_routePanel->setResultText(html);
    }
}

void MainWindow::onRoutePlanFailed(const QString &msg)
{
    QMessageBox::warning(this, QStringLiteral("路径规划失败"), msg);
    if (m_routePanel)
        m_routePanel->setResultText(
            QStringLiteral("<span style='color:red;'>%1</span>").arg(msg));
}

void MainWindow::onLoginRequested(const QString &username, const QString &password)
{
    m_userController->login(username, password);
}

void MainWindow::onRegisterRequested(const QString &username, const QString &password)
{
    m_userController->registerUser(username, password);
}

void MainWindow::onLoginSuccess(const QString &username)
{
    updateUserActions();
    updateStatusBar(QStringLiteral("欢迎, %1").arg(username));
    if (m_loginDialog) {
        m_loginDialog->accept();
    }
}

void MainWindow::onLoginFailed(const QString &msg)
{
    QMessageBox::warning(this, QStringLiteral("登录失败"), msg);
}

void MainWindow::onRegisterSuccess(const QString &username)
{
    QMessageBox::information(this, QStringLiteral("注册成功"),
        QStringLiteral("用户 %1 已注册, 请登录").arg(username));
}

void MainWindow::onRegisterFailed(const QString &msg)
{
    QMessageBox::warning(this, QStringLiteral("注册失败"), msg);
}

void MainWindow::onZoomIn()
{
    m_mapWidget->zoomIn();
}

void MainWindow::onZoomOut()
{
    m_mapWidget->zoomOut();
}

void MainWindow::onFitToWindow()
{
    m_mapWidget->fitToScene();
}

void MainWindow::onActionOpenMap()
{
    QString path = QFileDialog::getOpenFileName(this,
        QStringLiteral("打开地图数据"), QString(),
        QStringLiteral("JSON 文件 (*.json);;所有文件 (*.*)"));
    if (!path.isEmpty()) {
        m_mapController->loadFromJson(path);
    }
}

void MainWindow::onActionPlanRoute()
{
    if (!m_routeDialog) {
        m_routeDialog = new RouteDialog(this);
        connect(m_routeDialog, &RouteDialog::planRouteRequested,
                this, &MainWindow::onPlanRouteRequested);
    }

    m_routeDialog->clearNodeItems();
    for (const Node &n : m_mapController->nodes()) {
        QString label = QStringLiteral("节点 %1").arg(n.id);
        if (n.buildingId >= 0) {
            Building *b = m_mapController->findBuilding(n.buildingId);
            if (b) {
                label = QStringLiteral("%1 (%2)")
                    .arg(b->name)
                    .arg(n.isEntrance ? QStringLiteral("入口") : QStringLiteral("节点 %1").arg(n.id));
            }
        }
        m_routeDialog->addNodeItem(label, n.id);
    }

    m_routeDialog->show();
    m_routeStartNode = -1;
}

void MainWindow::onActionRouteHistory()
{
    if (!m_userController->isLoggedIn()) {
        QMessageBox::information(this, QStringLiteral("路线历史"),
            QStringLiteral("请先登录"));
        return;
    }

    if (!m_routeHistoryDialog) {
        m_routeHistoryDialog = new RouteHistoryDialog(this);
        connect(m_routeHistoryDialog, &RouteHistoryDialog::routeSelected,
                this, [this](int, int, const QString &routeJson) {
            QJsonDocument doc = QJsonDocument::fromJson(routeJson.toUtf8());
            if (!doc.isObject()) return;
            QJsonObject json = doc.object();
            QJsonArray pointArr = json["points"].toArray();
            QVector<QPointF> points;
            for (const QJsonValue &v : pointArr) {
                QJsonObject pt = v.toObject();
                points.append(QPointF(pt["x"].toDouble(), pt["y"].toDouble()));
            }
            m_mapWidget->highlightRoute(points);
            updateStatusBar(json["description"].toString());
        });
    }

    m_routeHistoryDialog->loadRecords(m_userController->routeHistoryRecords());
    m_routeHistoryDialog->show();
}

void MainWindow::onActionLogin()
{
    if (!m_loginDialog) {
        m_loginDialog = new LoginDialog(this);
        connect(m_loginDialog, &LoginDialog::loginRequested,
                this, &MainWindow::onLoginRequested);
        connect(m_loginDialog, &LoginDialog::registerRequested,
                this, &MainWindow::onRegisterRequested);
    }
    m_loginDialog->show();
}

void MainWindow::onActionLogout()
{
    m_userController->logout();
    updateUserActions();
    updateStatusBar(QStringLiteral("已退出登录"));
}

void MainWindow::onActionFavorites()
{
    if (!m_userController->isLoggedIn()) return;
    QVector<int> favs = m_userController->favoriteBuildingIds();
    QStringList names;
    for (int id : favs) {
        Building *b = m_mapController->findBuilding(id);
        if (b) names.append(b->name);
    }
    QMessageBox::information(this, QStringLiteral("我的收藏"),
        names.isEmpty() ? QStringLiteral("暂无收藏") : names.join("\n"));
}

void MainWindow::onUpdateAvailable(const QString &version, const QString &url)
{
    m_pendingUpdateUrl = url;
    auto ret = QMessageBox::question(this, QStringLiteral("发现更新"),
        QStringLiteral("新版本 %1 可用, 是否下载?").arg(version));
    if (ret == QMessageBox::Yes) {
        m_updateController->applyUpdate(m_pendingUpdateUrl);
    }
}

void MainWindow::onUpdateFailed(const QString &msg)
{
    qDebug() << "Update check failed:" << msg;
}

void MainWindow::setupFavoritesPanel()
{
    m_favDock = new QDockWidget(QStringLiteral("建筑详情"), this);
    m_favDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable);
    m_favDock->setMinimumWidth(200);

    QWidget *panel = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(panel);

    m_favNameLabel = new QLabel(QStringLiteral("点击地图上的建筑"), panel);
    m_favNameLabel->setWordWrap(true);
    QFont nameFont = m_favNameLabel->font();
    nameFont.setBold(true);
    nameFont.setPointSize(12);
    m_favNameLabel->setFont(nameFont);

    m_favTypeLabel = new QLabel(panel);
    m_favDescLabel = new QLabel(panel);
    m_favDescLabel->setWordWrap(true);

    m_favToggleBtn = new QPushButton(panel);
    m_favToggleBtn->setEnabled(false);
    m_favToggleBtn->setVisible(false);

    layout->addWidget(m_favNameLabel);
    layout->addWidget(m_favTypeLabel);
    layout->addWidget(m_favDescLabel);
    layout->addWidget(m_favToggleBtn);
    layout->addStretch();

    m_favDock->setWidget(panel);
    addDockWidget(Qt::RightDockWidgetArea, m_favDock);

    connect(m_favToggleBtn, &QPushButton::clicked, this, [this]() {
        if (!m_userController->isLoggedIn() || m_selectedBuildingId < 0) return;
        QVector<int> favs = m_userController->favoriteBuildingIds();
        if (favs.contains(m_selectedBuildingId)) {
            m_userController->removeFavorite(m_selectedBuildingId);
        } else {
            m_userController->addFavorite(m_selectedBuildingId);
        }
        updateFavoritesPanel(m_selectedBuildingId);
    });
}

void MainWindow::updateFavoritesPanel(int buildingId)
{
    Building *b = m_mapController->findBuilding(buildingId);
    if (!b) return;

    m_favNameLabel->setText(b->name);
    m_favTypeLabel->setText(QStringLiteral("类型: %1").arg(b->typeString()));
    m_favDescLabel->setText(b->description.isEmpty()
        ? QStringLiteral("暂无描述") : b->description);

    if (m_userController->isLoggedIn()) {
        m_favToggleBtn->setVisible(true);
        m_favToggleBtn->setEnabled(true);
        QVector<int> favs = m_userController->favoriteBuildingIds();
        bool isFav = favs.contains(buildingId);
        m_favToggleBtn->setText(isFav
            ? QStringLiteral("★ 取消收藏")
            : QStringLiteral("☆ 添加收藏"));
    } else {
        m_favToggleBtn->setVisible(false);
        m_favToggleBtn->setEnabled(false);
    }
}
