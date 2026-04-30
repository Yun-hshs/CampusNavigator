#pragma once
#include <QObject>
#include <QVector>
#include "src/algorithm/dijkstra.h"

class MapScene;

class PathHighlighter : public QObject {
    Q_OBJECT
public:
    explicit PathHighlighter(MapScene* scene, QObject* parent = nullptr);

    void showPath(const Dijkstra::Result& result);
    void clearPath();

    const QVector<int>& currentPath() const { return m_currentPath; }

private:
    MapScene* m_scene;
    QVector<int> m_currentPath;
};
