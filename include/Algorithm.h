#ifndef ALGORITHM_H
#define ALGORITHM_H

#include "LGraph.h"
#include <utility>
#include <vector>
#include <functional>
#include <unordered_map>

using WeightFunc = std::function<int(const Road&)>; // 权重获取器类型
using VertexFilter = std::function<bool(const std::string&)>; // 顶点过滤器类型

// 并查集，用于最小生成树Kruskal算法
class DSU {
public:
    void makeSet(const std::string& x);
    std::string find(const std::string& x);
    void unite(const std::string& x, const std::string& y);
private:
    std::unordered_map<std::string, std::string> parent_;
};

// 通用 Dijkstra 结果结构体
struct PathResult {
    bool reachable; // 是否可达
    int totalCost; // 总距离/耗时
    std::vector<std::string> nodes; // 路径上的地点 ID 列表
};

// 最小生成树结果结构体
struct MSTResult {
    bool connected; // 是否连通
    int totalDistance; // 总距离
    std::vector<Road> edges; // 最小生成树的边列表
};

// 关键节点和关键边
struct CriticalResult {
    std::vector<std::string> nodes; // 割点
    std::vector<std::pair<std::string, std::string>> edges; // 桥（存储为地点 ID 对）
};

// 共享单车券（分层图最短路径）结果结构体
struct KPathResult {
    bool reachable;
    int totalTime; // 总耗时（分钟）
    int usedK; // 实际用券数
    std::vector<std::string> nodes; // 节点序列
    std::vector<Road> fastEdges; // 用券的边列表（需标准化）
};

// 连通分量分析，返回<分量个数, 各分量大小列表（已降序）>
std::pair<int, std::vector<int>> computeComponents(const LGraph& graph);

// 通用 Dijkstra 算法（已排序）
PathResult dijkstra(const LGraph& graph, const std::string& from, const std::string& to, 
    WeightFunc weightFunc, VertexFilter vertexFilter = nullptr);

// 辅助时间过滤器函数
VertexFilter makeTimeFilter(const LGraph& graph, const std::string& time_str);

// 必经点路径规划（已排序）
PathResult mustPassPath(const LGraph& graph, const std::string& from, const std::string& to, 
    const std::string& mode, const std::vector<std::string>& mustPass); // mustPass: p1, p2, ..., pk

// 最小生成树算法（未排序）
MSTResult computeMST(const LGraph& graph);

// 关键节点和关键边分析（已排序）
CriticalResult computeCritical(const LGraph& graph);

// 分层图最短路径算法（已排序）
KPathResult shortestPathWithK(const LGraph& graph, const std::string& from, const std::string& to, int K);

#endif // ALGORITHM_H