#include "DataLoader.h"
#include "graph/Graph.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

bool DataLoader::loadFromJson(const QString& filePath, Graph& graph) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject root = doc.object();

    const QJsonArray nodes = root["nodes"].toArray();
    for (const auto& v : nodes) {
        QJsonObject o = v.toObject();
        graph.addNode(o["id"].toInt(), o["name"].toString(),
                      o["x"].toDouble(), o["y"].toDouble());
    }

    const QJsonArray edges = root["edges"].toArray();
    for (const auto& v : edges) {
        QJsonObject o = v.toObject();
        graph.addEdge(o["from"].toInt(), o["to"].toInt(),
                      o["weight"].toDouble());
    }

    return true;
}
