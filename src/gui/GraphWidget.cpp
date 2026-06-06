#include "GraphWidget.h"
#include "ForceLayout.h"
#include "LGraph.h"

#include <QPainter>
#include <QPaintEvent>
#include <QPen>

// ── 绘制常量 ──
static constexpr double kNODE_RADIUS    = 18.0;
static constexpr double kPADDING        = 50.0;
static constexpr int    kFONT_SIZE      = 9;
static const QColor     kNODE_FILL("#E0E0E0");
static const QColor     kCRITICAL_COLOR(128, 0, 128);   // 紫色

// ── 坐标变换 ──
struct ViewTransform {
    double scaleX, scaleY, offsetX, offsetY;
    QPointF map(double x, double y) const {
        return QPointF(offsetX + x * scaleX, offsetY + y * scaleY);
    }
};

GraphWidget::GraphWidget(LGraph* graph, QWidget *parent)
    : QWidget(parent)
    , m_graph(graph)
{
}

void GraphWidget::setGraph(LGraph* graph)
{
    m_graph = graph;
    clearHighlights();
    computeLayout();
    update();
}

void GraphWidget::setPathNodes(const QVector<QString>& nodes)
{
    m_pathNodes = nodes;
    update();
}

void GraphWidget::setCritical(const QVector<QString>& nodes,
                              const QVector<QPair<QString, QString>>& edges)
{
    m_criticalNodes.clear();
    for (const auto& n : nodes)
        m_criticalNodes.insert(n);

    m_criticalEdges.clear();
    for (const auto& e : edges) {
        // 标准化：from <= to
        if (e.first <= e.second)
            m_criticalEdges.insert(e);
        else
            m_criticalEdges.insert({e.second, e.first});
    }
    update();
}

void GraphWidget::clearHighlights()
{
    m_pathNodes.clear();
    m_criticalNodes.clear();
    m_criticalEdges.clear();
}

void GraphWidget::computeLayout()
{
    if (!m_graph) return;
    m_positions = ForceLayout::compute(*m_graph, width(), height());
}

// ── 收集所有边（去重，区分 open / closed） ──
static void collectEdges(const LGraph& graph,
                         std::vector<Road>& openEdges,
                         std::vector<Road>& closedEdges)
{
    QHash<QString, Road> seen;

    for (const auto& id : graph.getAllVertexIds()) {
        for (const auto& r : graph.getAdjacent(id)) {
            std::string key = (r.from_id <= r.to_id)
                ? (r.from_id + "|" + r.to_id)
                : (r.to_id + "|" + r.from_id);
            auto qkey = QString::fromStdString(key);
            if (seen.contains(qkey)) continue;
            seen[qkey] = r;
        }
    }

    for (auto it = seen.begin(); it != seen.end(); ++it) {
        if (it->status == "open")
            openEdges.push_back(it.value());
        else
            closedEdges.push_back(it.value());
    }
}

// ── 标准化边键 ──
static inline QPair<QString, QString> makeKey(const QString& a, const QString& b)
{
    return (a <= b) ? qMakePair(a, b) : qMakePair(b, a);
}

// ── 绘制 ──
void GraphWidget::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (!m_graph) return;

    // 无坐标时自动计算布局
    if (m_positions.empty()) {
        computeLayout();
        if (m_positions.empty()) return;
    }

    // ── 坐标范围 ──
    double minX = 1e9, minY = 1e9, maxX = -1e9, maxY = -1e9;
    for (auto it = m_positions.begin(); it != m_positions.end(); ++it) {
        minX = std::min(minX, it->x());
        minY = std::min(minY, it->y());
        maxX = std::max(maxX, it->x());
        maxY = std::max(maxY, it->y());
    }
    const double dataW = maxX - minX;
    const double dataH = maxY - minY;
    if (dataW < 1.0 || dataH < 1.0) return;

    const double availW = width()  - 2.0 * kPADDING;
    const double availH = height() - 2.0 * kPADDING;
    ViewTransform tr;
    tr.scaleX  = availW / dataW;
    tr.scaleY  = availH / dataH;
    tr.offsetX = kPADDING - minX * tr.scaleX;
    tr.offsetY = kPADDING - minY * tr.scaleY;

    // ── 构建路径边集合 ──
    QSet<QPair<QString, QString>> pathEdges;
    for (int i = 0; i + 1 < m_pathNodes.size(); ++i)
        pathEdges.insert(makeKey(m_pathNodes[i], m_pathNodes[i + 1]));

    // ── 收集边 ──
    std::vector<Road> openEdges, closedEdges;
    collectEdges(*m_graph, openEdges, closedEdges);

    // 辅助：判断边高亮类型并返回对应画笔
    auto edgePen = [&](const QString& a, const QString& b, bool isOpen) -> QPen {
        auto key = makeKey(a, b);
        if (pathEdges.contains(key))
            return QPen(Qt::green, 4, Qt::SolidLine);            // 路径边
        if (m_criticalEdges.contains(key))
            return QPen(kCRITICAL_COLOR, 3, Qt::SolidLine);      // 关键边
        if (isOpen)
            return QPen(Qt::black, 2, Qt::SolidLine);            // 普通 open
        return QPen(Qt::black, 2, Qt::DashLine);                   // 普通 closed
    };

    // ── 1. 绘制 open 边 ──
    for (const auto& e : openEdges) {
        auto a = QString::fromStdString(e.from_id);
        auto b = QString::fromStdString(e.to_id);
        painter.setPen(edgePen(a, b, true));
        QPointF p1 = tr.map(m_positions.value(a).x(), m_positions.value(a).y());
        QPointF p2 = tr.map(m_positions.value(b).x(), m_positions.value(b).y());
        painter.drawLine(p1, p2);
    }

    // ── 2. 绘制 closed 边 ──
    for (const auto& e : closedEdges) {
        auto a = QString::fromStdString(e.from_id);
        auto b = QString::fromStdString(e.to_id);
        painter.setPen(edgePen(a, b, false));
        QPointF p1 = tr.map(m_positions.value(a).x(), m_positions.value(a).y());
        QPointF p2 = tr.map(m_positions.value(b).x(), m_positions.value(b).y());
        painter.drawLine(p1, p2);
    }

    // ── 3. 绘制节点 ──
    QFont font = painter.font();
    font.setPointSize(kFONT_SIZE);
    painter.setFont(font);

    for (auto it = m_positions.begin(); it != m_positions.end(); ++it) {
        QPointF center = tr.map(it->x(), it->y());

        // 填充
        painter.setBrush(kNODE_FILL);
        // 边框：关键节点红色加粗
        if (m_criticalNodes.contains(it.key()))
            painter.setPen(QPen(Qt::red, 2.5));
        else
            painter.setPen(QPen(Qt::black, 1.5));

        painter.drawEllipse(center, kNODE_RADIUS, kNODE_RADIUS);

        // 标签
        painter.setPen(Qt::black);
        painter.setBrush(Qt::NoBrush);
        QRectF textRect(center.x() - kNODE_RADIUS * 2,
                        center.y() - kNODE_RADIUS,
                        kNODE_RADIUS * 4,
                        kNODE_RADIUS * 2);
        painter.drawText(textRect, Qt::AlignCenter, it.key());
    }
}
