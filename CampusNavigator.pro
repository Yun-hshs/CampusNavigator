QT += core gui widgets

TARGET   = CampusNavigator
TEMPLATE = app

CONFIG += c++17

INCLUDEPATH += $$PWD

SOURCES += \
    main.cpp \
    MainWindow.cpp \
    view/MapView.cpp \
    graph/Graph.cpp \
    algorithm/Dijkstra.cpp \
    data/DataLoader.cpp

HEADERS += \
    MainWindow.h \
    view/MapView.h \
    view/LabelManager.h \
    graph/Graph.h \
    algorithm/Dijkstra.h \
    data/DataLoader.h \
    data/GeoTransform.h

# Copy data directory to output so the app finds map.json at runtime
CONFIG(release, debug|release) {
    QMAKE_POST_LINK += $$QMAKE_COPY_DIR $$shell_path($$PWD/data) $$shell_path($$OUT_PWD/release/data)
} else {
    QMAKE_POST_LINK += $$QMAKE_COPY_DIR $$shell_path($$PWD/data) $$shell_path($$OUT_PWD/debug/data)
}

RESOURCES += \
    res.qrc
