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
            o["latitude"] = 34.3849 + (n.y / 100000.0);
            o["longitude"] = 108.9863 + (n.x / 100000.0);
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
            o["latitude"] = 34.3849 + (n.y / 100000.0);
            o["longitude"] = 108.9863 + (n.x / 100000.0);
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
