#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QHash>
#include <QPair>
#include <QPointF>
#include <QSet>
#include <QString>
#include <QVector>
#include <QWidget>

class LGraph;

/// 图可视化组件 — 手动绘制节点、边、路径高亮、关键节点/边
class GraphWidget : public QWidget {
    Q_OBJECT

public:
    explicit GraphWidget(LGraph* graph, QWidget *parent = nullptr);
    ~GraphWidget() override = default;

    /// 切换图数据，重新计算布局并重绘（清除高亮）
    void setGraph(LGraph* graph);

    /// 设置最短路径高亮
    void setPathNodes(const QVector<QString>& nodes);

    /// 设置关键节点和关键边高亮
    void setCritical(const QVector<QString>& nodes,
                     const QVector<QPair<QString, QString>>& edges);

    /// 清除所有高亮
    void clearHighlights();

    /// 调用 ForceLayout 计算所有顶点坐标
    void computeLayout();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    LGraph* m_graph;
    QHash<QString, QPointF> m_positions;

    // ── 高亮数据 ──
    QVector<QString> m_pathNodes;                       // 最短路径节点序列
    QSet<QPair<QString, QString>> m_criticalEdges;      // 关键边（标准化键）
    QSet<QString> m_criticalNodes;                      // 关键节点
};

#endif // GRAPHWIDGET_H
