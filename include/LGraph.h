#ifndef LGRAPH_H
#define LGRAPH_H

#include "GraphTypes.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

class LGraph {
public:
    // 地点操作
    bool addPlace(const Place& place); // 添加地点
    bool deletePlace(const std::string& id); // 删除地点（同时删除关联边）
    bool updatePlace(const Place& place, 
        const std::string& field, const std::string& value); // 更新地点信息（字段-值）
    const Place* getPlace(const std::string& id) const; // 获取地点信息
    bool placeExists(const std::string& id) const; // 检查地点是否存在

    // 道路操作
    bool addRoad(const Road& road); // 添加道路（无向，需添加两条有向边）
    bool deleteRoad(const std::string& from, const std::string& to); // 删除道路
    bool updateRoad(const std::string& from, const std::string& to, 
        const std::string& field, const std::string& value); // 更新道路信息
    bool closeRoad(const std::string& from, const std::string& to); // 关闭道路
    bool openRoad(const std::string& from, const std::string& to); // 开放道路
    bool roadExists(const std::string& from, const std::string& to) const; // 检查道路是否存在

    // 查询操作
    std::vector<Road> getAdjacent(const std::string& id) const; // 获取相邻道路
    std::vector<Place> getAllPlaces() const; // 用于按类别查询
    std::vector<Road> getAllOpenEdges() const; // 获取所有 open 边
    std::vector<std::string> getAllVertexIds() const; // 获取所有顶点 ID
    std::vector<Road> getOpenAdjacent(const std::string& id) const; // 获取某个顶点的所有 open 邻居

    // 清理
    void clear(); // 清空所有数据，用于 LOAD 命令

    // 辅助函数
    Road normalizeEdge(const Road& road) const; // 标准化道路（from_id <= to_id）
private:
    std::unordered_map<std::string, Place> places_; // id → Place
    std::unordered_map<std::string, std::vector<Road>> adj_; // from_id → 边列表
    std::unordered_set<std::string> edge_keys_; // 标准化键，用于判重

    // 辅助函数
    std::string makeEdgeKey(const std::string& from, const std::string& to) const; // min(id) + "|" + max(id)
};

#endif // LGRAPH_H