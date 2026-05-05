#ifndef ROUTEDIALOG_H
#define ROUTEDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

class RouteDialog : public QDialog {
    Q_OBJECT
public:
    explicit RouteDialog(QWidget *parent = nullptr);

    void setStartPoint(const QString &name, int nodeId);
    void setEndPoint(const QString &name, int nodeId);
    void setRouteInfo(const QString &description);

    void addNodeItem(const QString &name, int nodeId);
    void clearNodeItems();

    int startNodeId() const;
    int endNodeId() const;
    int strategyIndex() const;  // 0=Dijkstra, 1=AStar

signals:
    void planRouteRequested(int startNode, int endNode, int strategy);

private:
    QComboBox *m_startCombo;
    QComboBox *m_endCombo;
    QComboBox *m_strategyCombo;
    QLabel *m_infoLabel;
    QPushButton *m_planBtn;
};

#endif // ROUTEDIALOG_H