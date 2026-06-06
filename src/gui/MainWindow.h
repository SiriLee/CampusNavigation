#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class LGraph;
class GraphWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(LGraph* graph, QWidget *parent = nullptr);
    ~MainWindow() override = default;

protected:
    void showEvent(QShowEvent *event) override;

private:
    LGraph* m_graph;
    GraphWidget* m_graphWidget;
};

#endif // MAINWINDOW_H
