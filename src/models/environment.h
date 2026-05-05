#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <QString>
#include <QPolygonF>

enum class EnvironmentType {
    Water,
    Grass
};

struct EnvironmentArea {
    int id = 0;
    QString name;
    EnvironmentType type = EnvironmentType::Grass;
    QPolygonF polygon;

    static EnvironmentType typeFromString(const QString &s) {
        if (s == "water") return EnvironmentType::Water;
        return EnvironmentType::Grass;
    }
};

#endif // ENVIRONMENT_H
