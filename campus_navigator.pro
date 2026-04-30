QT += core gui widgets network

TARGET   = CampusNavigator
TEMPLATE = app

CONFIG += c++17

INCLUDEPATH += $$PWD

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/algorithm/graph.cpp \
    src/algorithm/dijkstra.cpp \
    src/data/dataloader.cpp \
    src/business/pathhighlighter.cpp \
    src/ui/mapscene.cpp \
    src/ui/osmmapview.cpp \
    src/ui/buildingitem.cpp \
    src/ui/roaditem.cpp

HEADERS += \
    src/mainwindow.h \
    src/models/building.h \
    src/models/road.h \
    src/algorithm/graph.h \
    src/algorithm/dijkstra.h \
    src/data/dataloader.h \
    src/business/pathhighlighter.h \
    src/ui/mapscene.h \
    src/ui/osmmapview.h \
    src/ui/buildingitem.h \
    src/ui/roaditem.h

FORMS += \
    mainwindow.ui

# Copy data directory to output so the app finds campus_map.json at runtime
CONFIG(release, debug|release) {
    QMAKE_POST_LINK += $$QMAKE_COPY_DIR $$shell_path($$PWD/data) $$shell_path($$OUT_PWD/release/data)
} else {
    QMAKE_POST_LINK += $$QMAKE_COPY_DIR $$shell_path($$PWD/data) $$shell_path($$OUT_PWD/debug/data)
}
