#include "ApiServer.h"
#include "data/DataLoader.h"
#include "algorithm/Dijkstra.h"

#include <QTcpSocket>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QUrlQuery>
#include <QDebug>
#include <QtGlobal>

namespace {
constexpr double X_MIN = 280.0;
constexpr double X_MAX = 720.0;
constexpr double Y_MIN = 280.0;
constexpr double Y_MAX = 950.0;

// 陕西科技大学周边经纬度包围盒（可继续通过标定微调）
constexpr double LON_WEST = 108.9810;
constexpr double LON_EAST = 108.9915;
constexpr double LAT_NORTH = 34.3895;
constexpr double LAT_SOUTH = 34.3805;

QPair<double, double> toLatLng(double x, double y) {
    const double nx = qBound(0.0, (x - X_MIN) / (X_MAX - X_MIN), 1.0);
    const double ny = qBound(0.0, (y - Y_MIN) / (Y_MAX - Y_MIN), 1.0);
    const double lon = LON_WEST + (LON_EAST - LON_WEST) * nx;
    const double lat = LAT_NORTH - (LAT_NORTH - LAT_SOUTH) * ny;
    return {lat, lon};
}
}


ApiServer::ApiServer(QObject *parent) : QObject(parent) {
    connect(&m_server, &QTcpServer::newConnection, this, &ApiServer::onNewConnection);
    DataLoader::loadFromJson("data/map.json", m_graph);
}

bool ApiServer::start(quint16 port) {
    const bool ok = m_server.listen(QHostAddress::Any, port);
    if (ok) qInfo() << "API server listening on" << m_server.serverAddress() << m_server.serverPort();
    else qWarning() << "API server failed:" << m_server.errorString();
    return ok;
}

void ApiServer::onNewConnection() {
    while (QTcpSocket *socket = m_server.nextPendingConnection()) {
        connect(socket, &QTcpSocket::readyRead, this, [this, socket]() { handleSocket(socket); });
        connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
    }
}

void ApiServer::handleSocket(QTcpSocket *socket) {
    const QByteArray req = socket->readAll();
    const QList<QByteArray> lines = req.split('\n');
    if (lines.isEmpty()) {
        socket->write(badRequest("empty request"));
        socket->disconnectFromHost();
        return;
    }

    const QList<QByteArray> parts = lines.first().trimmed().split(' ');
    if (parts.size() < 2) {
        socket->write(badRequest("invalid request line"));
        socket->disconnectFromHost();
        return;
    }

    socket->write(handleRequest(QString::fromUtf8(parts[0]), QString::fromUtf8(parts[1])));
    socket->disconnectFromHost();
}

QByteArray ApiServer::handleRequest(const QString &method, const QString &pathWithQuery) const {
    if (method != "GET" && method != "POST") return badRequest("method not supported");

    const QUrl url(pathWithQuery);
    const QString path = url.path();
    const QUrlQuery query(url);

    if (method == "GET" && path == "/api/buildings") {
        QJsonArray buildings;
        for (const Node &n : m_graph.allNodes()) {
            if (n.type != "building") continue;
            QJsonObject o;
            o["id"] = n.id;
            o["name"] = n.name;
            o["type"] = n.type;
            o["description"] = n.description;
            const auto [lat, lon] = toLatLng(n.x, n.y);
            o["latitude"] = lat;
            o["longitude"] = lon;
            buildings.append(o);
        }
        QJsonObject root{{"code", 0}, {"message", "ok"}, {"data", QJsonObject{{"buildings", buildings}}}};
        return jsonResponse(QJsonDocument(root).toJson(QJsonDocument::Compact));
    }

    if (method == "GET" && path == "/api/pois") {
        const QString keyword = query.queryItemValue("keyword").trimmed();
        QJsonArray pois;
        for (const Node &n : m_graph.allNodes()) {
            if (n.type != "building") continue;
            if (!keyword.isEmpty() && !n.name.contains(keyword, Qt::CaseInsensitive) && !n.description.contains(keyword, Qt::CaseInsensitive)) continue;
            QJsonObject o;
            o["id"] = n.id;
            o["name"] = n.name;
            o["type"] = n.type;
            const auto [lat, lon] = toLatLng(n.x, n.y);
            o["latitude"] = lat;
            o["longitude"] = lon;
            pois.append(o);
        }
        QJsonObject root{{"code", 0}, {"message", "ok"}, {"data", QJsonObject{{"pois", pois}}}};
        return jsonResponse(QJsonDocument(root).toJson(QJsonDocument::Compact));
    }

    return notFound();
}

QByteArray ApiServer::jsonResponse(const QByteArray &json, const QByteArray &status) const {
    QByteArray resp;
    resp += "HTTP/1.1 " + status + "\r\n";
    resp += "Content-Type: application/json; charset=utf-8\r\n";
    resp += "Access-Control-Allow-Origin: *\r\n";
    resp += "Content-Length: " + QByteArray::number(json.size()) + "\r\n\r\n";
    resp += json;
    return resp;
}

QByteArray ApiServer::notFound() const {
    QJsonObject root{{"code", 404}, {"message", "not found"}, {"data", QJsonObject()}};
    return jsonResponse(QJsonDocument(root).toJson(QJsonDocument::Compact), "404 Not Found");
}

QByteArray ApiServer::badRequest(const QString &msg) const {
    QJsonObject root{{"code", 400}, {"message", msg}, {"data", QJsonObject()}};
    return jsonResponse(QJsonDocument(root).toJson(QJsonDocument::Compact), "400 Bad Request");
}
