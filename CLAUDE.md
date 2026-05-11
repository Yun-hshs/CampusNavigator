# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

CampusNavigator is a C++17 Qt 6 desktop application — a campus map with pathfinding, built for a Chinese university campus (UI is in Chinese: "校园导航系统"). It uses qmake as its build system and targets MinGW 64-bit on Windows.

## Build & Run

```bash
# Build via qmake (Makefiles are pre-generated for Qt 6.10.2 MinGW 64-bit Debug)
qmake CampusNavigator.pro
make            # or mingw32-make on Windows
make release    # release build
```

Alternatively, open `CampusNavigator.pro` in Qt Creator and use the IDE build/run buttons.

The executable is `CampusNavigator.exe`. A `QMAKE_POST_LINK` step in the `.pro` file copies `data/` to the output directory so `data/map.json` is available at runtime. There is no test suite.

## Architecture

### Active Source (compiled by `.pro`)

```
main.cpp                    — Entry point, i18n translator setup
mainwindow.cpp/.h/.ui       — Main window: toolbar, info panel, navigation orchestration
view/MapView                — QGraphicsView-based map widget, switches between isometric and flat 2D
view/RenderContext.h        — Shared struct (scene, graph, iso transform) passed to all renderers
view/LabelManager.h         — Building label collision detection, priority layout, LOD
view/RoadRenderer           — Bezier-curve roads with layered strokes (shadow, base, surface, highlight, dashes)
view/AreaRenderer           — Terrain polygons (plazas, sports fields, gardens, lakes)
view/PathVisualizer         — Shortest-path route drawing + marching-ants animation + glowing ball
graph/Graph                 — Data model: Node, Edge, Area structs; adjacency-list graph
algorithm/Dijkstra          — Min-heap Dijkstra shortest path (skips "decoration" nodes)
data/DataLoader             — Loads graph from JSON
data/GeoTransform.h         — Coordinate transform: logical meters ↔ screen pixels (isometric 2:1 or flat)
data/map.json               — Primary runtime map data (20 nodes, 28 edges, 6 areas)
```

### Legacy `src/` directory

`src/` contains an earlier MVC-style architecture (controllers, models, DAO, dialogs, A* algorithm). It is **not compiled** by the current `.pro` file — kept for reference only. Do not add files from `src/` to the build.

### Data Flow

1. `MainWindow::loadMapData()` → `DataLoader::loadFromJson("data/map.json", graph)` populates the `Graph`
2. `MapView::setGraph()` + `drawMap()` triggers the rendering pipeline on a `QGraphicsScene`
3. User selects start/end nodes (click or combo boxes) → `Dijkstra::findShortestPath()` → `PathVisualizer` draws the route

### Rendering Pipeline (layered, in `MapView::drawMap()`)

1. Ground plane (isometric mode only)
2. AreaRenderer — terrain polygons
3. RoadRenderer — roads as bezier curves with multi-layer strokes
4. Buildings — Y-sorted for depth: `IsometricBuilding` (sprites) or `VectorBuilding` (rectangles)
5. LabelManager — collision-avoiding labels with LOD
6. PathVisualizer — glow effects, marching ants, animated ball

### Coordinate System (`GeoTransform`)

- Origin: campus southwest corner; x = east, y = north; units = meters
- Isometric mode: 2:1 ratio projection; Flat mode: scaled + Y-flipped

### Navigation Modes

- Walk: 5 km/h (83.3 m/min); Bike: 15 km/h (250 m/min)
- Two-click flow: first click = start, second = end (synced with combo box dropdowns)

## Key Conventions

- All UI strings are in Chinese
- Qt resource files: `res.qrc` (sprite PNGs), `resources.qrc` (i18n `.qm` files)
- Sprite assets: `buildings/` (129 tiles), `roads_river/` (~100 tiles)
- The `.pro` file's `QMAKE_POST_LINK` must be updated if new data files are added to `data/`
