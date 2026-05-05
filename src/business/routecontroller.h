#ifndef ROUTECONTROLLER_H
#define ROUTECONTROLLER_H

#include <QObject>
#include "graph.h"
#include "route.h"

enum class RouteStrategy {
    ShortestDistance,  // Dijkstra — 最短距离
    ShortestTime       // A* — 最短时间
};

class RouteController : public QObject {
    Q_OBJECT
public:
    explicit RouteController(QObject *parent = nullptr);

    Route planRoute(const Graph &graph, int startNode, int endNode,
                    RouteStrategy strategy = RouteStrategy::ShortestDistance);

    void setAvoidNodes(const QVector<int> &nodeIds);
    void clearAvoidNodes();

signals:
    void routePlanned(const Route &route);
    void routePlanFailed(const QString &msg);

private:
    Route buildRoute(const QVector<int> &nodeIds, const Graph &graph,
                     double distance, bool found);

    QVector<int> m_avoidNodes;
};

#endif // ROUTECONTROLLER_H