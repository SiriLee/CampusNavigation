#include "LGraph.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

class CommandProcessor {
public:
    explicit CommandProcessor(LGraph& graph);  // 持有对图的引用
    void run();  // 主循环：读取 stdin 直到 QUIT

private:
    LGraph& graph_;  // 引用传递，避免复制
    std::unordered_map<std::string, std::function<void(const std::vector<std::string>&)>> handlers_; // 命令映射

    // 初始化 handlers_ 映射表
    void initHandlers();

    // 各个命令的处理函数
    void cmdLoad(const std::vector<std::string>& args);
    void cmdSave(const std::vector<std::string>& args);
    void cmdAddPlace(const std::vector<std::string>& args);
    void cmdDeletePlace(const std::vector<std::string>& args);
    void cmdUpdatePlace(const std::vector<std::string>& args);
    void cmdAddRoad(const std::vector<std::string>& args);
    void cmdDeleteRoad(const std::vector<std::string>& args);
    void cmdUpdateRoad(const std::vector<std::string>& args);
    void cmdCloseRoad(const std::vector<std::string>& args);
    void cmdOpenRoad(const std::vector<std::string>& args);
    void cmdQueryPlace(const std::vector<std::string>& args);
    void cmdQueryCategory(const std::vector<std::string>& args);
    void cmdADJ(const std::vector<std::string>& args);
    void cmdComponents(const std::vector<std::string>& args);
    void cmdShortest(const std::vector<std::string>& args);
    void cmdTimedShortest(const std::vector<std::string>& args);

    // 辅助函数：分割输入行
    std::vector<std::string> splitLine(const std::string& line);
    // 辅助函数：检查参数数量，输出错误并返回 false
    bool checkArgCount(const std::vector<std::string>& args, size_t expected);
};