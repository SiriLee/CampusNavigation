# Campus Navigation System

C++ 校园导航程序，支持命令行（CLI）与图形化界面（GUI）两种交互方式。核心功能包括地点/道路管理、最短路径规划、必经点路径、时刻约束路径、最小生成树、关键节点/边分析，以及共享单车券分层图最短路。

> 📋 **实验报告**：[docs/project-report.md](docs/project-report.md) — 数据结构设计、算法复杂度分析、测试方案、AI 协作记录

---

## 项目架构

```
CampusNavigation/
├── CMakeLists.txt              # 根 CMake（C++17，含 GUI option 和测试子目录）
├── CMakePresets.json           # CMake 构建预设（default=CLI, gui=Qt6）
├── README.md
├── docs/
│   ├── project-report.md       # 实验报告（数据结构、算法、测试、AI 协作记录）
│   ├── gui-user-guide.md       # GUI 用户指南（编译、操作、截图）
│   ├── custom-dataset.md       # 自定义场景数据集说明
│   └── adversarial-dataset.md  # 对抗样例数据集说明
├── include/                    # 头文件
│   ├── GraphTypes.h            # Place / Road 数据结构
│   ├── LGraph.h                # 图存储层（邻接表）
│   ├── CsvIO.h                 # CSV 读写
│   ├── Algorithm.h             # 所有算法声明 + 结果结构体
│   └── CommandProcessor.h      # 命令解析与分发
├── src/                        # 实现文件
│   ├── main.cpp                # 入口：--gui → runGui() / 否则 CLI
│   ├── gui_main.cpp            # GUI 入口 runGui()
│   ├── gui/                    # GUI 源码
│   │   ├── MainWindow.h/cpp    # 主窗口（菜单栏、控件栏、GraphWidget）
│   │   ├── GraphWidget.h/cpp   # 图绘制（节点/边/高亮，手绘 QWidget）
│   │   └── ForceLayout.h/cpp   # Fruchterman-Reingold 力导向布局
│   ├── LGraph.cpp              # 图存储实现（邻接表 + 边键集合）
│   ├── CsvIO.cpp               # CSV 加载与保存
│   ├── Algorithm.cpp           # Dijkstra / Tarjan / Kruskal / BFS 等
│   └── CommandProcessor.cpp    # 20 条命令的解析与处理
├── tests/                      # 测试程序
│   ├── test_common.h           # 共享基础设施（归一化、比较器、进程启动）
│   ├── test_core.cpp           # 基础功能测试
│   ├── test_explore.cpp        # SHORTEST_K 拓展测试
│   ├── test_custom.cpp         # 自定义场景测试
│   ├── test_adversarial.cpp    # 对抗样例测试
│   └── CMakeLists.txt          # 4 个测试目标
└── test_data/                  # 测试数据
    ├── must_do/                # 基础功能（13 个用例）
    ├── explore/                # SHORTEST_K（4 个用例）
    ├── custom/                 # 虚拟校园场景（1 个用例）
    └── adversarial/            # 对抗样例（5 个用例）
```

### 组件职责

| 组件 | 职责 |
|------|------|
| `GraphTypes.h` | `Place`（6 字段）和 `Road`（5 字段）结构体 |
| `LGraph` | 无向图存储。邻接表 `adj_` + 边键集合 `edge_keys_` 判重。边标准化（`from_id <= to_id`） |
| `CsvIO` | `loadPlaces` / `loadRoads` / `savePlaces` / `saveRoads`，自动跳过 CSV 表头 |
| `Algorithm` | 所有图算法：Dijkstra、Tarjan、Kruskal、BFS 连通分量、分层图最短路 |
| `CommandProcessor` | 从 `stdin` 逐行读取命令，按命令名分发到 handler；命令格式错误时输出 `ERROR` |
| `MainWindow` | GUI 主窗口：菜单栏、控件栏、图数据加载与算法调用 |
| `GraphWidget` | QWidget 子类，手绘节点（圆+标签）、边（实线/虚线）、路径高亮（绿色）、关键边（紫色） |
| `ForceLayout` | Fruchterman-Reingold 力导向布局，70 次迭代，仅使用 open 边 |

---

## 编译与运行

**环境要求**：CMake ≥ 3.19（presets），GCC/MinGW 支持 C++17。GUI 额外需要 Qt 6 (Widgets 模块, MinGW 64-bit)。

```bash
# CLI 版本（无需 Qt，使用系统 MinGW）
cmake --preset default
cmake --build build
./build/CampusNavigation.exe

# GUI 版本（需要 Qt 6，使用 Qt 自带 MinGW）
cmake --preset gui
cmake --build build-gui
./build-gui/CampusNavigation.exe --gui          # GUI 模式
./build-gui/CampusNavigation.exe                # CLI 模式（不加 --gui）

# 运行测试
ctest --test-dir build --output-on-failure
```

> **Qt 路径配置**：`cmake --preset gui` 无需设置环境变量，路径已硬编码在 `CMakePresets.json`。其他机器使用时编辑该文件中的 Qt 路径即可。详见 [GUI 用户指南](docs/gui-user-guide.md)。

---

## 数据格式

### places.csv

```csv
place_id,display_name,category,stay_time,open_time,close_time
P001,第一教学楼,Teaching,50,07:00,22:00
```

| 字段 | 类型 | 说明 |
|------|------|------|
| `place_id` | string | 唯一标识，无空格（如 `P0001`） |
| `display_name` | string | 展示名称，无空格 |
| `category` | string | `Teaching` / `Dining` / `Dormitory` / `Sports` / `Medical` / `Other` |
| `stay_time` | int | 建议停留时间（分钟） |
| `open_time` | string | 开放时间 `HH:MM`，闭区间起点 |
| `close_time` | string | 关闭时间 `HH:MM`，闭区间终点 |

### roads.csv

```csv
from_id,to_id,distance,walk_time,status
P001,P002,100,2,open
```

| 字段 | 类型 | 说明 |
|------|------|------|
| `from_id`, `to_id` | string | 端点 ID，无向 |
| `distance` | int | 距离（米） |
| `walk_time` | int | 步行耗时（分钟），共享单车券用后缩短为 `ceil(walk_time/3)` |
| `status` | string | `open` 或 `closed`；closed 道路不参与路径规划、MST、割点/桥分析 |

---

## 命令参考

命令从 `stdin` 逐行读入，空行和 `#` 开头的行被跳过，`QUIT` 退出。

### 数据管理

| 命令 | 格式 | 成功输出 |
|------|------|---------|
| `LOAD` | `LOAD <places.csv> <roads.csv>` | `OK` |
| `SAVE` | `SAVE <places.csv> <roads.csv>` | `OK` |
| `ADD_PLACE` | `ADD_PLACE <id> <name> <category> <stay> <open> <close>` | `OK` |
| `DELETE_PLACE` | `DELETE_PLACE <id>` | `OK` |
| `UPDATE_PLACE` | `UPDATE_PLACE <id> <field> <value>` | `OK` |
| `ADD_ROAD` | `ADD_ROAD <from> <to> <dist> <walk> <status>` | `OK` |
| `DELETE_ROAD` | `DELETE_ROAD <from> <to>` | `OK` |
| `UPDATE_ROAD` | `UPDATE_ROAD <from> <to> <field> <value>` | `OK` |
| `CLOSE_ROAD` | `CLOSE_ROAD <from> <to>` | `OK` |
| `OPEN_ROAD` | `OPEN_ROAD <from> <to>` | `OK` |

常见错误：`ERROR place_not_found`、`ERROR road_not_found`、`ERROR invalid_arguments`、`ERROR invalid_field`

### 查询

| 命令 | 格式 | 输出示例 |
|------|------|---------|
| `QUERY_PLACE` | `QUERY_PLACE <id>` | `PLACE P001 第一教学楼 Teaching 50 07:00 22:00` |
| `QUERY_CATEGORY` | `QUERY_CATEGORY <category>` | `CATEGORY Teaching 3 P001 P002 P003`（按 place_id 升序） |
| `ADJ` | `ADJ <id>` | `ADJ P001 4 P002:100:2:open P004:150:3:open ...`（按 neighbor_id 升序） |

### 图分析

| 命令 | 格式 | 输出示例 |
|------|------|---------|
| `COMPONENTS` | `COMPONENTS` | `COMPONENTS 2 SIZES 13 2`（分量数 + 各分量大小降序） |
| `MST` | `MST` | `MST 980 EDGES P001-P002:200 ...` 或 `DISCONNECTED` |
| `CRITICAL` | `CRITICAL` | `CRITICAL NODES 2 P002 P003 EDGES 2 P001-P002 P002-P003` |

- **MST**：Kruskal 算法，仅使用 `open` 边。不连通图输出 `DISCONNECTED`
- **CRITICAL**：Tarjan 算法求割点和桥，仅考虑 `open` 边。结果已排序，边标准化为 `from_id <= to_id`

### 路径规划

| 命令 | 格式 | 输出示例 |
|------|------|---------|
| `SHORTEST` | `SHORTEST <from> <to> <DIST\|TIME>` | `PATH DIST 400 NODES P001 P005 P007` |
| `TIMED_SHORTEST` | `TIMED_SHORTEST <from> <to> <HH:MM> <DIST\|TIME>` | 同上；时刻外返回 `NO_PATH` |
| `MUST_PASS` | `MUST_PASS <from> <to> <DIST\|TIME> <k> <p1> ... <pk>` | 同上；不可达返回 `NO_PATH` |
| `SHORTEST_K` | `SHORTEST_K <from> <to> <K>` | `PATH 3 K_USED 2 NODES ... FAST 2 P001-P005 P005-P007` |

- **SHORTEST**：Dijkstra，按 DIST 或 TIME（walk_time）求最短路
- **TIMED_SHORTEST**：Dijkstra + 顶点时间过滤器（`open_time ≤ time ≤ close_time`），起点或终点不在开放时间则 `NO_PATH`
- **MUST_PASS**：按必经点给定**顺序**逐段 Dijkstra 拼接，段间跳过重复端点
- **SHORTEST_K**：分层图 Dijkstra（最多 K 张共享单车券），券将 `walk_time` 缩短为 `ceil(walk_time/3)`；K=0 退化为标准 Dijkstra

---

## 关键算法

### Dijkstra（`dijkstra`）

通用实现，支持：
- **权重函数**：`WeightFunc` → `int(const Road&)`，DIST 用 `road.distance`，TIME 用 `road.walk_time`
- **顶点过滤器**：`VertexFilter` → `bool(const string&)`，TIMED_SHORTEST 用于排除不在开放时间的顶点
- **确定性**：优先队列使用 `std::pair<int, string>` + `std::greater<>`，代价相同时顶点 ID 字典序决定出队顺序

### MUST_PASS（`mustPassPath`）

1. 构造点序列：`[from, p1, p2, ..., pk, to]`
2. 对每对相邻点调用 Dijkstra，拼接路径（跳过重复段间节点）
3. 任一段不可达则整体返回 `NO_PATH`

### 分层图最短路（`shortestPathWithK`）

- K=0 退化为标准 Dijkstra
- K>0：状态为 `(place_id, used)`，共 K+1 层
  - 不使用券：走同层，代价 `+walk_time`（仅 open 边）
  - 使用券：走下一层，代价 `+ceil(walk_time/3)`（仅 open 边）
- 回溯时收集使用券的边，标准化并字典序排序

### Kruskal MST（`computeMST`）

1. 收集所有 `open` 边，按 distance 升序排序
2. DSU 贪心选边
3. 若边数 = V-1 则连通；否则 `DISCONNECTED`

### Tarjan 割点/桥（`computeCritical`）

- 仅使用 `open` 边构建邻接表
- 桥判定：`low[v] > disc[u]`（严格大于）
- 割点判定：根节点 `children > 1`；非根节点 `low[v] >= disc[u]`
- 孤立点（parent=-1, children=0）不是割点

### 连通分量（`computeComponents`）

- BFS 遍历所有顶点（含无边孤立点）
- 仅使用 `open` 边
- 分量大小降序排列

---

## 测试基础设施

### 测试架构

所有测试程序共享 [`tests/test_common.h`](tests/test_common.h)，采用策略模式——每个测试仅定义自己的 `compareOutputLine` 函数：

```
test_common.h  ───  runTestSuite<CompareFn>() 模板
                 │   归一化、文件 I/O、进程启动、用例发现
                 │   compareStandardPathLines / compareShortestKPathLines
                 │   compareMstLines / compareCriticalLines
                 │
  ├─ test_core.cpp        → must_do 基础功能测试
  ├─ test_explore.cpp     → SHORTEST_K 拓展测试
  ├─ test_custom.cpp      → 自定义场景测试
  └─ test_adversarial.cpp → 对抗样例测试
```

### 测试原理（黑盒）

1. 扫描数据目录，识别包含 `command.txt` + `answer.txt` 的目录为用例
2. 以用例目录为工作目录启动被测程序，重定向 `stdin`/`stdout`
3. 归一化实际输出（换行统一、行尾空白裁剪），与 `answer.txt` 逐行比较
4. 支持 30 秒超时保护

### 输出归一化

1. `\r\n` → `\n`
2. 每行末尾空格和制表符移除

### 行比较策略

| 行类型 | 严格字段 | 宽松字段 |
|--------|---------|---------|
| `PATH <MODE> <COST> NODES ...` | MODE、COST、首节点、末节点 | 中间节点（等代价替代路径） |
| `PATH <TIME> K_USED <K> NODES ... FAST <N> ...` | totalTime、usedK、首/末节点、FAST 边数 | 中间节点、FAST 边集合（集合比较） |
| `MST <DIST> EDGES ...` | 总距离、边数（token 数） | 具体边集合（同权边等价 MST） |
| `CRITICAL NODES ... EDGES ...` | 割点数、桥数、节点/边集合 | 输出顺序 |
| 其他行 | 全部严格 | — |

### 运行测试

```bash
cmake --build build

# 四个测试套件
./build/tests/CampusNavigationTests.exe           ./build/CampusNavigation.exe ./test_data/must_do      ./build/test_outputs
./build/tests/CampusNavigationExploreTests.exe     ./build/CampusNavigation.exe ./test_data/explore      ./build/test_outputs_explore
./build/tests/CampusNavigationCustomTests.exe      ./build/CampusNavigation.exe ./test_data/custom       ./build/test_outputs_custom
./build/tests/CampusNavigationAdversarialTests.exe ./build/CampusNavigation.exe ./test_data/adversarial  ./build/test_outputs_adversarial

# 或通过 CTest
ctest --test-dir build --output-on-failure
```

当前全部通过：**must_do 13/13, explore 4/4, custom 1/1, adversarial 5/5**

### 测试数据一览

| 数据集 | 位置 | 用例数 | 说明 |
|--------|------|--------|------|
| must_do | `test_data/must_do/` | 13 | 基础功能全覆盖（small/medium/large/sample） |
| explore | `test_data/explore/` | 4 | SHORTEST_K 分层图最短路 |
| custom | `test_data/custom/` | 1 | 15 节点虚拟校园场景，详见 [`docs/custom-dataset.md`](docs/custom-dataset.md) |
| adversarial | `test_data/adversarial/` | 5 | 等代价路径、必经点顺序、时间边界、桥/割点、孤立点，详见 [`docs/adversarial-dataset.md`](docs/adversarial-dataset.md) |

---

## 图形化界面

通过 `--gui` 启动图形界面，提供图可视化、最短路径高亮和关键节点/边分析功能。所有算法直接复用 CLI 的 `Algorithm.h` 和 `LGraph` 接口，不重复实现。

### 功能

- **图可视化**：力导向布局自动排列节点，黑色实线 = `open` 边，黑色虚线 = `closed` 边
- **路径高亮**：输入起点/终点，选择 DIST/TIME 模式，绿色粗线标出最短路径
- **关键分析**：紫色粗线标出桥边，红色外圈标出割点
- **数据加载**：File → Open CSV 选择 places/roads 文件

### 编译

```bash
cmake --preset gui && cmake --build build-gui
```

### 代码组织

```
src/gui_main.cpp               # runGui()：QApplication + MainWindow
src/gui/MainWindow.h/cpp        # 主窗口，菜单栏 + 控件栏 + GraphWidget
src/gui/GraphWidget.h/cpp       # QWidget 子类，paintEvent 手绘
src/gui/ForceLayout.h/cpp       # Fruchterman-Reingold 算法
```

完整说明见 [GUI 用户指南](docs/gui-user-guide.md)。
