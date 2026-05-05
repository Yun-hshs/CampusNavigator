#include "updatecontroller.h"
#include "networkmanager.h"
#include "mapcontroller.h"
#include "databasemanager.h"
#include "jsonparser.h"
#include <QJsonObject>
#include <QDebug>

UpdateController::UpdateController(QObject *parent)
    : QObject(parent)
{
}

void UpdateController::setNetworkManager(NetworkManager *nm) { m_networkManager = nm; }
void UpdateController::setMapController(MapController *mc) { m_mapController = mc; }
void UpdateController::setDatabaseManager(DatabaseManager *dm) { m_dbManager = dm; }

void UpdateController::checkForUpdates(const QString &serverUrl, const QString &localVersion)
{
    if (!m_networkManager) {
        emit updateFailed("NetworkManager not set");
        return;
    }

    connect(m_networkManager, &NetworkManager::updateAvailable,
            this, [this](const QString &version, const QString &url) {
        m_lastUpdateUrl = url;
        emit updateAvailable(version, url);
    });
    connect(m_networkManager, &NetworkManager::noUpdateAvailable,
            this, [this]() { /* no signal needed, already up to date */ });
    connect(m_networkManager, &NetworkManager::networkError,
            this, [this](const QString &msg) { emit updateFailed(msg); });

    m_networkManager->checkUpdate(serverUrl, localVersion);
}

void UpdateController::applyUpdate(const QString &downloadUrl)
{
    if (!m_networkManager || !m_mapController) {
        emit updateFailed("Missing dependencies for update");
        return;
    }

    // Fetch new map data
    connect(m_networkManager, &NetworkManager::mapDataReceived,
            this, [this](const QJsonObject &data) {
        // Load into map controller
        JsonParser parser;
        if (!parser.parseMapData(data)) {
            emit updateFailed("Failed to parse updated map data");
            return;
        }

        // Save to database
        if (m_dbManager && m_dbManager->isOpen()) {
            m_mapController->saveToDatabase(m_dbManager);
        }

        emit updateCompleted();
    });

    m_networkManager->fetchMapData(downloadUrl);
}