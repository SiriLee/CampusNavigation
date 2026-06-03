#ifndef ALGORITHM_H
#define ALGORITHM_H

#include "LGraph.h"
#include <utility>
#include <vector>
#include <functional>

// 连通分量分析，返回<分量个数, 各分量大小列表（已降序）>
std::pair<int, std::vector<int>> computeComponents(const LGraph& graph);

// 通用 Dijkstra 结果结构体
struct PathResult {
    bool reachable; // 是否可达
    int totalCost; // 总距离/耗时
    std::vector<std::string> nodes; // 路径上的地点 ID 列表
};
// 权重获取器类型：接受 const Road&，返回 int
using WeightFunc = std::function<int(const Road&)>;
// 顶点过滤器类型：接受 const std::string& vertex_id，返回 bool（是否允许经过）
using VertexFilter = std::function<bool(const std::string&)>;
// 通用 Dijkstra 算法
PathResult dijkstra(const LGraph& graph, const std::string& from, const std::string& to, 
    WeightFunc weightFunc, VertexFilter vertexFilter = nullptr);

// 最短路径计算，mode: "DIST" 或 "TIME"，仅考虑 open 边
[[deprecated("Use dijkstra instead")]] 
PathResult shortestPath(const LGraph& graph, const std::string& from, const std::string& to, 
    const std::string& mode); // mode: "DIST" 或 "TIME"

#endif // ALGORITHM_H