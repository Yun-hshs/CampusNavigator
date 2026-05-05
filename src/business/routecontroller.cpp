#include "routecontroller.h"
#include "dijkstra.h"
#include "astar.h"
#include <QDebug>

RouteController::RouteController(QObject *parent)
    : QObject(parent)
{
}

Route RouteController::planRoute(const Graph &graph, int startNode, int endNode,
                                  RouteStrategy strategy)
{
    Route route;

    Graph filteredGraph = graph;
    for (int nid : m_avoidNodes)
        filteredGraph.removeNode(nid);

    if (strategy == RouteStrategy::ShortestDistance) {
        auto result = Dijkstra::findShortestPath(filteredGraph, startNode, endNode);
        route = buildRoute(result.path, filteredGraph, result.totalDistance, result.found);
    } else {
        auto result = AStar::findShortestPath(filteredGraph, startNode, endNode);
        route = buildRoute(result.path, filteredGraph, result.totalDistance, result.found);
    }

    if (route.nodeIds.isEmpty()) {
        emit routePlanFailed("No route found");
    } else {
        emit routePlanned(route);
    }

    return route;
}

void RouteController::setAvoidNodes(const QVector<int> &nodeIds)
{
    m_avoidNodes = nodeIds;
}

void RouteController::clearAvoidNodes()
{
    m_avoidNodes.clear();
}

Route RouteController::buildRoute(const QVector<int> &nodeIds, const Graph &graph,
                                   double distance, bool found)
{
    Route route;
    if (!found) return route;

    route.nodeIds = nodeIds;
    route.totalDistance = distance;

    for (int nid : nodeIds) {
        route.points.append(graph.nodeCoord(nid));
    }

    // Estimate walking time (avg 1.2 m/s)
    route.totalTime = distance / 1.2;

    // Build description
    route.description = QStringLiteral("路线: %1 个节点, %2 米, 约 %3 分钟")
        .arg(nodeIds.size())
        .arg(int(distance))
        .arg(int(route.totalTime / 60));

    return route;
}