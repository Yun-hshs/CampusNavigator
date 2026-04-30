#include "dataloader.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

CampusData DataLoader::loadFromJson(const QString& filePath) {
    CampusData data;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return data;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject root = doc.object();

    const QJsonArray buildingsArr = root["buildings"].toArray();
    for (const auto& val : buildingsArr) {
        QJsonObject obj = val.toObject();
        Building b;
        b.id   = obj["id"].toInt();
        b.name = obj["name"].toString();
        b.x    = obj["x"].toDouble();
        b.y    = obj["y"].toDouble();
        b.w    = obj["w"].toDouble(55);
        b.h    = obj["h"].toDouble(35);
        b.type = obj["type"].toString("teaching");
        b.desc = obj["desc"].toString();
        data.buildings.append(b);
    }

    const QJsonArray roadsArr = root["roads"].toArray();
    for (const auto& val : roadsArr) {
        QJsonObject obj = val.toObject();
        Road r;
        r.name = obj["name"].toString();
        r.type = obj["type"].toString("sub");
        const QJsonArray pts = obj["path"].toArray();
        for (const auto& p : pts) {
            QJsonObject pt = p.toObject();
            r.path.append(QPointF(pt["x"].toDouble(), pt["y"].toDouble()));
        }
        data.roads.append(r);
    }

    return data;
}
