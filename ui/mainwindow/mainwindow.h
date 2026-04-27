#pragma once
#include <QMainWindow>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "graph.h"
#include "mapview.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void onStartNavigation();

private:
    Graph graph;
    MapView* mapView;
    QComboBox* startComboBox;
    QComboBox* endComboBox;
    QPushButton* startButton;
    void setupGraph();
};