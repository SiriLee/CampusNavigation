#include "Algorithm.h"

#include <string>
#include <utility>
#include <vector>
#include <unordered_set>
#include <queue>
#include <algorithm>
#include <unordered_map>
#include <limits>

void DSU::makeSet(const std::string& x) {
    if (parent_.count(x) == 0) {
        parent_[x] = x;
    }
}

std::string DSU::find(const std::string& x) {
    if (parent_[x] != x) {
        parent_[x] = find(parent_[x]); // 路径压缩
    }
    return parent_[x];
}

void DSU::unite(const std::string& x, const std::string& y) {
    parent_[find(x)] = find(y);
}

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

PathResult dijkstra(const LGraph& graph, const std::string& from, const std::string& to, 
    WeightFunc weightFunc, VertexFilter vertexFilter) {
    PathResult result;
    if (!graph.placeExists(from) || !graph.placeExists(to)) {
        result.reachable = false;
        return result; // 起点或终点不存在（实际在命令操作中判断）
    }
    if (vertexFilter && (!vertexFilter(from) || !vertexFilter(to))) {
        result.reachable = false;
        return result; // 起点或终点被过滤器禁止经过
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
        if (vertexFilter && !vertexFilter(current)) continue; // 顶点过滤器不允许经过，跳过
        for (const auto& edge : graph.getOpenAdjacent(current)) { // 只考虑 open 边
            const auto& neighbor = (edge.from_id == current) ? edge.to_id : edge.from_id;
            int edgeCost = weightFunc(edge);
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

VertexFilter makeTimeFilter(const LGraph& graph, const std::string& time_str) {
    return [graph, time_str](const std::string& vertex_id) {
        auto* place = graph.getPlace(vertex_id);
        return place && (place->open_time <= time_str && time_str <= place->close_time);
    };
}

PathResult mustPassPath(const LGraph& graph, const std::string& from, const std::string& to, 
    const std::string& mode, const std::vector<std::string>& mustPass) {
    // 构建完整的路径点列表：from -> mustPass... -> to
    std::vector<std::string> points = {from}; 
    points.reserve(2 + mustPass.size());
    points.insert(points.end(), mustPass.begin(), mustPass.end());
    points.push_back(to);

    PathResult finalResult;
    finalResult.reachable = true; // 初始假设可达
    finalResult.totalCost = 0;

    auto weightFunc = [&](const Road& road) -> int {
        return (mode == "DIST") ? road.distance : road.walk_time;
    };

    // 依次计算每段路径
    for (size_t i = 0; i < points.size() - 1; ++i) {
        auto segmentResult = dijkstra(graph, points[i], points[i + 1], weightFunc);
        if (!segmentResult.reachable) {
            finalResult.reachable = false; // 任一段不可达，则整体不可达
            return finalResult;
        }
        finalResult.totalCost += segmentResult.totalCost; // 累加总成本
        if (i == 0) {
            finalResult.nodes.insert(finalResult.nodes.end(), segmentResult.nodes.begin(), segmentResult.nodes.end());
        } else {
            // 后续段去掉起点（已在前一段末尾），避免重复
            finalResult.nodes.insert(finalResult.nodes.end(), segmentResult.nodes.begin() + 1, segmentResult.nodes.end());
        }
    }
    return finalResult;
}

MSTResult computeMST(const LGraph& graph) {
    MSTResult result;
    DSU dsu;
    std::vector<Road> edges = graph.getAllOpenEdges(); // 只考虑 open 边
    for (const auto& vertex : graph.getAllVertexIds()) {
        dsu.makeSet(vertex);
    }
    // 按 distance 升序排序边
    std::sort(edges.begin(), edges.end(), [](const Road& a, const Road& b) {
        return a.distance < b.distance;
    });
    
    result.totalDistance = 0;
    for (const auto& edge : edges) {
        if (dsu.find(edge.from_id) != dsu.find(edge.to_id)) {
            dsu.unite(edge.from_id, edge.to_id);
            result.edges.push_back(edge);
            result.totalDistance += edge.distance;
        }
    }
    
    // 检查连通性：MST边数 = 顶点数 - 1
    result.connected = (result.edges.size() == graph.getAllVertexIds().size() - 1);
    
    return result;
}

CriticalResult computeCritical(const LGraph& graph) {
    CriticalResult result;
    
    // 1. 获取顶点并映射为整数索引
    auto vertices = graph.getAllVertexIds();
    if (vertices.empty()) return result;
    int n = vertices.size();

    std::unordered_map<std::string, int> idToIndex;// ID -> 索引
    std::vector<std::string> indexToId(n); // 索引 -> ID
    for (int i = 0; i < n; ++i) {
        idToIndex[vertices[i]] = i;
        indexToId[i] = vertices[i];
    }

    // 2. 构建整数邻接表（只考虑 open 边）
    std::vector<std::vector<int>> adj(n); // 邻接表
    for (const auto& edge : graph.getAllOpenEdges()) {
        int u = idToIndex[edge.from_id];
        int v = idToIndex[edge.to_id];
        adj[u].push_back(v);
        adj[v].push_back(u);
    }

    // 3. Tarjan 初始化
    std::vector<int> disc(n, -1), low(n, -1), parent(n, -1);
    std::vector<bool> isArticulation(n, false);
    std::vector<std::pair<int, int>> bridges; // 存储桥的索引对
    int time = 0;

    // 4. dfs 递归函数定义
    std::function<void(int)> dfs = [&](int u) {
        // 1. 获取顶点并映射为整数索引
        disc[u] = low[u] = ++time;
        int children = 0;
        for (int v : adj[u]) {
            if (disc[v] == -1) { // v 未访问过
                parent[v] = u;
                ++children;
                dfs(v);
                low[u] = std::min(low[u], low[v]);
                // 判断割点
                if (parent[u] == -1 && children > 1) {
                    isArticulation[u] = true; // 根节点且有多个子树
                }
                if (parent[u] != -1 && low[v] >= disc[u]) {
                    isArticulation[u] = true; // 非根节点且没有后代通过 back edge 回到 u 或更早的祖先
                }
                // 判断桥
                if (low[v] > disc[u]) {
                    bridges.emplace_back(u, v);
                }
            } else if (v != parent[u]) { // back edge
                low[u] = std::min(low[u], disc[v]);
            }
        }
    };

    // 5.对每个连通分量运行 DFS
    for (int i = 0; i < n; ++i) {
        if (disc[i] == -1) {
            dfs(i);
        }
    }

    // 6. 收集关键节点（转为字符串 ID）
    for (int i = 0; i < n; ++i) {
        if (isArticulation[i]) {
            result.nodes.push_back(indexToId[i]);
        }
    }
    std::sort(result.nodes.begin(), result.nodes.end()); // 升序排序

    // 7. 收集关键边（标准化 from_id <= to_id）
    for (const auto& [u, v] : bridges) {
        auto left = indexToId[u];
        auto right = indexToId[v];
        if (left > right) {
            std::swap(left, right);
        }
        result.edges.emplace_back(left, right);
    }
    std::sort(result.edges.begin(), result.edges.end()); // 升序排序

    return result;
}

KPathResult shortestPathWithK(const LGraph& graph, const std::string& from, const std::string& to, int K) {
    using State = std::pair<std::string, int>; // (place_id, used)
    auto hashState = [](const State& s) {
        return std::hash<std::string>()(s.first) ^ std::hash<int>()(s.second);
    };
    // 距离数组 (place_id, used) -> totalTime
    std::unordered_map<State, int, decltype(hashState)> dist(0);
    // 前驱记录 (place_id, used) -> ((prev_place_id, prev_used), usedK)
    std::unordered_map<State, std::pair<State, bool>, decltype(hashState)> prev(0);
    
    // Dijkstra with state (place_id, used)
    using PQElement = std::pair<int, State>; // (totalTime, (place_id, used))
    std::priority_queue<PQElement, std::vector<PQElement>, std::greater<>> pq;
    dist[{from, 0}] = 0;
    pq.push({0, {from, 0}});
    while (!pq.empty()) {
        auto [time, state] = pq.top();
        pq.pop();
        const auto& [place_id, used] = state;
        if (time > dist[state]) continue;
        if (place_id == to) {
            break; // 找到目标，提前退出
        }
        // Explore neighbors
        for (const auto& edge : graph.getAdjacent(place_id)) {
            // 1. 不使用券
            if (edge.status == "open") {
                int newTime = time + edge.walk_time;
                State newState = { (edge.from_id == place_id) ? edge.to_id : edge.from_id, used };
                if (dist.count(newState) == 0 || newTime < dist[newState]) {
                    dist[newState] = newTime;
                    prev[newState] = { state, false }; // 没有使用券
                    pq.push({newTime, newState});
                }
            }
            // 2. 使用券（如果还有券可用）
            if (used < K) {
                int newTime = time + edge.walk_time / 3; // 使用券后时间缩短
                State newState = { (edge.from_id == place_id) ? edge.to_id : edge.from_id, used + 1 };
                if (dist.count(newState) == 0 || newTime < dist[newState]) {
                    dist[newState] = newTime;
                    prev[newState] = { state, true }; // 使用了券
                    pq.push({newTime, newState});
                }
            }
        }
    }

    KPathResult result;
    // 所有层均未达到to
    if (dist.count({to, 0}) == 0 && dist.count({to, 1}) == 0 && dist.count({to, 2}) == 0) {
        result.reachable = false;
        return result;
    }

    // 最优used层
    State bestState = {to, 0};
    for (int used = 0; used <= K; ++used) {
        State candidate = {to, used};
        if (dist.count(candidate) > 0 && (dist.count(bestState) == 0 || dist[candidate] < dist[bestState])) {
            bestState = candidate;
        }
    }

    // bestState -> (from, 0) 回溯路径
    std::vector<std::string> path;
    std::vector<Road> fastEdges;
    for (State s = bestState; prev.count(s) > 0; s = prev[s].first) {
        path.push_back(s.first);
        if (prev[s].second) { // 使用了券
            const auto& [prevState, _] = prev[s];
            std::string from = prevState.first;
            std::string to = s.first;
            // 获取标准化的边
            for (const auto& edge : graph.getAdjacent(from)) {
                if ((edge.from_id == from && edge.to_id == to) || (edge.from_id == to && edge.to_id == from)) {
                    fastEdges.push_back(graph.normalizeEdge(edge));
                    break;
                }
            }
        }
    }

    // 构建结果
    result.reachable = true;
    result.totalTime = dist[bestState];
    result.usedK = bestState.second;

    // 节点序列反转
    std::reverse(path.begin(), path.end());
    result.nodes = std::move(path);
    // 字典序排序 fastEdges
    std::sort(fastEdges.begin(), fastEdges.end(), [](const Road& a, const Road& b) {
        if (a.from_id != b.from_id) return a.from_id < b.from_id;
        return a.to_id < b.to_id;
    });
    result.fastEdges = std::move(fastEdges);
    
    return result;
}





