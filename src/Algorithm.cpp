#include "Algorithm.h"

#include <utility>
#include <vector>
#include <unordered_set>
#include <queue>
#include <algorithm>
#include <unordered_map>
#include <limits>

std::pair<int, std::vector<int>> computeComponents(const LGraph& graph) {
    auto vertices = graph.getAllVertexIds();
    std::unordered_set<std::string> visited;
    std::vector<int> componentSizes; // 各分量大小列表
    for (const auto& vertex : vertices) {
        if (visited.count(vertex) != 0) continue; // 已访问过，跳过
        int size = 0;
        // BFS 遍历当前连通分量
        std::queue<std::string> q;
        q.push(vertex);
        visited.insert(vertex);
        while (!q.empty()) {
            auto current = q.front(); q.pop();
            ++size; // 分量大小加 1
            for (const auto& edge : graph.getOpenAdjacent(current)) { // 只考虑 open 边
                const auto& neighbor = (edge.from_id == current) ? edge.to_id : edge.from_id;
                if (visited.count(neighbor) == 0) {
                    visited.insert(neighbor);
                    q.push(neighbor);
                }
            }
        }
        // 记录当前分量的大小
        componentSizes.push_back(size);
    }
    std::sort(componentSizes.begin(), componentSizes.end(), std::greater<>()); // 降序排序
    return std::make_pair(componentSizes.size(), componentSizes);
}

PathResult shortestPath(const LGraph& graph, const std::string& from, const std::string& to, 
    const std::string& mode) {
    PathResult result;
    if (!graph.placeExists(from) || !graph.placeExists(to)) {
        result.reachable = false;
        return result; // 起点或终点不存在
    }

    using PQElement = std::pair<int, std::string>; // <累计成本, 顶点 ID>
    const int INF = std::numeric_limits<int>::max(); // 无穷大

    // Dijkstra 算法
    std::priority_queue<PQElement, std::vector<PQElement>, std::greater<>> pq;
    std::unordered_map<std::string, int> dist; // 顶点 ID → 累计成本
    std::unordered_map<std::string, std::string> prev; // 顶点 ID → 前驱顶点 ID
    for (const auto& vertex : graph.getAllVertexIds()) {
        dist[vertex] = INF; // 初始化为无穷大
    }
    dist[from] = 0;
    pq.push({0, from});
    
    while (!pq.empty()) {
        auto [cost, current] = pq.top(); pq.pop();
        if (current == to) break; // 找到目标，提前退出
        if (cost > dist[current]) continue; // 已经有更优路径了，跳过
        for (const auto& edge : graph.getOpenAdjacent(current)) { // 只考虑 open 边
            const auto& neighbor = (edge.from_id == current) ? edge.to_id : edge.from_id;
            int edgeCost = (mode == "DIST") ? edge.distance : edge.walk_time;
            int newCost = cost + edgeCost;
            if (newCost < dist[neighbor]) {
                dist[neighbor] = newCost;
                prev[neighbor] = current;
                pq.push({newCost, neighbor});
            }
        }
    }
    
    if (dist[to] == INF) {
        result.reachable = false; // 无法到达
    } else {
        result.reachable = true;
        result.totalCost = dist[to];
        // 反向构建路径
        for (std::string at = to; at != from; at = prev[at]) {
            result.nodes.push_back(at);
        }
        result.nodes.push_back(from);
        std::reverse(result.nodes.begin(), result.nodes.end()); // 正序输出
    }
    return result;
}