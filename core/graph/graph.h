#pragma once
#include <vector>
#include <string>
#include <unordered_map>

struct Node {
    int id;
    std::string name;
    double x, y;
};

struct Edge {
    int to;
    double weight;
};

class Graph {
public:
    void addNode(int id, const std::string& name, double x, double y);
    void addEdge(int from, int to, double weight);

    int getNodeId(const std::string& name) const;
    const Node& getNode(int id) const;
    const std::vector<Edge>& getNeighbors(int id) const;

    int size() const;

private:
    std::vector<Node> nodes;
    std::vector<std::vector<Edge>> adjList;
    std::unordered_map<std::string, int> nameToId;
};