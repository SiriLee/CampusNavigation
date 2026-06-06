#include "MainWindow.h"
#include "GraphWidget.h"
#include "LGraph.h"

#include <QShowEvent>

MainWindow::MainWindow(LGraph* graph, QWidget *parent)
    : QMainWindow(parent)
    , m_graph(graph)
    , m_graphWidget(nullptr)
{
    setWindowTitle("Campus Navigation - GUI");
    resize(800, 600);

    m_graphWidget = new GraphWidget(m_graph, this);
    setCentralWidget(m_graphWidget);
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    if (m_graphWidget) {
        m_graphWidget->setGraph(m_graph);
    }
}
