#ifndef USERCONTROLLER_H
#define USERCONTROLLER_H

#include <QObject>
#include <QString>
#include <QVector>
#include "userdao.h"

class UserDAO;

class UserController : public QObject {
    Q_OBJECT
public:
    explicit UserController(QObject *parent = nullptr);

    void setUserDAO(UserDAO *dao);

    bool login(const QString &username, const QString &password);
    bool registerUser(const QString &username, const QString &password);
    void logout();

    int currentUserId() const;
    QString currentUsername() const;
    bool isLoggedIn() const;

    QVector<int> favoriteBuildingIds();
    bool addFavorite(int buildingId);
    bool removeFavorite(int buildingId);

    bool saveRouteHistory(int fromNode, int toNode, const QString &routeJson);
    QVector<QString> loadRouteHistory();
    QVector<RouteHistoryRecord> routeHistoryRecords();

signals:
    void loginSuccess(const QString &username);
    void loginFailed(const QString &msg);
    void registerSuccess(const QString &username);
    void registerFailed(const QString &msg);
    void favoritesChanged();

private:
    QString hashPassword(const QString &password);

    UserDAO *m_dao = nullptr;
    int m_currentUserId = -1;
    QString m_currentUsername;
};

#endif // USERCONTROLLER_H