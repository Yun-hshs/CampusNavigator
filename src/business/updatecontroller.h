#ifndef UPDATECONTROLLER_H
#define UPDATECONTROLLER_H

#include <QObject>
#include <QString>

class NetworkManager;
class MapController;
class DatabaseManager;

class UpdateController : public QObject {
    Q_OBJECT
public:
    explicit UpdateController(QObject *parent = nullptr);

    void setNetworkManager(NetworkManager *nm);
    void setMapController(MapController *mc);
    void setDatabaseManager(DatabaseManager *dm);

    void checkForUpdates(const QString &serverUrl, const QString &localVersion);
    void applyUpdate(const QString &downloadUrl);

signals:
    void updateAvailable(const QString &version, const QString &url);
    void updateCompleted();
    void updateFailed(const QString &msg);

private:
    NetworkManager *m_networkManager = nullptr;
    MapController *m_mapController = nullptr;
    DatabaseManager *m_dbManager = nullptr;
    QString m_lastUpdateUrl;
};

#endif // UPDATECONTROLLER_H