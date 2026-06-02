#include "LGraph.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

std::string LGraph::makeEdgeKey(const std::string& from, const std::string& to) const {
    return (from < to) ? (from + "|" + to) : (to + "|" + from);
}

Road LGraph::normalizeEdge(const Road& road) const {
    if (road.from_id <= road.to_id) {
        return road; // 已经是标准化的
    } else {
        return Road{road.to_id, road.from_id, road.distance, road.walk_time, road.status};
    }
}

bool LGraph::addPlace(const Place& place) {
    if (placeExists(place.place_id)) {
        return false; // 地点已存在
    }
    places_[place.place_id] = place;
    adj_[place.place_id] = {}; // 初始化邻接列表
    return true;
}

bool LGraph::deletePlace(const std::string& id) {
    if (!placeExists(id)) {
        return false; // 地点不存在
    }
    // !删除关联边
    for (const auto& road : adj_[id]) {
        std::string other_id = (road.from_id == id) ? road.to_id : road.from_id;
        adj_[other_id].erase(
            std::remove_if(adj_[other_id].begin(), adj_[other_id].end(),
                [&](const Road& r) { return (r.from_id == id || r.to_id == id); }),
            adj_[other_id].end());
        edge_keys_.erase(makeEdgeKey(id, other_id));
    }
    adj_.erase(id); // 删除邻接列表
    places_.erase(id); // 删除地点
    return true;
}

bool LGraph::updatePlace(const Place& place, const std::string& field, const std::string& value) {
    auto it = places_.find(place.place_id);
    if (it == places_.end()) {
        return false; // 地点不存在
    }
    Place& p = it->second;
    if (field == "display_name") {
        p.display_name = value;
    } else if (field == "category") {
        p.category = value;
    } else if (field == "stay_time") {
        p.stay_time = std::stoi(value);
    } else if (field == "open_time") {
        p.open_time = value;
    } else if (field == "close_time") {
        p.close_time = value;
    } else {
        return false; // 无效字段
    }
    return true;
}

const Place* LGraph::getPlace(const std::string& id) const {
    auto it = places_.find(id);
    return (it != places_.end()) ? &(it->second) : nullptr;
}

bool LGraph::placeExists(const std::string& id) const {
    return places_.count(id) > 0;
}

bool LGraph::addRoad(const Road& road) {
    Road norm_road = normalizeEdge(road);
    std::string edge_key = makeEdgeKey(norm_road.from_id, norm_road.to_id);
    if (edge_keys_.count(edge_key) > 0) {
        return false; // 道路已存在
    }
    if (!placeExists(norm_road.from_id) || !placeExists(norm_road.to_id)) {
        return false; // 起点或终点不存在
    }
    adj_[norm_road.from_id].push_back(norm_road);
    adj_[norm_road.to_id].push_back(norm_road);
    edge_keys_.insert(edge_key);
    return true;
}

bool LGraph::deleteRoad(const std::string& from, const std::string& to) {
    std::string edge_key = makeEdgeKey(from, to);
    if (edge_keys_.count(edge_key) == 0) {
        return false; // 道路不存在
    }
    adj_[from].erase(
        std::remove_if(adj_[from].begin(), adj_[from].end(),
            [&](const Road& r) { return (r.from_id == from && r.to_id == to) || (r.from_id == to && r.to_id == from); }),
        adj_[from].end());
    adj_[to].erase(
        std::remove_if(adj_[to].begin(), adj_[to].end(),
            [&](const Road& r) { return (r.from_id == from && r.to_id == to) || (r.from_id == to && r.to_id == from); }),
        adj_[to].end());
    edge_keys_.erase(edge_key);
    return true;
}

bool LGraph::updateRoad(const std::string& from, const std::string& to, const std::string& field, const std::string& value) {
    std::string edge_key = makeEdgeKey(from, to);
    if (edge_keys_.count(edge_key) == 0) {
        return false; // 道路不存在
    }
    for (Road& road : adj_[from]) {
        if ((road.from_id == from && road.to_id == to) || (road.from_id == to && road.to_id == from)) {
            if (field == "distance") {
                road.distance = std::stoi(value);
            } else if (field == "walk_time") {
                road.walk_time = std::stoi(value);
            } else if (field == "status") {
                road.status = value;
            } else {
                return false; // 无效字段
            }
        }
    }
    for (Road& road : adj_[to]) {
        if ((road.from_id == from && road.to_id == to) || (road.from_id == to && road.to_id == from)) {
            if (field == "distance") {
                road.distance = std::stoi(value);
            } else if (field == "walk_time") {
                road.walk_time = std::stoi(value);
            } else if (field == "status") {
                road.status = value;
            }
        }
    }
    return true;
}

bool LGraph::closeRoad(const std::string& from, const std::string& to) {
    return updateRoad(from, to, "status", "closed");
}

bool LGraph::openRoad(const std::string& from, const std::string& to) {
    return updateRoad(from, to, "status", "open");
}

bool LGraph::roadExists(const std::string& from, const std::string& to) const {
    std::string edge_key = makeEdgeKey(from, to);
    return edge_keys_.count(edge_key) > 0;
}

std::vector<Road> LGraph::getAdjacent(const std::string& id) const {
    if (!placeExists(id)) {
        return {}; // 地点不存在
    }
    return adj_.at(id);
}

std::vector<Place> LGraph::getAllPlaces() const {
    std::vector<Place> result;
    for (const auto& [id, place] : places_) {
        result.push_back(place);
    }
    return result;
}

std::vector<Road> LGraph::getAllOpenEdges() const {
    std::vector<Road> result;
    std::unordered_set<std::string> seen_edges; // 避免重复添加同一条道路
    for (const auto& [from_id, roads] : adj_) {
        for (const Road& road : roads) {
            if (road.status == "open") {
                std::string edge_key = makeEdgeKey(road.from_id, road.to_id);
                if (seen_edges.count(edge_key) == 0) { // 去重
                    seen_edges.insert(edge_key);
                    result.push_back(road);
                }
            }
        }
    }
    return result;
}

std::vector<std::string> LGraph::getAllVertexIds() const {
    std::vector<std::string> result;
    for (const auto& [id, place] : places_) {
        result.push_back(id);
    }
    return result;
}

std::vector<Road> LGraph::getOpenAdjacent(const std::string& id) const {
    std::vector<Road> result;
    if (!placeExists(id)) {
        return result; // 地点不存在
    }
    for (const Road& road : adj_.at(id)) {
        if (road.status == "open") {
            result.push_back(road);
        }
    }
    return result;
}

void LGraph::clear() {
    places_.clear();
    adj_.clear();
    edge_keys_.clear();
}