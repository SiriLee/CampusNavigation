#ifndef FORCELAYOUT_H
#define FORCELAYOUT_H

#include <QHash>
#include <QPointF>
#include <QString>

class LGraph;

/// Fruchterman-Reingold 力导向布局算法
class ForceLayout {
public:
    /// 计算节点坐标
    /// @param graph  图数据（仅使用 open 边）
    /// @param width  布局区域宽度
    /// @param height 布局区域高度
    /// @return place_id → 坐标 (QPointF)
    static QHash<QString, QPointF> compute(const LGraph& graph,
                                           int width, int height);
};

#endif // FORCELAYOUT_H
