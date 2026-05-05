#include "databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
}

DatabaseManager::~DatabaseManager()
{
    close();
}

bool DatabaseManager::open(const QString &dbPath)
{
    m_dbPath = dbPath;
    m_db = QSqlDatabase::addDatabase("QSQLITE", "campus_nav");
    m_db.setDatabaseName(dbPath);
    if (!m_db.open()) {
        qWarning() << "Failed to open database:" << m_db.lastError().text();
        return false;
    }
    return true;
}

void DatabaseManager::close()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool DatabaseManager::isOpen() const
{
    return m_db.isOpen();
}

QSqlDatabase DatabaseManager::database() const
{
    return m_db;
}

bool DatabaseManager::initTables()
{
    QSqlQuery q(m_db);

    auto exec = [&](const QString &sql) -> bool {
        if (!q.exec(sql)) {
            qWarning() << "SQL error:" << q.lastError().text() << "\nSQL:" << sql;
            return false;
        }
        return true;
    };

    bool ok = true;
    ok &= exec("CREATE TABLE IF NOT EXISTS buildings ("
               "id INTEGER PRIMARY KEY, name TEXT, type TEXT, "
               "center_x REAL, center_y REAL, polygon TEXT, description TEXT, aliases TEXT)");

    ok &= exec("CREATE TABLE IF NOT EXISTS roads ("
               "id INTEGER PRIMARY KEY, name TEXT, type TEXT, "
               "node_ids TEXT, points TEXT, weight_factor REAL DEFAULT 1.0)");

    ok &= exec("CREATE TABLE IF NOT EXISTS nodes ("
               "id INTEGER PRIMARY KEY, x REAL, y REAL, "
               "building_id INTEGER DEFAULT -1, is_entrance INTEGER DEFAULT 0)");

    ok &= exec("CREATE TABLE IF NOT EXISTS edges ("
               "id INTEGER PRIMARY KEY, from_node INTEGER, to_node INTEGER, "
               "road_id INTEGER, distance REAL)");

    ok &= exec("CREATE TABLE IF NOT EXISTS users ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT UNIQUE, "
               "password_hash TEXT, created_at TEXT)");

    ok &= exec("CREATE TABLE IF NOT EXISTS favorites ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, user_id INTEGER, building_id INTEGER)");

    ok &= exec("CREATE TABLE IF NOT EXISTS route_history ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, user_id INTEGER, "
               "from_node INTEGER, to_node INTEGER, route_json TEXT, timestamp TEXT)");

    return ok;
}