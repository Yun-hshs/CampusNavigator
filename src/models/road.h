#pragma once
#include <QString>
#include <QVector>
#include <QPointF>

struct Road {
    QString name;
    QString type = "sub";        // main / sub / path
    QVector<QPointF> path;       // pixel coordinates
};
