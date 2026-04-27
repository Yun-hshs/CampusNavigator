#include "graph.h"

void Graph::addNode(int id, const std::string& name, double x, double y) {
    nodes.push_back({id, name, x, y});
    adjList.emplace_back();
    nameToId[name] = id;
}

void Graph::addEdge(int from, int to, double weight) {
    adjList[from].push_back({to, weight});
    adjList[to].push_back({from, weight}); // 双向边
}

int Graph::getNodeId(const std::string& name) const {
    return nameToId.at(name);
}

const Node& Graph::getNode(int id) const {
    return nodes[id];
}

const std::vector<Edge>& Graph::getNeighbors(int id) const {
    return adjList[id];
}

int Graph::size() const {
    return nodes.size();
}