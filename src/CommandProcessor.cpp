#include "CommandProcessor.h"
#include "CsvIO.h"
#include "Algorithm.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <tuple>
#include <functional>
#include <cctype>

CommandProcessor::CommandProcessor(LGraph& graph) : graph_(graph) {
    initHandlers();
}

void CommandProcessor::initHandlers() {
    handlers_["LOAD"] = [this](const std::vector<std::string>& args) { cmdLoad(args); };
    handlers_["SAVE"] = [this](const std::vector<std::string>& args) { cmdSave(args); };
    handlers_["ADD_PLACE"] = [this](const std::vector<std::string>& args) { cmdAddPlace(args); };
    handlers_["DELETE_PLACE"] = [this](const std::vector<std::string>& args) { cmdDeletePlace(args); };
    handlers_["UPDATE_PLACE"] = [this](const std::vector<std::string>& args) { cmdUpdatePlace(args); };
    handlers_["ADD_ROAD"] = [this](const std::vector<std::string>& args) { cmdAddRoad(args); };
    handlers_["DELETE_ROAD"] = [this](const std::vector<std::string>& args) { cmdDeleteRoad(args); };
    handlers_["UPDATE_ROAD"] = [this](const std::vector<std::string>& args) { cmdUpdateRoad(args); };
    handlers_["CLOSE_ROAD"] = [this](const std::vector<std::string>& args) { cmdCloseRoad(args); };
    handlers_["OPEN_ROAD"] = [this](const std::vector<std::string>& args) { cmdOpenRoad(args); };
    handlers_["QUERY_PLACE"] = [this](const std::vector<std::string>& args) { cmdQueryPlace(args); };
    handlers_["QUERY_CATEGORY"] = [this](const std::vector<std::string>& args) { cmdQueryCategory(args); };
    handlers_["ADJ"] = [this](const std::vector<std::string>& args) { cmdADJ(args); };
    handlers_["COMPONENTS"] = [this](const std::vector<std::string>& args) { cmdComponents(args); };
    handlers_["SHORTEST"] = [this](const std::vector<std::string>& args) { cmdShortest(args); };
    handlers_["TIMED_SHORTEST"] = [this](const std::vector<std::string>& args) { cmdTimedShortest(args); };
    handlers_["MUST_PASS"] = [this](const std::vector<std::string>& args) { cmdMustPass(args); };
    handlers_["MST"] = [this](const std::vector<std::string>& args) { cmdMST(args); };
    handlers_["CRITICAL"] = [this](const std::vector<std::string>& args) { cmdCritical(args); };
}

void CommandProcessor::run() {
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue; // 跳过空行
        if (line[0] == '#') continue; // 跳过注释行
        auto tokens = splitLine(line);
        if (tokens.empty()) continue; // 跳过没有命令的行
        // 获取 cmd 和 args
        const std::string& cmd = tokens[0];
        std::vector<std::string> args(tokens.begin() + 1, tokens.end());
        // 查找并执行对应的命令处理函数
        if (cmd == "QUIT") { 
            break; // 退出循环
        }
        auto it = handlers_.find(cmd);
        if (it != handlers_.end()) {
            it->second(args);
        } else {
            std::cout << "ERROR unknown_command" << std::endl;
        }
    }
}

void CommandProcessor::cmdLoad(const std::vector<std::string>& args) {
    if (!checkArgCount(args, 2)) {
        std::cout << "ERROR invalid_arguments" << std::endl;
        return;
    }
    const std::string& placesFile = args[0];
    const std::string& roadsFile = args[1];
    graph_.clear(); // 先清空现有数据
    std::string errorMsg;
    if (!loadPlaces(graph_, placesFile, errorMsg)) {
        std::cout << "ERROR " << errorMsg << std::endl;
        return;
    }
    if (!loadRoads(graph_, roadsFile, errorMsg)) {
        std::cout << "ERROR " << errorMsg << std::endl;
        return;
    }
    std::cout << "OK" << std::endl;
}

void CommandProcessor::cmdSave(const std::vector<std::string>& args) {
    if (!checkArgCount(args, 2)) {
        std::cout << "ERROR invalid_arguments" << std::endl;
        return;
    }
    const std::string& placesFile = args[0];
    const std::string& roadsFile = args[1];
    std::string errorMsg;
    if (!savePlaces(graph_, placesFile, errorMsg)) {
        std::cout << "ERROR " << errorMsg << std::endl;
        return;
    }
    if (!saveRoads(graph_, roadsFile, errorMsg)) {
        std::cout << "ERROR " << errorMsg << std::endl;
        return;
    }
    std::cout << "OK" << std::endl;
}

void CommandProcessor::cmdAddPlace(const std::vector<std::string>& args) {
    if (!checkArgCount(args, 6)) {
        std::cout << "ERROR invalid_arguments" << std::endl;
        return;
    }
    Place place{
        args[0], // place_id
        args[1], // display_name
        args[2], // category
        std::stoi(args[3]), // stay_time
        args[4], // open_time
        args[5]  // close_time
    };
    if (!graph_.addPlace(place)) {
        std::cout << "ERROR duplicate_place_id" << std::endl;
        return;
    }
    std::cout << "OK" << std::endl;
}

void CommandProcessor::cmdDeletePlace(const std::vector<std::string>& args) {
    if (!checkArgCount(args, 1)) {
        std::cout << "ERROR invalid_arguments" << std::endl;
        return;
    }
    const std::string& place_id = args[0];
    if (!graph_.deletePlace(place_id)) {
        std::cout << "ERROR place_not_found" << std::endl;
        return;
    }
    std::cout << "OK" << std::endl;
}

void CommandProcessor::cmdUpdatePlace(const std::vector<std::string>& args) {
    if (!checkArgCount(args, 3)) {
        std::cout << "ERROR invalid_arguments" << std::endl;
        return;
    }
    const std::string& place_id = args[0];
    const std::string& field = args[1];
    const std::string& value = args[2];
    const Place* place = graph_.getPlace(place_id);
    if (!place) {
        std::cout << "ERROR place_not_found" << std::endl;
        return;
    }
    if (!graph_.updatePlace(*place, field, value)) {
        std::cout << "ERROR invalid_field_or_value" << std::endl;
        return;
    }
    std::cout << "OK" << std::endl;
}

void CommandProcessor::cmdAddRoad(const std::vector<std::string>& args) {
    if (!checkArgCount(args, 5)) {
        std::cout << "ERROR invalid_arguments" << std::endl;
        return;
    }
    Road road{
        args[0], // from_id
        args[1], // to_id
        std::stoi(args[2]), // distance
        std::stoi(args[3]), // walk_time
        args[4]  // status
    };
    if (!graph_.addRoad(road)) {
        std::cout << "ERROR duplicate_road_or_place_not_found" << std::endl;
        return;
    }
    std::cout << "OK" << std::endl;
}

void CommandProcessor::cmdDeleteRoad(const std::vector<std::string>& args) {
    if (!checkArgCount(args, 2)) {
        std::cout << "ERROR invalid_arguments" << std::endl;
        return;
    }
    const std::string& from_id = args[0];
    const std::string& to_id = args[1];
    if (!graph_.deleteRoad(from_id, to_id)) {
        std::cout << "ERROR road_not_found" << std::endl;
        return;
    }
    std::cout << "OK" << std::endl;
}

void CommandProcessor::cmdUpdateRoad(const std::vector<std::string>& args) {
    if (!checkArgCount(args, 4)) {
        std::cout << "ERROR invalid_arguments" << std::endl;
        return;
    }
    const std::string& from_id = args[0];
    const std::string& to_id = args[1];
    const std::string& field = args[2];
    const std::string& value = args[3];
    if (!graph_.updateRoad(from_id, to_id, field, value)) {
        std::cout << "ERROR invalid_field_or_value" << std::endl;
        return;
    }
    std::cout << "OK" << std::endl;
}

void CommandProcessor::cmdCloseRoad(const std::vector<std::string>& args) {
    if (!checkArgCount(args, 2)) {
        std::cout << "ERROR invalid_arguments" << std::endl;
        return;
    }
    const std::string& from_id = args[0];
    const std::string& to_id = args[1];
    if (!graph_.closeRoad(from_id, to_id)) {
        std::cout << "ERROR road_not_found" << std::endl;
        return;
    }
    std::cout << "OK" << std::endl;
}

void CommandProcessor::cmdOpenRoad(const std::vector<std::string>& args) {
    if (!checkArgCount(args, 2)) {
        std::cout << "ERROR invalid_arguments" << std::endl;
        return;
    }
    const std::string& from_id = args[0];
    const std::string& to_id = args[1];
    if (!graph_.openRoad(from_id, to_id)) {
        std::cout << "ERROR road_not_found" << std::endl;
        return;
    }
    std::cout << "OK" << std::endl;
}

void CommandProcessor::cmdQueryPlace(const std::vector<std::string>& args) {
    if (!checkArgCount(args, 1)) {
        std::cout << "ERROR invalid_arguments" << std::endl;
        return;
    }
    const std::string& place_id = args[0];
    const Place* place = graph_.getPlace(place_id);
    if (!place) {
        std::cout << "ERROR place_not_found" << std::endl;
        return;
    }
    std::cout << "PLACE " << place->place_id << " " << place->display_name << " " 
              << place->category << " " << place->stay_time << " "
              << place->open_time << " " << place->close_time << std::endl;
}

void CommandProcessor::cmdQueryCategory(const std::vector<std::string>& args) {
    if (!checkArgCount(args, 1)) {
        std::cout << "ERROR invalid_arguments" << std::endl;
        return;
    }
    const std::string& category = args[0];
    auto places = graph_.getAllPlaces();
    std::vector<Place> filtered;
    for (const auto& place : places) {
        if (place.category == category) {
            filtered.push_back(place);
        }
    }
    // place_id 升序排序
    std::sort(filtered.begin(), filtered.end(), [](const Place& a, const Place& b) {
        return a.place_id < b.place_id;
    });
    for (const auto& place : filtered) {
        std::cout << "PLACE " << place.place_id << " " << place.display_name << " " 
                  << place.category << " " << place.stay_time << " "
                  << place.open_time << " " << place.close_time << std::endl;
    }
}

void CommandProcessor::cmdADJ(const std::vector<std::string>& args) {
    if (!checkArgCount(args, 1)) {
        std::cout << "ERROR invalid_arguments" << std::endl;
        return;
    }
    const std::string& place_id = args[0];
    auto roads = graph_.getAdjacent(place_id);
    // <neighbor_id>:<distance>:<walk_time>:<status>
    std::vector<std::tuple<std::string, int, int, std::string>> neighbors;
    for (const auto& road : roads) {
        std::string neighbor_id = (road.from_id == place_id) ? road.to_id : road.from_id;
        neighbors.emplace_back(neighbor_id, road.distance, road.walk_time, road.status);
    }
    // neighbor_id 升序排序
    std::sort(neighbors.begin(), neighbors.end(), [](const auto& a, const auto& b) {
        return std::get<0>(a) < std::get<0>(b);
    });
    std::cout << "ADJ " << place_id << " " << neighbors.size();
    for (const auto& neighbor : neighbors) {
        std::cout << " " << std::get<0>(neighbor) << ":" << std::get<1>(neighbor) 
                  << ":" << std::get<2>(neighbor) << ":" << std::get<3>(neighbor);
    }
    std::cout << std::endl;
}

void CommandProcessor::cmdComponents(const std::vector<std::string>& args) {
    if (!checkArgCount(args, 0)) {
        std::cout << "ERROR invalid_arguments" << std::endl;
        return;
    }
    auto [count, sizes] = computeComponents(graph_);
    std::cout << "COMPONENTS " << count << " SIZES";
    for (int size : sizes) {
        std::cout << " " << size;
    }
    std::cout << std::endl;
}

void CommandProcessor::cmdShortest(const std::vector<std::string>& args) {
    // SHORTEST <from_id> <to_id> <DIST|TIME>
    if (!checkArgCount(args, 3)) {
        std::cout << "ERROR invalid_arguments" << std::endl;
        return;
    }
    const std::string& from = args[0];
    const std::string& to = args[1];
    const std::string& mode = args[2];
    if (!graph_.placeExists(from) || !graph_.placeExists(to)) {
        std::cout << "ERROR place_not_found" << std::endl;
        return;
    }
    if (mode != "DIST" && mode != "TIME") {
        std::cout << "ERROR invalid_mode" << std::endl;
        return;
    }
    
    auto result = dijkstra(graph_, from, to, 
        [&mode](const Road& road) {
            return (mode == "DIST") ? road.distance : road.walk_time;
        }
    );
    if (!result.reachable) {
        std::cout << "NO_PATH" << std::endl;
    } else {
        std::cout << "PATH " << mode << " " << result.totalCost << " NODES";
        for (const auto& node : result.nodes) {
            std::cout << " " << node;
        }
        std::cout << std::endl;
    }
}

void CommandProcessor::cmdTimedShortest(const std::vector<std::string>& args) {
    // TIMED_SHORTEST <from_id> <to_id> <time> <DIST|TIME>
    if (!checkArgCount(args, 4)) {
        std::cout << "ERROR invalid_arguments" << std::endl;
        return;
    }
    const std::string& from = args[0];
    const std::string& to = args[1];
    const std::string& time_str = args[2]; // HH:MM
    const std::string& mode = args[3];
    if (!graph_.placeExists(from) || !graph_.placeExists(to)) {
        std::cout << "ERROR place_not_found" << std::endl;
        return; // 起点或终点不存在
    }
    if (mode != "DIST" && mode != "TIME") {
        std::cout << "ERROR invalid_mode" << std::endl;
        return; // 模式错误
    }
    if (time_str.size() != 5 || time_str[2] != ':' || 
        !std::isdigit(time_str[0]) || !std::isdigit(time_str[1]) || 
        !std::isdigit(time_str[3]) || !std::isdigit(time_str[4])) {
        std::cout << "ERROR invalid_time" << std::endl;
        return; // 时间格式错误
    }

    // 使用通用 Dijkstra，增加时间过滤器
    auto result = dijkstra(graph_, from, to, 
        [&mode](const Road& road) {
            return (mode == "DIST") ? road.distance : road.walk_time;
        }, // 模式对应的权重函数
        makeTimeFilter(graph_, time_str) // 顶点过滤器：检查时间是否在开放时间内
    );

    if (!result.reachable) {
        std::cout << "NO_PATH" << std::endl;
    } else {
        std::cout << "PATH " << mode << " " << result.totalCost << " NODES";
        for (const auto& node : result.nodes) {
            std::cout << " " << node;
        }
        std::cout << std::endl;
    }
}

void CommandProcessor::cmdMustPass(const std::vector<std::string>& args) {
    // MUST_PASS <from> <to> <DIST|TIME> <k> <p1> <p2> ... <pk>
    if (args.size() < 5) {
        std::cout << "ERROR invalid_arguments" << std::endl;
        return;
    }
    const std::string& from = args[0];
    const std::string& to = args[1];
    const std::string& mode = args[2];
    int k = std::stoi(args[3]);
    if (args.size() != 4 + k) {
        std::cout << "ERROR invalid_arguments" << std::endl;
        return;
    }
    if (mode != "DIST" && mode != "TIME") {
        std::cout << "ERROR invalid_mode" << std::endl;
        return; // 模式错误
    }
    if (!graph_.placeExists(from) || !graph_.placeExists(to)) {
        std::cout << "ERROR place_not_found" << std::endl;
        return; // 起点或终点不存在
    }

    std::vector<std::string> mustPass(args.begin() + 4, args.end());
    for (const auto& p : mustPass) {
        if (!graph_.placeExists(p)) {
            std::cout << "ERROR place_not_found: " << p << std::endl;
            return; // 必经点不存在
        }
    }

    auto result = mustPassPath(graph_, from, to, mode, mustPass);
    if (!result.reachable) {
        std::cout << "NO_PATH" << std::endl;
    } else {
        std::cout << "PATH " << mode << " " << result.totalCost << " NODES";
        for (const auto& node : result.nodes) {
            std::cout << " " << node;
        }
        std::cout << std::endl;
    }
}

void CommandProcessor::cmdMST(const std::vector<std::string>& args) {
    if (!checkArgCount(args, 0)) {
        std::cout << "ERROR invalid_arguments" << std::endl;
        return;
    }
    auto result = computeMST(graph_);
    if (!result.connected) {
        std::cout << "DISCONNECTED" << std::endl;
        return;
    }
    auto& edges = result.edges; // 避免不必要的复制
    // (min_id, max_id)
    for (auto& edge : edges) {
        if (edge.from_id > edge.to_id) {
            std::swap(edge.from_id, edge.to_id);
        }
    }
    // 按 (from_id, to_id) 升序排序
    std::sort(edges.begin(), edges.end(), [](const Road& a, const Road& b) {
        if (a.from_id != b.from_id) return a.from_id < b.from_id;
        return a.to_id < b.to_id;
    });
    // MST <totalDistance> EDGES <u1>-<v1>:<w1> <u2>-<v2>:<w2> ...
    std::cout << "MST " << result.totalDistance << " EDGES";
    for (const auto& edge : edges) {
        std::cout << " " << edge.from_id << "-" << edge.to_id << ":" << edge.distance;
    }
    std::cout << std::endl;
}

void CommandProcessor::cmdCritical(const std::vector<std::string>& args) {
    if (!checkArgCount(args, 0)) {
        std::cout << "ERROR invalid_arguments" << std::endl;
        return;
    }
    auto result = computeCritical(graph_);
    auto& nodes = result.nodes; // 割点
    auto& edges = result.edges; // 桥
    // CRITICAL NODES <node_count> <id1> <id2> ... EDGES <edge_count> <u1>-<v1> <u2>-<v2> ...
    std::cout << "CRITICAL NODES " << nodes.size();
    for (const auto& node : nodes) {
        std::cout << " " << node;
    }
    std::cout << " EDGES " << edges.size();
    for (const auto& edge : edges) {
        std::cout << " " << edge.first << "-" << edge.second;
    }
    std::cout << std::endl;
}

std::vector<std::string> CommandProcessor::splitLine(const std::string& line) {
    std::vector<std::string> tokens;
    std::istringstream iss(line);
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

bool CommandProcessor::checkArgCount(const std::vector<std::string>& args, size_t expected) {
    return args.size() == expected;
}