#include "mainwindow.h"
#include "navigator.h"
#include <QLabel>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout;

    // 控制面板布局
    QHBoxLayout* controlLayout = new QHBoxLayout;
    startComboBox = new QComboBox();
    endComboBox = new QComboBox();
    startButton = new QPushButton("开始导航");

    controlLayout->addWidget(new QLabel("起点"));
    controlLayout->addWidget(startComboBox);
    controlLayout->addWidget(new QLabel("终点"));
    controlLayout->addWidget(endComboBox);
    controlLayout->addWidget(startButton);

    // 地图显示区域
    mapView = new MapView(this);
    mainLayout->addLayout(controlLayout);
    mainLayout->addWidget(mapView);

    centralWidget->setLayout(mainLayout);

    setupGraph();
    connect(startButton, &QPushButton::clicked, this, &MainWindow::onStartNavigation);
}

void MainWindow::setupGraph() {
    // 加载节点和边数据
    std::ifstream nodeFile("data/nodes.txt");
    int id;
    std::string name;
    double x, y;
    while (nodeFile >> id >> name >> x >> y) {
        graph.addNode(id, name, x, y);
    }

    std::ifstream edgeFile("data/edges.txt");
    int from, to;
    double weight;
    while (edgeFile >> from >> to >> weight) {
        graph.addEdge(from, to, weight);
    }

    // 将节点加载到下拉框
    for (int i = 0; i < graph.size(); ++i) {
        startComboBox->addItem(QString::fromStdString(graph.getNode(i).name));
        endComboBox->addItem(QString::fromStdString(graph.getNode(i).name));
    }

    // 绘制地图和边
    mapView->drawNodes(graph);
    mapView->drawEdges(graph);
}

void MainWindow::onStartNavigation() {
    int start = startComboBox->currentIndex();
    int end = endComboBox->currentIndex();

    // 计算最短路径
    std::vector<int> path = Navigator::dijkstra(graph, start, end);

    // 绘制路径
    mapView->drawPath(path, graph);
}