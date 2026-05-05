#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QString>

class NetworkManager : public QObject {
    Q_OBJECT
public:
    explicit NetworkManager(QObject *parent = nullptr);

    void fetchMapData(const QString &url);
    void checkUpdate(const QString &url, const QString &localVersion);

signals:
    void mapDataReceived(const QJsonObject &data);
    void updateAvailable(const QString &newVersion, const QString &downloadUrl);
    void noUpdateAvailable();
    void networkError(const QString &msg);

private:
    void onFetchFinished(QNetworkReply *reply);

    QNetworkAccessManager *m_manager;
};

#endif // NETWORKMANAGER_H