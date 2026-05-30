#include "GraphTypes.h"
#include "LGraph.h"
#include "CsvIO.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <fstream>

// 通用的 CSV 行解析函数
std::vector<std::string> splitCSVLine(const std::string& line) {
    std::vector<std::string> result;
    std::istringstream ss(line);
    std::string item;
    while (std::getline(ss, item, ',')) {
        result.push_back(item);
    }
    return result;
}

std::vector<std::string> readLines(const std::string& filename) {
    std::vector<std::string> lines;
    std::ifstream file(filename);
    if (!file.is_open()) {
        return lines; // 返回空向量表示读取失败
    }
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    return lines;
}

// 加载 Places：place_id,display_name,category,stay_time,open_time,close_time
bool loadPlaces(LGraph& graph, const std::string& filename, std::string& errorMsg) {
    auto lines = readLines(filename);
    if (lines.empty()) {
        errorMsg = "file_empty_or_unreadable";
        return false; // 文件读取失败或为空
    }

    for (size_t i = 0; i < lines.size(); ++i) {
        if (i == 0 && lines[i].find("place_id") != std::string::npos) {
            continue; // 跳过表头
        }
        auto fields = splitCSVLine(lines[i]);
        if (fields.size() != 6) {
            errorMsg = "invalid_csv_format";
            return false; // 格式错误
        }
        Place place{
            fields[0], // place_id
            fields[1], // display_name
            fields[2], // category
            std::stoi(fields[3]), // stay_time
            fields[4], // open_time
            fields[5]  // close_time
        };
        if (!graph.addPlace(place)) {
            errorMsg = "duplicate_place_id";
            return false; // 添加失败（如重复 ID）
        }
    }
    return true;
}

// 加载 Roads：from_id,to_id,distance,walk_time,status
bool loadRoads(LGraph& graph, const std::string& filename, std::string& errorMsg) {
    auto lines = readLines(filename);
    if (lines.empty()) {
        errorMsg = "file_empty_or_unreadable";
        return false; // 文件读取失败或为空
    }

    for (size_t i = 0; i < lines.size(); ++i) {
        if (i == 0 && lines[i].find("from_id") != std::string::npos) {
            continue; // 跳过表头
        }
        auto fields = splitCSVLine(lines[i]);
        if (fields.size() != 5) {
            errorMsg = "invalid_csv_format";
            return false; // 格式错误
        }
        Road road{
            fields[0], // from_id
            fields[1], // to_id
            std::stoi(fields[2]), // distance
            std::stoi(fields[3]), // walk_time
            fields[4]  // status
        };
        if (!graph.addRoad(road)) {
            errorMsg = "duplicate_road_or_place_not_exist";
            return false; // 添加失败（如重复道路或地点不存在）
        }
    }
    return true;
}

// 保存 Places：place_id,display_name,category,stay_time,open_time,close_time
bool savePlaces(const LGraph& graph, const std::string& filename, std::string& errorMsg) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        errorMsg = "file_unwritable";
        return false; // 文件无法写入
    }
    file << "place_id,display_name,category,stay_time,open_time,close_time\n"; // 表头
    for (const auto& place : graph.getAllPlaces()) {
        file << place.place_id << ","
             << place.display_name << ","
             << place.category << ","
             << place.stay_time << ","
             << place.open_time << ","
             << place.close_time << "\n";
    }
    file.close();
    return true;
}

// 保存 Roads：from_id,to_id,distance,walk_time,status
bool saveRoads(const LGraph& graph, const std::string& filename, std::string& errorMsg) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        errorMsg = "file_unwritable";
        return false; // 文件无法写入
    }
    file << "from_id,to_id,distance,walk_time,status\n"; // 表头
    auto places = graph.getAllPlaces();
    for (const auto& place : places) {
        auto roads = graph.getAdjacent(place.place_id);
        for (const auto& road : roads) {
            // 道路在图中以标准化方向保存，只在起点一侧输出一次
            if (place.place_id == road.from_id) {
                file << road.from_id << ","
                     << road.to_id << ","
                     << road.distance << ","
                     << road.walk_time << ","
                     << road.status << "\n";
            }
        }
    }
    file.close();
    return true;
}
