#pragma once
#include <QString>

struct Building {
    int id;
    QString name;
    double x = 0;
    double y = 0;
    double w = 55;
    double h = 35;
    QString type = "teaching";
    QString desc;
};
