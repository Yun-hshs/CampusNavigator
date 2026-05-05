#include "networkmanager.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QDebug>

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent), m_manager(new QNetworkAccessManager(this))
{
}

void NetworkManager::fetchMapData(const QString &url)
{
    QNetworkRequest request{QUrl(url)};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply *reply = m_manager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { onFetchFinished(reply); });
}

void NetworkManager::checkUpdate(const QString &url, const QString &localVersion)
{
    QNetworkRequest request{QUrl(url)};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply *reply = m_manager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, localVersion]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit networkError(reply->errorString());
            return;
        }
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        if (doc.isNull()) {
            emit networkError("Invalid JSON response");
            return;
        }
        QJsonObject obj = doc.object();
        QString remoteVersion = obj["version"].toString();
        if (remoteVersion > localVersion) {
            emit updateAvailable(remoteVersion, obj["download_url"].toString());
        } else {
            emit noUpdateAvailable();
        }
    });
}

void NetworkManager::onFetchFinished(QNetworkReply *reply)
{
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
        emit networkError(reply->errorString());
        return;
    }
    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    if (doc.isNull()) {
        emit networkError("Invalid JSON response");
        return;
    }
    emit mapDataReceived(doc.object());
}
