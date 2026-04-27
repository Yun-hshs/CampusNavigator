#pragma once
#include "graph.h"
#include <vector>

class Navigator {
public:
    static std::vector<int> dijkstra(const Graph& graph, int start, int end);
};