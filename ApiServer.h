#pragma once

#include <QObject>
#include <QTcpServer>
#include "graph/Graph.h"

class QTcpSocket;

class ApiServer : public QObject {
    Q_OBJECT
public:
    explicit ApiServer(QObject *parent = nullptr);
    bool start(quint16 port = 8080);

private slots:
    void onNewConnection();

private:
    void handleSocket(QTcpSocket *socket);
    QByteArray handleRequest(const QString &method, const QString &pathWithQuery, const QByteArray &body = QByteArray()) const;
    QByteArray jsonResponse(const QByteArray &json, const QByteArray &status = "200 OK") const;
    QByteArray notFound() const;
    QByteArray badRequest(const QString &msg) const;

    Graph m_graph;
    QTcpServer m_server;
};
