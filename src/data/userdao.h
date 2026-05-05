#ifndef USERDAO_H
#define USERDAO_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QSqlDatabase>

struct UserInfo {
    int id;
    QString username;
    QString passwordHash;
};

struct FavoriteRecord {
    int id;
    int userId;
    int buildingId;
};

struct RouteHistoryRecord {
    int id;
    int userId;
    int fromNode;
    int toNode;
    QString routeJson;
    QString timestamp;
};

class UserDAO : public QObject {
    Q_OBJECT
public:
    explicit UserDAO(QSqlDatabase db, QObject *parent = nullptr);

    bool createUser(const QString &username, const QString &passwordHash);
    UserInfo findUser(const QString &username);
    bool verifyPassword(int userId, const QString &passwordHash);

    bool addFavorite(int userId, int buildingId);
    bool removeFavorite(int userId, int buildingId);
    QVector<FavoriteRecord> getFavorites(int userId);

    bool addRouteHistory(int userId, int fromNode, int toNode, const QString &routeJson);
    QVector<RouteHistoryRecord> getRouteHistory(int userId);

private:
    QSqlDatabase m_db;
};

#endif // USERDAO_H