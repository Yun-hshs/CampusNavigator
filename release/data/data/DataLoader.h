#pragma once
#include <QString>

class Graph;

class DataLoader {
public:
    /// Loads nodes and edges from JSON into the given graph.
    /// Returns true on success.
    static bool loadFromJson(const QString& filePath, Graph& graph);
};
