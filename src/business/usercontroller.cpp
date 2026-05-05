#include "usercontroller.h"
#include "userdao.h"
#include <QCryptographicHash>
#include <QDebug>

UserController::UserController(QObject *parent)
    : QObject(parent)
{
}

void UserController::setUserDAO(UserDAO *dao)
{
    m_dao = dao;
}

bool UserController::login(const QString &username, const QString &password)
{
    if (!m_dao) {
        emit loginFailed("DAO not initialized");
        return false;
    }

    UserInfo user = m_dao->findUser(username);
    if (user.id == 0) {
        emit loginFailed("User not found");
        return false;
    }

    QString hash = hashPassword(password);
    if (!m_dao->verifyPassword(user.id, hash)) {
        emit loginFailed("Wrong password");
        return false;
    }

    m_currentUserId = user.id;
    m_currentUsername = username;
    emit loginSuccess(username);
    return true;
}

bool UserController::registerUser(const QString &username, const QString &password)
{
    if (!m_dao) {
        emit registerFailed("DAO not initialized");
        return false;
    }

    UserInfo existing = m_dao->findUser(username);
    if (existing.id != 0) {
        emit registerFailed("Username already exists");
        return false;
    }

    QString hash = hashPassword(password);
    if (!m_dao->createUser(username, hash)) {
        emit registerFailed("Failed to create user");
        return false;
    }

    emit registerSuccess(username);
    return true;
}

void UserController::logout()
{
    m_currentUserId = -1;
    m_currentUsername.clear();
}

int UserController::currentUserId() const { return m_currentUserId; }
QString UserController::currentUsername() const { return m_currentUsername; }
bool UserController::isLoggedIn() const { return m_currentUserId > 0; }

QVector<int> UserController::favoriteBuildingIds()
{
    QVector<int> ids;
    if (!m_dao || !isLoggedIn()) return ids;

    auto records = m_dao->getFavorites(m_currentUserId);
    for (const auto &r : records) {
        ids.append(r.buildingId);
    }
    return ids;
}

bool UserController::addFavorite(int buildingId)
{
    if (!m_dao || !isLoggedIn()) return false;
    bool ok = m_dao->addFavorite(m_currentUserId, buildingId);
    if (ok) emit favoritesChanged();
    return ok;
}

bool UserController::removeFavorite(int buildingId)
{
    if (!m_dao || !isLoggedIn()) return false;
    bool ok = m_dao->removeFavorite(m_currentUserId, buildingId);
    if (ok) emit favoritesChanged();
    return ok;
}

bool UserController::saveRouteHistory(int fromNode, int toNode, const QString &routeJson)
{
    if (!m_dao || !isLoggedIn()) return false;
    return m_dao->addRouteHistory(m_currentUserId, fromNode, toNode, routeJson);
}

QVector<QString> UserController::loadRouteHistory()
{
    QVector<QString> history;
    if (!m_dao || !isLoggedIn()) return history;

    auto records = m_dao->getRouteHistory(m_currentUserId);
    for (const auto &r : records) {
        history.append(r.routeJson);
    }
    return history;
}

QVector<RouteHistoryRecord> UserController::routeHistoryRecords()
{
    if (!m_dao || !isLoggedIn()) return {};
    return m_dao->getRouteHistory(m_currentUserId);
}

QString UserController::hashPassword(const QString &password)
{
    return QCryptographicHash::hash(password.toUtf8(),
                                     QCryptographicHash::Sha256).toHex();
}