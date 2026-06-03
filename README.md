# Campus Navigation System

## 编译与运行

```bash
# 配置（使用 MinGW Makefiles）
cmake -B build -G "MinGW Makefiles"

# 编译
cmake --build build

# 运行（Windows + MinGW）
.\build\CampusNavigation.exe

.\build\CampusNavigation < command.txt > output.txt