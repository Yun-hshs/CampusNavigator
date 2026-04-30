#include "mainwindow.h"
#include "src/ui/osmmapview.h"
#include "src/ui/mapscene.h"
#include "src/ui/buildingitem.h"
#include "src/business/pathhighlighter.h"
#include "src/data/dataloader.h"
#include <QToolBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QFrame>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("校园导航系统 — Campus Navigator");
    resize(1280, 850);

    m_mapScene    = new MapScene(this);
    m_highlighter = new PathHighlighter(m_mapScene, this);

    setupUi();
    setupToolBar();
    setupInfoPanel();
    loadMapData();

    connect(m_mapScene, &MapScene::buildingClicked,
            this, &MainWindow::onBuildingClicked);
}

void MainWindow::setupUi() {
    m_osmView = new OsmMapView(m_mapScene, this);
    setCentralWidget(m_osmView);
}

void MainWindow::setupToolBar() {
    QToolBar* toolbar = addToolBar("导航");
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(20, 20));
    toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolbar->setStyleSheet(
        "QToolBar { border-bottom: 1px solid #ccc; padding: 4px 10px;"
        " background: #FAFAFA; spacing: 8px; }");

    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("搜索建筑...");
    m_searchEdit->setMaximumWidth(180);
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setStyleSheet(
        "QLineEdit { padding: 4px 8px; border: 1px solid #ccc; border-radius: 4px; }");
    connect(m_searchEdit, &QLineEdit::returnPressed,
            this, &MainWindow::onSearchReturn);

    m_startCombo = new QComboBox();
    m_startCombo->setMinimumWidth(130);
    m_startCombo->setToolTip("选择起点建筑");
    m_startCombo->setStyleSheet(
        "QComboBox { padding: 4px 8px; border: 1px solid #ccc; border-radius: 4px; }");

    m_endCombo = new QComboBox();
    m_endCombo->setMinimumWidth(130);
    m_endCombo->setToolTip("选择终点建筑");
    m_endCombo->setStyleSheet(
        "QComboBox { padding: 4px 8px; border: 1px solid #ccc; border-radius: 4px; }");

    m_navigateBtn = new QPushButton("路径规划");
    m_navigateBtn->setCursor(Qt::PointingHandCursor);
    m_navigateBtn->setStyleSheet(
        "QPushButton { background-color: #3A7BD5; color: white;"
        " padding: 6px 22px; border-radius: 4px; font-weight: bold;"
        " border: none; }"
        "QPushButton:hover { background-color: #5A9BE5; }");

    m_clearBtn = new QPushButton("清除");
    m_clearBtn->setCursor(Qt::PointingHandCursor);
    m_clearBtn->setStyleSheet(
        "QPushButton { padding: 6px 16px; border-radius: 4px;"
        " border: 1px solid #ccc; background: #f5f5f5; }"
        "QPushButton:hover { background-color: #E0E0E0; }");

    connect(m_navigateBtn, &QPushButton::clicked, this, &MainWindow::onNavigate);
    connect(m_clearBtn,    &QPushButton::clicked, this, &MainWindow::onClear);

    auto lbl = [](const QString& t) {
        QLabel* l = new QLabel(t);
        l->setStyleSheet("font-weight: bold; color: #555; font-size: 13px;");
        return l;
    };

    toolbar->addWidget(lbl("搜索"));
    toolbar->addWidget(m_searchEdit);
    toolbar->addSeparator();
    toolbar->addWidget(lbl("起点"));
    toolbar->addWidget(m_startCombo);
    toolbar->addWidget(lbl("终点"));
    toolbar->addWidget(m_endCombo);
    toolbar->addSeparator();
    toolbar->addWidget(m_navigateBtn);
    toolbar->addWidget(m_clearBtn);
}

void MainWindow::setupInfoPanel() {
    QDockWidget* dock = new QDockWidget("校园地图信息", this);
    dock->setFeatures(QDockWidget::DockWidgetMovable |
                      QDockWidget::DockWidgetFloatable);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dock->setMinimumWidth(240);

    QWidget* panel = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(10);

    QGroupBox* infoGroup = new QGroupBox("选中建筑");
    infoGroup->setStyleSheet(
        "QGroupBox { font-weight: bold; border: 1px solid #ddd;"
        " border-radius: 4px; margin-top: 8px; padding-top: 16px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 4px; }");
    QVBoxLayout* gLayout = new QVBoxLayout(infoGroup);

    m_infoTitle = new QLabel("—");
    m_infoTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #333;");

    m_infoType = new QLabel("");
    m_infoType->setStyleSheet("font-size: 12px; color: #888;");

    m_infoDetail = new QLabel("点击地图上的建筑查看详情");
    m_infoDetail->setWordWrap(true);
    m_infoDetail->setStyleSheet("color: #666; font-size: 12px;");

    gLayout->addWidget(m_infoTitle);
    gLayout->addWidget(m_infoType);
    gLayout->addWidget(m_infoDetail);
    gLayout->addStretch();
    layout->addWidget(infoGroup);

    QGroupBox* pathGroup = new QGroupBox("路径信息");
    pathGroup->setStyleSheet(infoGroup->styleSheet());
    QVBoxLayout* pLayout = new QVBoxLayout(pathGroup);

    m_pathInfo = new QLabel("选择起点和终点后点击「路径规划」");
    m_pathInfo->setWordWrap(true);
    m_pathInfo->setStyleSheet("color: #555; font-size: 12px;");
    pLayout->addWidget(m_pathInfo);
    layout->addWidget(pathGroup);

    QGroupBox* legendGroup = new QGroupBox("图例");
    legendGroup->setStyleSheet(infoGroup->styleSheet());
    QVBoxLayout* lLayout = new QVBoxLayout(legendGroup);
    lLayout->setSpacing(6);

    struct LegendEntry { QString text; QColor color; };
    const QVector<LegendEntry> entries = {
        {"教学楼 / 实验室",  BuildingItem::colorForType("teaching")},
        {"学生宿舍",          BuildingItem::colorForType("dorm")},
        {"食堂 / 餐厅",       BuildingItem::colorForType("dining")},
        {"运动场馆",          BuildingItem::colorForType("sports")},
        {"生活服务",          BuildingItem::colorForType("service")},
        {"校门",              BuildingItem::colorForType("gate")},
        {"其他",              BuildingItem::colorForType("other")},
    };

    for (const auto& e : entries) {
        QHBoxLayout* row = new QHBoxLayout();
        QLabel* swatch = new QLabel();
        swatch->setFixedSize(14, 14);
        swatch->setStyleSheet(
            QString("background-color: %1; border: 1px solid #999;"
                    " border-radius: 2px;").arg(e.color.lighter(110).name()));
        QLabel* label = new QLabel(e.text);
        label->setStyleSheet("font-size: 12px; color: #555;");
        row->addWidget(swatch);
        row->addWidget(label);
        row->addStretch();
        lLayout->addLayout(row);
    }

    {
        QHBoxLayout* row = new QHBoxLayout();
        QFrame* line = new QFrame();
        line->setFixedSize(30, 4);
        line->setStyleSheet("background-color: #646469; border-radius: 1px;");
        row->addWidget(line);
        row->addWidget(new QLabel("主干道"));
        row->addStretch();
        lLayout->addLayout(row);
    }
    {
        QHBoxLayout* row = new QHBoxLayout();
        QFrame* line = new QFrame();
        line->setFixedSize(30, 3);
        line->setStyleSheet("background-color: #919196; border-radius: 1px;");
        row->addWidget(line);
        row->addWidget(new QLabel("次干道"));
        row->addStretch();
        lLayout->addLayout(row);
    }
    {
        QHBoxLayout* row = new QHBoxLayout();
        QFrame* line = new QFrame();
        line->setFixedSize(30, 2);
        line->setStyleSheet("background-color: #AFB0B4; border-radius: 1px;");
        row->addWidget(line);
        row->addWidget(new QLabel("支路"));
        row->addStretch();
        lLayout->addLayout(row);
    }

    lLayout->addStretch();
    layout->addWidget(legendGroup);
    layout->addStretch();

    dock->setWidget(panel);
    addDockWidget(Qt::RightDockWidgetArea, dock);
}

void MainWindow::loadMapData() {
    CampusData data = DataLoader::loadFromJson("data/campus_map.json");
    if (data.buildings.isEmpty()) {
        QMessageBox::warning(this, "警告", "无法加载地图数据文件 data/campus_map.json");
        return;
    }

    m_mapScene->loadMap(data.buildings, data.roads);

    for (const auto& b : data.buildings) {
        m_startCombo->addItem(b.name, b.id);
        m_endCombo->addItem(b.name, b.id);
    }

    // Center view on the middle of the data
    double cx = 0, cy = 0;
    for (const auto& b : data.buildings) {
        cx += b.x + b.w / 2;
        cy += b.y + b.h / 2;
    }
    int n = data.buildings.size();
    m_osmView->setViewCenter(cx / n, cy / n);

    statusBar()->showMessage(
        QString("已加载 %1 栋建筑, %2 条道路")
            .arg(data.buildings.size()).arg(data.roads.size()), 4000);
}

void MainWindow::onNavigate() {
    int startId = m_startCombo->currentData().toInt();
    int endId   = m_endCombo->currentData().toInt();

    if (startId == endId) {
        QMessageBox::information(this, "提示", "起点和终点不能相同");
        return;
    }

    auto result = Dijkstra::findPath(m_mapScene->graph(), startId, endId);

    if (!result.reachable) {
        QMessageBox::information(this, "提示", "无法到达目标建筑");
        m_pathInfo->setText("无法到达目标建筑");
        return;
    }

    m_highlighter->showPath(result);

    m_pathInfo->setText(
        QString("路径长度: %1m\n经过 %2 个建筑节点")
            .arg(result.totalWeight, 0, 'f', 1)
            .arg(result.path.size()));
    statusBar()->showMessage(
        QString("路径长度: %1m, 经过 %2 个节点")
            .arg(result.totalWeight, 0, 'f', 1)
            .arg(result.path.size()));
}

void MainWindow::onClear() {
    m_highlighter->clearPath();
    m_pathInfo->setText("选择起点和终点后点击「路径规划」");
    statusBar()->clearMessage();
}

void MainWindow::onBuildingClicked(int id, const QString& name, const QString& desc) {
    m_infoTitle->setText(name);

    const Building& b = m_mapScene->graph().building(id);
    static const QHash<QString, QString> typeNames = {
        {"teaching", "教学楼 / 实验室"},
        {"dorm",     "学生宿舍"},
        {"dining",   "食堂 / 餐厅"},
        {"sports",   "运动场馆"},
        {"service",  "生活服务"},
        {"gate",     "校门"},
        {"other",    "其他"},
    };
    m_infoType->setText(
        QString("类型: %1").arg(typeNames.value(b.type, b.type)));
    m_infoDetail->setText(desc.isEmpty() ? name : desc);

    int startIdx = m_startCombo->findData(id);
    if (startIdx >= 0)
        m_startCombo->setCurrentIndex(startIdx);

    int endIdx = m_endCombo->findData(id);
    if (endIdx >= 0)
        m_endCombo->setCurrentIndex(endIdx);
}

void MainWindow::onSearchReturn() {
    QString keyword = m_searchEdit->text().trimmed();
    if (keyword.isEmpty()) return;

    for (int i = 0; i < m_startCombo->count(); ++i) {
        QString name = m_startCombo->itemText(i);
        if (name.contains(keyword, Qt::CaseInsensitive)) {
            int id = m_startCombo->itemData(i).toInt();
            m_startCombo->setCurrentIndex(i);
            m_mapScene->clearHighlight();
            m_mapScene->selectAndCenterBuilding(id);
            statusBar()->showMessage("搜索到: " + name, 2000);
            return;
        }
    }
    statusBar()->showMessage("未找到: " + keyword, 2000);
}
