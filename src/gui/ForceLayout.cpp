#include "ForceLayout.h"
#include "LGraph.h"

#include <algorithm>
#include <cmath>
#include <QRandomGenerator>
#include <QVector>

QHash<QString, QPointF> ForceLayout::compute(const LGraph& graph,
                                              int width, int height)
{
    QHash<QString, QPointF> positions;

    const auto vertexIds = graph.getAllVertexIds();
    if (vertexIds.empty()) return positions;

    const int n = static_cast<int>(vertexIds.size());
    const double area = static_cast<double>(width) * height;
    const double k = std::sqrt(area / n);           // 理想节点间距
    const int iterations = 70;
    const double margin = 50.0;

    // ── 1. 随机初始化坐标 ──
    QRandomGenerator rng(42);  // 固定种子，每次布局结果一致
    QVector<QString> ids;
    ids.reserve(n);
    for (const auto& id : vertexIds) {
        auto qid = QString::fromStdString(id);
        ids.append(qid);
        positions[qid] = QPointF(
            margin + rng.bounded(width - 2.0 * margin),
            margin + rng.bounded(height - 2.0 * margin));
    }

    // ── 2. 构建邻接表（仅 open 边） ──
    const auto edges = graph.getAllOpenEdges();
    QHash<QString, QVector<QString>> adj;
    for (const auto& e : edges) {
        auto f = QString::fromStdString(e.from_id);
        auto t = QString::fromStdString(e.to_id);
        adj[f].append(t);
        adj[t].append(f);
    }

    // ── 3. 迭代 ──
    double t = width / 10.0;                     // 初始温度
    const double cooling = t / iterations;       // 每步降温

    for (int iter = 0; iter < iterations; ++iter) {
        // 位移累加器
        QHash<QString, QPointF> disp;

        // 3a. 排斥力 — 所有节点对之间
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                QPointF delta = positions[ids[i]] - positions[ids[j]];
                double dist = std::hypot(delta.x(), delta.y());
                if (dist < 1.0) dist = 1.0;

                double force = k * k / dist;
                QPointF d = delta / dist * force;
                disp[ids[i]] += d;
                disp[ids[j]] -= d;
            }
        }

        // 3b. 吸引力 — open 边端点之间
        for (const auto& e : edges) {
            auto f = QString::fromStdString(e.from_id);
            auto t = QString::fromStdString(e.to_id);

            QPointF delta = positions[f] - positions[t];
            double dist = std::hypot(delta.x(), delta.y());
            if (dist < 1.0) dist = 1.0;

            double force = dist * dist / k;
            QPointF d = delta / dist * force;
            disp[f] -= d;
            disp[t] += d;
        }

        // 3c. 应用位移（受温度限制）
        for (const auto& id : ids) {
            QPointF d = disp[id];
            double dist = std::hypot(d.x(), d.y());
            if (dist < 1.0) continue;

            // 位移不超过当前温度
            QPointF limited = d / dist * std::min(dist, t);
            double nx = std::clamp(positions[id].x() + limited.x(),
                                   margin, double(width) - margin);
            double ny = std::clamp(positions[id].y() + limited.y(),
                                   margin, double(height) - margin);
            positions[id] = QPointF(nx, ny);
        }

        t -= cooling;
    }

    return positions;
}
