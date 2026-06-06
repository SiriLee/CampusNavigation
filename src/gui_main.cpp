#include "MainWindow.h"
#include "LGraph.h"

#include <QApplication>

int runGui(int argc, char* argv[])
{
    QApplication app(argc, argv);
    LGraph graph;
    MainWindow window(&graph);
    window.show();
    return app.exec();
}
