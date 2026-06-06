#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class LGraph;
class GraphWidget;
class QLineEdit;
class QComboBox;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(LGraph* graph, QWidget *parent = nullptr);
    ~MainWindow() override = default;

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void onOpenCsv();
    void onCalcPath();
    void onAnalyzeCritical();
    void onResetView();

private:
    QWidget* createControlBar();

    LGraph* m_graph;
    GraphWidget* m_graphWidget;

    QLineEdit* m_fromEdit;
    QLineEdit* m_toEdit;
    QComboBox* m_modeCombo;
};

#endif // MAINWINDOW_H
