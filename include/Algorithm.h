#ifndef ALGORITHM_H
#define ALGORITHM_H

#include "LGraph.h"
#include <utility>
#include <vector>

// 连通分量分析，返回<分量个数, 各分量大小列表（已降序）>
std::pair<int, std::vector<int>> computeComponents(const LGraph& graph);

#endif // ALGORITHM_H