#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QHash>
#include <QPointF>
#include <QString>
#include <QWidget>

class LGraph;

/// 图可视化组件 — 手动绘制节点、边、路径高亮
class GraphWidget : public QWidget {
    Q_OBJECT

public:
    explicit GraphWidget(LGraph* graph, QWidget *parent = nullptr);
    ~GraphWidget() override = default;

    /// 切换图数据，重新计算布局并重绘
    void setGraph(LGraph* graph);

    /// 调用 ForceLayout 计算所有顶点坐标
    void computeLayout();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    LGraph* m_graph;
    QHash<QString, QPointF> m_positions;
};

#endif // GRAPHWIDGET_H
