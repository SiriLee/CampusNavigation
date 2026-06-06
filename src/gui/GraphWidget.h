#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QGraphicsView>

/// 图可视化组件
/// 后续将实现节点、边绘制、路径高亮等功能
class GraphWidget : public QGraphicsView {
    Q_OBJECT

public:
    explicit GraphWidget(QWidget *parent = nullptr);
    ~GraphWidget() override = default;
};

#endif // GRAPHWIDGET_H
