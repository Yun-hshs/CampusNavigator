#include "pathhighlighter.h"
#include "src/ui/mapscene.h"

PathHighlighter::PathHighlighter(MapScene* scene, QObject* parent)
    : QObject(parent), m_scene(scene)
{
}

void PathHighlighter::showPath(const Dijkstra::Result& result) {
    if (!result.reachable) return;
    m_currentPath = result.path;
    m_scene->highlightPath(m_currentPath, result.pathPoints);
}

void PathHighlighter::clearPath() {
    m_currentPath.clear();
    m_scene->clearHighlight();
}
