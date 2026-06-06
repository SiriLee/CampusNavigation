#include "GraphWidget.h"
#include "ForceLayout.h"
#include "LGraph.h"

#include <QPainter>
#include <QPaintEvent>
#include <QPen>

// ── 绘制常量 ──
static constexpr double kNODE_RADIUS    = 12.0;
static constexpr double kPADDING        = 50.0;
static constexpr int    kEDGE_WIDTH     = 2;
static constexpr int    kFONT_SIZE      = 9;
static const QColor     kNODE_FILL      = QColor("#E0E0E0");

// ── 坐标变换 ──
struct ViewTransform {
    double scaleX, scaleY, offsetX, offsetY;

    QPointF map(double x, double y) const {
        return QPointF(offsetX + x * scaleX,
                       offsetY + y * scaleY);
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
    computeLayout();
    update();
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
    QHash<QString, Road> seen;  // 去重键

    for (const auto& id : graph.getAllVertexIds()) {
        for (const auto& r : graph.getAdjacent(id)) {
            // 标准化键: from_id <= to_id
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

    // 计算坐标范围
    double minX = 1e9, minY = 1e9, maxX = -1e9, maxY = -1e9;
    for (auto it = m_positions.begin(); it != m_positions.end(); ++it) {
        minX = std::min(minX, it->x());
        minY = std::min(minY, it->y());
        maxX = std::max(maxX, it->x());
        maxY = std::max(maxY, it->y());
    }

    const double dataW = maxX - minX;
    const double dataH = maxY - minY;
    if (dataW < 1.0 || dataH < 1.0) return;  // 无效布局

    const double availW = width()  - 2.0 * kPADDING;
    const double availH = height() - 2.0 * kPADDING;

    ViewTransform tr;
    tr.scaleX  = availW / dataW;
    tr.scaleY  = availH / dataH;
    tr.offsetX = kPADDING - minX * tr.scaleX;
    tr.offsetY = kPADDING - minY * tr.scaleY;

    // ── 1. 收集边 ──
    std::vector<Road> openEdges, closedEdges;
    collectEdges(*m_graph, openEdges, closedEdges);

    // ── 2. 绘制 open 边（黑色实线） ──
    {
        QPen pen(Qt::black, kEDGE_WIDTH, Qt::SolidLine);
        painter.setPen(pen);
        for (const auto& e : openEdges) {
            auto fp = m_positions.value(QString::fromStdString(e.from_id));
            auto tp = m_positions.value(QString::fromStdString(e.to_id));
            QPointF a = tr.map(fp.x(), fp.y());
            QPointF b = tr.map(tp.x(), tp.y());
            painter.drawLine(a, b);
        }
    }

    // ── 3. 绘制 closed 边（红色虚线） ──
    {
        QPen pen(Qt::red, kEDGE_WIDTH, Qt::DashLine);
        painter.setPen(pen);
        for (const auto& e : closedEdges) {
            auto fp = m_positions.value(QString::fromStdString(e.from_id));
            auto tp = m_positions.value(QString::fromStdString(e.to_id));
            QPointF a = tr.map(fp.x(), fp.y());
            QPointF b = tr.map(tp.x(), tp.y());
            painter.drawLine(a, b);
        }
    }

    // ── 4. 绘制节点（圆形 + 标签） ──
    QFont font = painter.font();
    font.setPointSize(kFONT_SIZE);
    painter.setFont(font);

    for (auto it = m_positions.begin(); it != m_positions.end(); ++it) {
        QPointF center = tr.map(it->x(), it->y());

        // 圆形
        painter.setBrush(kNODE_FILL);
        painter.setPen(QPen(Qt::black, 1.5));
        painter.drawEllipse(center, kNODE_RADIUS, kNODE_RADIUS);

        // 文本
        painter.setPen(Qt::black);
        painter.setBrush(Qt::NoBrush);
        QRectF textRect(center.x() - kNODE_RADIUS * 2,
                        center.y() - kNODE_RADIUS,
                        kNODE_RADIUS * 4,
                        kNODE_RADIUS * 2);
        painter.drawText(textRect, Qt::AlignCenter, it.key());
    }
}
