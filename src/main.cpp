#include <QApplication>
#include "mainwindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("CampusNavigator");
    app.setApplicationDisplayName("校园导航系统");

    MainWindow w;
    w.show();

    return app.exec();
}
