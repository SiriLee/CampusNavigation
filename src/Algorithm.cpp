#include "Algorithm.h"

#include <utility>
#include <vector>
#include <unordered_set>
#include <queue>
#include <algorithm>

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