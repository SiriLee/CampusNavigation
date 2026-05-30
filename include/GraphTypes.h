#include <string>

struct Place {
    std::string place_id; // 唯一 ID，如 `P0001`，无空格
    std::string display_name; // 展示名称，无空格
    std::string category; // 类别：`Teaching`、`Dining`、`Dormitory`、`Sports`、`Medical`、`Other` 等
    int stay_time; // 建议停留时间（分钟）
    std::string open_time; // 开放时间，如 `08:00`
    std::string close_time; // 关闭时间，如 `22:00`
};

struct Road {
    std::string from_id; // 起点 place_id
    std::string to_id; // 终点 place_id
    int distance; // 距离（米）
    int walk_time; // 步行耗时（分钟）
    std::string status; // `open` 或 `closed`
};