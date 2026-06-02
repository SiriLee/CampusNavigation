#ifndef ALGORITHM_H
#define ALGORITHM_H

#include "LGraph.h"
#include <utility>
#include <vector>

// 连通分量分析，返回<分量个数, 各分量大小列表（已降序）>
std::pair<int, std::vector<int>> computeComponents(const LGraph& graph);

// 最短路径算法
struct PathResult {
    bool reachable; // 是否可达
    int totalCost; // 总距离/耗时
    std::vector<std::string> nodes; // 路径上的地点 ID 列表
};
PathResult shortestPath(const LGraph& graph, const std::string& from, const std::string& to, 
    const std::string& mode); // mode: "DIST" 或 "TIME"

#endif // ALGORITHM_H