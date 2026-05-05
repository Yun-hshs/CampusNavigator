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

    // ── Nodes ──
    const QJsonArray nodes = root["nodes"].toArray();
    for (const auto& v : nodes) {
        QJsonObject o = v.toObject();
        graph.addNode(o["id"].toInt(), o["name"].toString(),
                      o["x"].toDouble(), o["y"].toDouble(),
                      o["description"].toString(),
                      o["type"].toString("building"),
                      o["sprite"].toString("default"));
    }

    // ── Edges ──
    const QJsonArray edges = root["edges"].toArray();
    for (const auto& v : edges) {
        QJsonObject o = v.toObject();
        graph.addEdge(o["from"].toInt(), o["to"].toInt(),
                      o["weight"].toDouble(),
                      o["type"].toString("main_road"));
    }

    // ── Areas (terrain polygons) ──
    const QJsonArray areas = root["areas"].toArray();
    for (const auto& v : areas) {
        QJsonObject o = v.toObject();
        QPolygonF poly;
        const QJsonArray verts = o["vertices"].toArray();
        for (const auto& vp : verts) {
            QJsonArray pt = vp.toArray();
            poly << QPointF(pt[0].toDouble(), pt[1].toDouble());
        }
        graph.addArea(o["id"].toInt(), o["name"].toString(),
                      o["type"].toString(), poly,
                      o["zOrder"].toInt(-80));
    }

    return true;
}
