#include "MainWindow.h"

#include <QApplication>

int runGui(int argc, char* argv[])
{
    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    return app.exec();
}
