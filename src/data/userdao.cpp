#include "userdao.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QDebug>

UserDAO::UserDAO(QSqlDatabase db, QObject *parent)
    : QObject(parent), m_db(db)
{
}

bool UserDAO::createUser(const QString &username, const QString &passwordHash)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO users (username, password_hash, created_at) VALUES (?, ?, ?)");
    q.addBindValue(username);
    q.addBindValue(passwordHash);
    q.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
    if (!q.exec()) {
        qWarning() << "Create user failed:" << q.lastError().text();
        return false;
    }
    return true;
}

UserInfo UserDAO::findUser(const QString &username)
{
    QSqlQuery q(m_db);
    q.prepare("SELECT id, username, password_hash FROM users WHERE username = ?");
    q.addBindValue(username);
    if (q.exec() && q.next()) {
        return {q.value(0).toInt(), q.value(1).toString(), q.value(2).toString()};
    }
    return {};
}

bool UserDAO::verifyPassword(int userId, const QString &passwordHash)
{
    QSqlQuery q(m_db);
    q.prepare("SELECT password_hash FROM users WHERE id = ?");
    q.addBindValue(userId);
    if (q.exec() && q.next()) {
        return q.value(0).toString() == passwordHash;
    }
    return false;
}

bool UserDAO::addFavorite(int userId, int buildingId)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO favorites (user_id, building_id) VALUES (?, ?)");
    q.addBindValue(userId);
    q.addBindValue(buildingId);
    return q.exec();
}

bool UserDAO::removeFavorite(int userId, int buildingId)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM favorites WHERE user_id = ? AND building_id = ?");
    q.addBindValue(userId);
    q.addBindValue(buildingId);
    return q.exec();
}

QVector<FavoriteRecord> UserDAO::getFavorites(int userId)
{
    QVector<FavoriteRecord> result;
    QSqlQuery q(m_db);
    q.prepare("SELECT id, user_id, building_id FROM favorites WHERE user_id = ?");
    q.addBindValue(userId);
    if (q.exec()) {
        while (q.next()) {
            result.append({q.value(0).toInt(), q.value(1).toInt(), q.value(2).toInt()});
        }
    }
    return result;
}

bool UserDAO::addRouteHistory(int userId, int fromNode, int toNode, const QString &routeJson)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO route_history (user_id, from_node, to_node, route_json, timestamp) "
              "VALUES (?, ?, ?, ?, ?)");
    q.addBindValue(userId);
    q.addBindValue(fromNode);
    q.addBindValue(toNode);
    q.addBindValue(routeJson);
    q.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
    return q.exec();
}

QVector<RouteHistoryRecord> UserDAO::getRouteHistory(int userId)
{
    QVector<RouteHistoryRecord> result;
    QSqlQuery q(m_db);
    q.prepare("SELECT id, user_id, from_node, to_node, route_json, timestamp "
              "FROM route_history WHERE user_id = ? ORDER BY timestamp DESC");
    q.addBindValue(userId);
    if (q.exec()) {
        while (q.next()) {
            result.append({q.value(0).toInt(), q.value(1).toInt(),
                           q.value(2).toInt(), q.value(3).toInt(),
                           q.value(4).toString(), q.value(5).toString()});
        }
    }
    return result;
}