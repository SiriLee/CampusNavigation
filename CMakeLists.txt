cmake_minimum_required(VERSION 3.10)   # 设置最低 CMake 版本
project(CampusNavigation)              # 项目名称

# 使用 C++17 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加可执行文件
add_executable(CampusNavigation
    src/main.cpp
    src/LGraph.cpp
    src/CsvIO.cpp
)

# 指定头文件目录
target_include_directories(CampusNavigation PRIVATE include)