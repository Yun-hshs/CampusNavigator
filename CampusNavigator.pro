QT       += core gui sql network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += \
    src/algorithm \
    src/business \
    src/data \
    src/models \
    src/ui

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    src/algorithm/astar.cpp \
    src/algorithm/dijkstra.cpp \
    src/algorithm/graph.cpp \
    src/algorithm/spatialindex.cpp \
    src/business/mapcontroller.cpp \
    src/business/routecontroller.cpp \
    src/business/searchcontroller.cpp \
    src/business/updatecontroller.cpp \
    src/business/usercontroller.cpp \
    src/data/databasemanager.cpp \
    src/data/jsonparser.cpp \
    src/data/networkmanager.cpp \
    src/data/userdao.cpp \
    src/models/building.cpp \
    src/models/road.cpp \
    src/ui/logindialog.cpp \
    src/ui/calibrationpanel.cpp \
    src/ui/mapwidget.cpp \
    src/ui/routepanel.cpp \
    src/ui/routedialog.cpp \
    src/ui/routehistorydialog.cpp \
    src/ui/searchbar.cpp

HEADERS += \
    mainwindow.h \
    src/algorithm/astar.h \
    src/algorithm/dijkstra.h \
    src/algorithm/graph.h \
    src/algorithm/spatialindex.h \
    src/business/mapcontroller.h \
    src/business/routecontroller.h \
    src/business/searchcontroller.h \
    src/business/updatecontroller.h \
    src/business/usercontroller.h \
    src/data/databasemanager.h \
    src/data/jsonparser.h \
    src/data/networkmanager.h \
    src/data/userdao.h \
    src/models/building.h \
    src/models/environment.h \
    src/models/node.h \
    src/models/road.h \
    src/models/route.h \
    src/ui/logindialog.h \
    src/ui/calibrationpanel.h \
    src/ui/maplayer.h \
    src/ui/mapwidget.h \
    src/ui/routepanel.h \
    src/ui/routedialog.h \
    src/ui/routehistorydialog.h \
    src/ui/searchbar.h

FORMS += \
    mainwindow.ui

# TRANSLATIONS += \
#     CampusNavigator_zh_CN.ts
# CONFIG += lrelease
# CONFIG += embed_translations
# RESOURCES += \
#     resources.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
