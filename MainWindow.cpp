#include "MainWindow.h"
#include "view/MapView.h"
#include "data/DataLoader.h"
#include "algorithm/Dijkstra.h"
#include <QToolBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QFrame>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("校园导航系统 — Campus Navigator");
    resize(1280, 850);
    setStyleSheet(
        "QMainWindow { background: #F5F7FA; }"
        "QDockWidget { font-weight: bold; font-size: 13px; }"
        "QDockWidget::title { background: #3A7BD5; color: white;"
        " padding: 6px 10px; }"
        "QStatusBar { background: #F0F2F5; border-top: 1px solid #ddd;"
        " font-size: 12px; color: #555; }");

    setupUi();
    setupToolBar();
    setupInfoPanel();
    loadMapData();

    connect(m_mapView, &MapView::nodeClicked,
            this, &MainWindow::onNodeClicked);
}

void MainWindow::setupUi() {
    m_mapView = new MapView(this);
    setCentralWidget(m_mapView);
}

void MainWindow::setupToolBar() {
    QToolBar* tb = addToolBar("导航");
    tb->setMovable(false);
    tb->setIconSize(QSize(20, 20));
    tb->setStyleSheet(
        "QToolBar { border-bottom: 2px solid #3A7BD5; padding: 6px 12px;"
        " background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        " stop:0 #FAFBFD, stop:1 #F0F2F5); spacing: 10px; }");

    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("输入地点名称搜索...");
    m_searchEdit->setMaximumWidth(200);
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setStyleSheet(
        "QLineEdit { border: 1px solid #ccc; border-radius: 4px;"
        " padding: 5px 10px; font-size: 13px; background: white; }"
        "QLineEdit:focus { border-color: #3A7BD5; }");
    connect(m_searchEdit, &QLineEdit::returnPressed,
            this, &MainWindow::onSearchReturn);

    m_startCombo = new QComboBox();
    m_startCombo->setMinimumWidth(130);
    m_startCombo->setStyleSheet(
        "QComboBox { border: 1px solid #ccc; border-radius: 4px;"
        " padding: 5px 10px; font-size: 13px; background: white; }"
        "QComboBox:focus { border-color: #3A7BD5; }");

    m_endCombo = new QComboBox();
    m_endCombo->setMinimumWidth(130);
    m_endCombo->setStyleSheet(
        "QComboBox { border: 1px solid #ccc; border-radius: 4px;"
        " padding: 5px 10px; font-size: 13px; background: white; }"
        "QComboBox:focus { border-color: #3A7BD5; }");

    m_navigateBtn = new QPushButton("  路径规划");
    m_navigateBtn->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        " stop:0 #4A8BE5, stop:1 #3A7BD5); color: white;"
        " padding: 7px 24px; border-radius: 5px; font-weight: bold;"
        " font-size: 13px; border: none; }"
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        " stop:0 #5A9BE5, stop:1 #4A8BE5); }"
        "QPushButton:pressed { background: #2A6BC5; }");
    connect(m_navigateBtn, &QPushButton::clicked, this, &MainWindow::onNavigate);

    m_clearBtn = new QPushButton("  清除");
    m_clearBtn->setStyleSheet(
        "QPushButton { padding: 7px 18px; border-radius: 5px;"
        " border: 1px solid #bbb; background: #f8f8f8; font-size: 13px; }"
        "QPushButton:hover { background: #e8e8e8; }"
        "QPushButton:pressed { background: #d8d8d8; }");
    connect(m_clearBtn, &QPushButton::clicked, this, &MainWindow::onClear);

    auto lbl = [](const QString& t) {
        auto* l = new QLabel(t);
        l->setStyleSheet("font-weight: bold; color: #555; font-size: 13px;");
        return l;
    };

    tb->addWidget(lbl("搜索"));
    tb->addWidget(m_searchEdit);
    tb->addSeparator();
    tb->addWidget(lbl("起点"));
    tb->addWidget(m_startCombo);
    tb->addWidget(lbl("终点"));
    tb->addWidget(m_endCombo);
    tb->addSeparator();
    tb->addWidget(m_navigateBtn);
    tb->addWidget(m_clearBtn);
    tb->addSeparator();

    auto btn = [](const QString& text) {
        auto* b = new QPushButton(text);
        b->setFixedSize(34, 30);
        b->setStyleSheet(
            "QPushButton { border: 1px solid #ccc; border-radius: 5px;"
            " background: #f0f2f5; font-size: 16px; font-weight: bold;"
            " color: #444; }"
            "QPushButton:hover { background: #e0e3e8; border-color: #3A7BD5; }"
            "QPushButton:pressed { background: #d0d3d8; }");
        return b;
    };

    auto* zoomInBtn  = btn("+");
    auto* zoomOutBtn = btn("-");
    auto* fitBtn     = btn("⬜");
    fitBtn->setToolTip("适应窗口");

    connect(zoomInBtn,  &QPushButton::clicked, m_mapView, &MapView::zoomIn);
    connect(zoomOutBtn, &QPushButton::clicked, m_mapView, &MapView::zoomOut);
    connect(fitBtn,     &QPushButton::clicked, m_mapView, &MapView::fitMap);

    tb->addWidget(zoomInBtn);
    tb->addWidget(zoomOutBtn);
    tb->addWidget(fitBtn);
}

void MainWindow::setupInfoPanel() {
    auto* dock = new QDockWidget("信息面板", this);
    dock->setFeatures(QDockWidget::DockWidgetMovable |
                      QDockWidget::DockWidgetFloatable);
    dock->setMinimumWidth(240);

    auto* panel = new QWidget();
    auto* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(10);

    // ── 选中节点 ──
    auto* infoGroup = new QGroupBox("选中节点");
    infoGroup->setStyleSheet(
        "QGroupBox { font-weight: bold; border: 1px solid #E0E4EA;"
        " border-radius: 6px; margin-top: 10px; padding-top: 18px;"
        " background: #FAFBFD; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 12px;"
        " padding: 0 6px; color: #3A7BD5; }");
    auto* gLayout = new QVBoxLayout(infoGroup);

    m_infoTitle = new QLabel("—");
    m_infoTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #333;");

    m_infoDetail = new QLabel("点击地图上的节点选择起终点");
    m_infoDetail->setWordWrap(true);
    m_infoDetail->setStyleSheet("color: #666; font-size: 12px;");

    gLayout->addWidget(m_infoTitle);
    gLayout->addWidget(m_infoDetail);
    gLayout->addStretch();
    layout->addWidget(infoGroup);

    // ── 路径信息 ──
    auto* pathGroup = new QGroupBox("路径信息");
    pathGroup->setStyleSheet(infoGroup->styleSheet());
    auto* pLayout = new QVBoxLayout(pathGroup);

    m_pathLabel = new QLabel("—");
    m_pathLabel->setStyleSheet(
        "font-size: 22px; font-weight: bold; color: #E6321E;"
        " padding: 6px 0; border-bottom: 1px solid #eee;");
    m_pathLabel->setAlignment(Qt::AlignCenter);

    m_pathInfo = new QLabel("第一次点击选起点，第二次点击选终点");
    m_pathInfo->setWordWrap(true);
    m_pathInfo->setStyleSheet("color: #555; font-size: 12px;");

    pLayout->addWidget(m_pathLabel);
    pLayout->addWidget(m_pathInfo);
    layout->addWidget(pathGroup);

    // ── 状态提示 ──
    m_statusLabel = new QLabel("就绪");
    m_statusLabel->setStyleSheet(
        "color: #3A7BD5; font-weight: bold; font-size: 14px;"
        " padding: 8px; background: #E8F0FE; border-radius: 4px;");
    layout->addWidget(m_statusLabel);

    layout->addStretch();
    dock->setWidget(panel);
    addDockWidget(Qt::RightDockWidgetArea, dock);
}

void MainWindow::loadMapData() {
    if (!DataLoader::loadFromJson("data/map.json", m_graph)) {
        QMessageBox::warning(this, "警告", "无法加载 data/map.json");
        return;
    }

    m_mapView->setGraph(&m_graph);
    m_mapView->drawMap();

    for (const auto& n : m_graph.allNodes()) {
        m_startCombo->addItem(n.name, n.id);
        m_endCombo->addItem(n.name, n.id);
    }

    statusBar()->showMessage(
        QString("已加载 %1 个节点").arg(m_graph.nodeCount()), 4000);
}

// ═══════════════════════════════════════════════════════════════════════════
// Node click — two-click flow
// ═══════════════════════════════════════════════════════════════════════════

void MainWindow::onNodeClicked(int id) {
    const Node& n = m_graph.node(id);
    m_infoTitle->setText(n.name);
    m_infoDetail->setText(QString("坐标: (%1, %2)").arg(n.x).arg(n.y));

    m_clickCount++;

    if (m_clickCount == 1) {
        // First click → start
        m_startId = id;
        m_endId = -1;
        m_mapView->clearPath();
        m_pathLabel->setText("—");

        int idx = m_startCombo->findData(id);
        if (idx >= 0) m_startCombo->setCurrentIndex(idx);

        m_pathInfo->setText(
            QString("起点: %1\n请再次点击选择终点").arg(n.name));
        m_statusLabel->setText("请选择终点");

    } else {
        // Second click → end, navigate
        if (id == m_startId) {
            m_clickCount = 1;
            m_pathInfo->setText("终点不能与起点相同，请重新选择");
            return;
        }

        m_endId = id;
        m_clickCount = 0;

        int idx = m_endCombo->findData(id);
        if (idx >= 0) m_endCombo->setCurrentIndex(idx);

        navigate(m_startId, m_endId);
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Navigate — Dijkstra + draw + animate
// ═══════════════════════════════════════════════════════════════════════════

void MainWindow::onNavigate() {
    int startId = m_startCombo->currentData().toInt();
    int endId   = m_endCombo->currentData().toInt();

    if (startId == endId) {
        QMessageBox::information(this, "提示", "起点和终点不能相同");
        return;
    }
    navigate(startId, endId);
}

void MainWindow::navigate(int startId, int endId) {
    m_mapView->clearPath();

    auto path = Dijkstra::findPath(m_graph, startId, endId);

    if (path.isEmpty()) {
        m_pathInfo->setText("无法到达目标节点");
        m_statusLabel->setText("不可达");
        return;
    }

    m_mapView->drawPath(path);
    m_mapView->highlightStartEnd(startId, endId);
    m_mapView->animatePath(path);

    // Calculate total distance
    double totalDist = 0;
    for (int i = 0; i + 1 < path.size(); ++i) {
        for (const auto& e : m_graph.getEdges(path[i])) {
            if (e.to == path[i + 1]) { totalDist += e.weight; break; }
        }
    }

    const Node& s = m_graph.node(startId);
    const Node& t = m_graph.node(endId);

    m_pathLabel->setText(QString("%1 m").arg(totalDist, 0, 'f', 1));
    m_pathInfo->setText(
        QString("%1 → %2\n经过 %3 个节点")
            .arg(s.name, t.name)
            .arg(path.size()));
    m_statusLabel->setText("导航完成");

    statusBar()->showMessage(
        QString("%1 → %2, 路径长度: %3m, 经过 %4 个节点")
            .arg(s.name, t.name)
            .arg(totalDist, 0, 'f', 1)
            .arg(path.size()));
}

// ═══════════════════════════════════════════════════════════════════════════
// Clear & Search
// ═══════════════════════════════════════════════════════════════════════════

void MainWindow::onClear() {
    m_startId = -1;
    m_endId = -1;
    m_clickCount = 0;
    m_mapView->clearPath();
    m_pathLabel->setText("—");
    m_pathInfo->setText("第一次点击选起点，第二次点击选终点");
    m_statusLabel->setText("就绪");
    statusBar()->clearMessage();
}

void MainWindow::onSearchReturn() {
    QString keyword = m_searchEdit->text().trimmed();
    if (keyword.isEmpty()) return;

    for (int i = 0; i < m_startCombo->count(); ++i) {
        if (m_startCombo->itemText(i).contains(keyword, Qt::CaseInsensitive)) {
            int nodeId = m_startCombo->itemData(i).toInt();
            m_startCombo->setCurrentIndex(i);

            // Center map on found node and highlight it
            m_mapView->centerOnNode(nodeId);
            m_mapView->highlightNode(nodeId, QColor(255, 165, 0));

            // Update info panel
            const Node& n = m_graph.node(nodeId);
            m_infoTitle->setText(n.name);
            m_infoDetail->setText(QString("坐标: (%1, %2)").arg(n.x).arg(n.y));

            statusBar()->showMessage("搜索到: " + m_startCombo->itemText(i), 2000);
            return;
        }
    }
    statusBar()->showMessage("未找到: " + keyword, 2000);
}
