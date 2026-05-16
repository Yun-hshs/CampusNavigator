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
#include <cmath>

namespace {
// 仿射变换: map(x,y) → GCJ-02(lat,lon)
// 标定点 A: 图书馆 map(550,630) → GCJ-02(34.3776775, 108.9772584)
// 标定点 B: 西门   map(280,750) → GCJ-02(34.377092,  108.97174)
constexpr double PX_A = 550.0, PY_A = 630.0;
constexpr double LAT_A = 34.37767749579197, LON_A = 108.97725841533695;

// 仿射系数（从两个标定点解出）
// lat = LAT_A + a1*dx + b1*dy
// lon = LON_A + a2*dx + b2*dy
constexpr double A1 = -2.03614e-8;  // lat 对 x 的灵敏度
constexpr double B1 =  4.90082e-6;  // lat 对 y 的灵敏度
constexpr double A2 = -2.03614e-5;  // lon 对 x 的灵敏度
constexpr double B2 = -5.06939e-6;  // lon 对 y 的灵敏度

QPair<double, double> toLatLng(double x, double y) {
    double dx = x - PX_A;
    double dy = y - PY_A;
    double lat = LAT_A + A1 * dx + B1 * dy;
    double lon = LON_A + A2 * dx + B2 * dy;
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

    // 提取请求体（在空行之后）
    const int bodyStart = req.indexOf("\r\n\r\n");
    const QByteArray body = (bodyStart >= 0) ? req.mid(bodyStart + 4) : QByteArray();

    socket->write(handleRequest(QString::fromUtf8(parts[0]), QString::fromUtf8(parts[1]), body));
    socket->disconnectFromHost();
}

QByteArray ApiServer::handleRequest(const QString &method, const QString &pathWithQuery, const QByteArray &body) const {
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

    if (method == "GET" && path.startsWith("/api/poi/")) {
        const QString idStr = path.mid(9);
        bool ok;
        const int id = idStr.toInt(&ok);
        if (!ok) return badRequest("invalid id");

        for (const Node &n : m_graph.allNodes()) {
            if (n.id == id) {
                QJsonObject o;
                o["id"] = n.id;
                o["name"] = n.name;
                o["type"] = n.type;
                o["description"] = n.description;
                const auto [lat, lon] = toLatLng(n.x, n.y);
                o["latitude"] = lat;
                o["longitude"] = lon;
                QJsonObject root{{"code", 0}, {"message", "ok"}, {"data", o}};
                return jsonResponse(QJsonDocument(root).toJson(QJsonDocument::Compact));
            }
        }
        return notFound();
    }

    if (method == "POST" && path == "/api/route") {
        const QJsonDocument doc = QJsonDocument::fromJson(body);
        const QJsonObject obj = doc.object();

        const int startId = obj["start"].toInt();
        const int endId = obj["end"].toInt();

        const QVector<int> pathNodes = Dijkstra::findPath(m_graph, startId, endId);

        if (pathNodes.empty()) {
            QJsonObject root{{"code", 1}, {"message", "no route found"}, {"data", QJsonObject()}};
            return jsonResponse(QJsonDocument(root).toJson(QJsonDocument::Compact));
        }

        QJsonArray pathArray;
        double totalDistance = 0.0;

        for (int i = 0; i < pathNodes.size(); ++i) {
            if (!m_graph.hasNode(pathNodes[i])) continue;

            const Node n = m_graph.node(pathNodes[i]);
            const auto [lat, lon] = toLatLng(n.x, n.y);
            QJsonObject point;
            point["id"] = n.id;
            point["name"] = n.name;
            point["latitude"] = lat;
            point["longitude"] = lon;
            pathArray.append(point);

            // 计算路径距离
            if (i > 0 && m_graph.hasNode(pathNodes[i - 1])) {
                const Node prev = m_graph.node(pathNodes[i - 1]);
                double dx = n.x - prev.x;
                double dy = n.y - prev.y;
                totalDistance += std::sqrt(dx * dx + dy * dy);
            }
        }

        QJsonObject data;
        data["path"] = pathArray;
        data["distance"] = totalDistance;
        data["time"] = totalDistance / 83.3; // 步行速度 5km/h = 83.3m/min

        QJsonObject root{{"code", 0}, {"message", "ok"}, {"data", data}};
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
