#include "MainWindow.h"

#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Campus Navigation - GUI");
    resize(800, 600);

    // 空白中央区域
    setCentralWidget(new QWidget(this));
}
