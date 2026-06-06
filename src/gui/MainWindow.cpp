#include "MainWindow.h"
#include "GraphWidget.h"
#include "LGraph.h"
#include "Algorithm.h"
#include "CsvIO.h"

#include <QComboBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QShowEvent>
#include <QVBoxLayout>

MainWindow::MainWindow(LGraph* graph, QWidget *parent)
    : QMainWindow(parent)
    , m_graph(graph)
    , m_graphWidget(nullptr)
    , m_fromEdit(nullptr)
    , m_toEdit(nullptr)
    , m_modeCombo(nullptr)
{
    setWindowTitle("Campus Navigation - GUI");
    resize(900, 650);

    // ── 菜单栏 ──
    auto* fileMenu = menuBar()->addMenu(tr("&File"));
    auto* openAction = fileMenu->addAction(tr("Open CSV..."));
    connect(openAction, &QAction::triggered, this, &MainWindow::onOpenCsv);

    auto* viewMenu = menuBar()->addMenu(tr("&View"));
    auto* resetAction = viewMenu->addAction(tr("Reset Highlights"));
    connect(resetAction, &QAction::triggered, this, &MainWindow::onResetView);

    // ── 中央布局 ──
    auto* central = new QWidget(this);
    auto* mainLayout = new QVBoxLayout(central);

    mainLayout->addWidget(createControlBar());

    m_graphWidget = new GraphWidget(m_graph, central);
    mainLayout->addWidget(m_graphWidget, 1);  // stretch=1

    setCentralWidget(central);
}

QWidget* MainWindow::createControlBar()
{
    auto* bar = new QWidget(this);
    auto* layout = new QHBoxLayout(bar);
    layout->setContentsMargins(8, 4, 8, 4);

    layout->addWidget(new QLabel(tr("From:")));
    m_fromEdit = new QLineEdit();
    m_fromEdit->setMaximumWidth(100);
    layout->addWidget(m_fromEdit);

    layout->addWidget(new QLabel(tr("To:")));
    m_toEdit = new QLineEdit();
    m_toEdit->setMaximumWidth(100);
    layout->addWidget(m_toEdit);

    layout->addWidget(new QLabel(tr("Mode:")));
    m_modeCombo = new QComboBox();
    m_modeCombo->addItem("DIST");
    m_modeCombo->addItem("TIME");
    layout->addWidget(m_modeCombo);

    auto* calcBtn = new QPushButton(tr("Calculate Path"));
    connect(calcBtn, &QPushButton::clicked, this, &MainWindow::onCalcPath);
    layout->addWidget(calcBtn);

    auto* criticalBtn = new QPushButton(tr("Analyze Critical"));
    connect(criticalBtn, &QPushButton::clicked, this, &MainWindow::onAnalyzeCritical);
    layout->addWidget(criticalBtn);

    auto* resetBtn = new QPushButton(tr("Reset View"));
    connect(resetBtn, &QPushButton::clicked, this, &MainWindow::onResetView);
    layout->addWidget(resetBtn);

    layout->addStretch();
    return bar;
}

// ── 打开 CSV ──
void MainWindow::onOpenCsv()
{
    if (!m_graph) return;

    // 获取 places.csv 和 roads.csv
    QString placesFile = QFileDialog::getOpenFileName(
        this, tr("Open Places CSV"), QString(),
        tr("CSV Files (*.csv);;All Files (*)"));
    if (placesFile.isEmpty()) return;

    QString roadsFile = QFileDialog::getOpenFileName(
        this, tr("Open Roads CSV"), QString(),
        tr("CSV Files (*.csv);;All Files (*)"));
    if (roadsFile.isEmpty()) return;

    // 清空旧数据
    m_graph->clear();
    m_graphWidget->clearHighlights();

    // 加载
    std::string errMsg;
    if (!loadPlaces(*m_graph, placesFile.toStdString(), errMsg)) {
        QMessageBox::warning(this, tr("Error"),
            tr("Failed to load places: %1").arg(QString::fromStdString(errMsg)));
        return;
    }
    if (!loadRoads(*m_graph, roadsFile.toStdString(), errMsg)) {
        QMessageBox::warning(this, tr("Error"),
            tr("Failed to load roads: %1").arg(QString::fromStdString(errMsg)));
        return;
    }

    // 刷新图显示
    m_graphWidget->setGraph(m_graph);
}

// ── 计算最短路径 ──
void MainWindow::onCalcPath()
{
    if (!m_graph) return;

    QString from = m_fromEdit->text().trimmed();
    QString to   = m_toEdit->text().trimmed();
    if (from.isEmpty() || to.isEmpty()) {
        QMessageBox::information(this, tr("Info"),
            tr("Please enter both start and end place IDs."));
        return;
    }

    std::string fromId = from.toStdString();
    std::string toId   = to.toStdString();

    // 检查顶点是否存在
    if (!m_graph->placeExists(fromId)) {
        QMessageBox::warning(this, tr("Error"),
            tr("Place not found: %1").arg(from));
        return;
    }
    if (!m_graph->placeExists(toId)) {
        QMessageBox::warning(this, tr("Error"),
            tr("Place not found: %1").arg(to));
        return;
    }

    // 选择权重函数
    WeightFunc weightFn;
    if (m_modeCombo->currentText() == "TIME")
        weightFn = [](const Road& r) { return r.walk_time; };
    else
        weightFn = [](const Road& r) { return r.distance; };

    PathResult result = dijkstra(*m_graph, fromId, toId, weightFn);

    if (!result.reachable) {
        QMessageBox::information(this, tr("Result"),
            tr("No path from %1 to %2.").arg(from, to));
        m_graphWidget->setPathNodes({});
        return;
    }

    // 设置高亮路径
    QVector<QString> pathNodes;
    pathNodes.reserve(static_cast<int>(result.nodes.size()));
    for (const auto& n : result.nodes)
        pathNodes.append(QString::fromStdString(n));
    m_graphWidget->setPathNodes(pathNodes);
}

// ── 分析关键节点/边 ──
void MainWindow::onAnalyzeCritical()
{
    if (!m_graph) return;

    CriticalResult cr = computeCritical(*m_graph);

    QVector<QString> nodes;
    nodes.reserve(static_cast<int>(cr.nodes.size()));
    for (const auto& n : cr.nodes)
        nodes.append(QString::fromStdString(n));

    QVector<QPair<QString, QString>> edges;
    edges.reserve(static_cast<int>(cr.edges.size()));
    for (const auto& e : cr.edges)
        edges.append({QString::fromStdString(e.first),
                      QString::fromStdString(e.second)});

    m_graphWidget->setCritical(nodes, edges);
}

// ── 重置视图 ──
void MainWindow::onResetView()
{
    m_graphWidget->clearHighlights();
    m_graphWidget->update();
}

// ── 首次显示时触发布局 ──
void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    if (m_graphWidget) {
        m_graphWidget->setGraph(m_graph);
    }
}
