## GUI开发约束文档

### 1. 总体目标

为现有C++校园导航CLI程序增加一个**图形化界面（GUI）**，通过 `--gui` 参数启动，满足 [任务](Task.md) 的要求。GUI **不替代CLI**，且尽可能少修改现有核心代码（`LGraph`、`Algorithm`、`CommandProcessor` 等仅作最小必要的暴露，不改变原有逻辑）。

### 2. 技术选型

- **框架**：Qt 6 (Widgets 模块)，使用 MinGW 64-bit 编译。
- **构建集成**：在根 `CMakeLists.txt` 中添加 `option(BUILD_GUI "Build GUI version" OFF)`，默认OFF；当 `-DBUILD_GUI=ON` 时，`find_package(Qt6 REQUIRED COMPONENTS Widgets)`，并编译一个额外的可执行文件或添加 `gui_main.cpp`。为保持独立，建议创建 `src/gui/` 目录存放GUI源码，不混入原有 `src/`。
- **启动方式**：在 `main.cpp` 中检查命令行参数 `--gui`，若存在则调用 `runGui(argc, argv)`（定义在单独的 `gui_main.cpp`），否则执行原有 `CommandProcessor::run()`。

### 3. 功能范围（最小化可行产品）

- **图的可视化**：
  - 节点位置采用**力导向布局**（优先使用已有库，如 Qt 自带的 `QGraphicsScene` 配合简单的弹簧-电荷模型，或调用第三方库如 `OGDF`，但为简化，建议手写一个简单的 Fruchterman-Reingold 迭代算法，代码量约150行）。
  - 边绘制区分 `open`（黑色实线）和 `closed`（红色虚线）。
  - 节点标注显示 `place_id`（或 `display_name` 缩写）。
- **路径高亮**：用户通过两个 `QLineEdit` 输入起点和终点的 `place_id`，点击“计算最短路径”按钮后，调用现有 `dijkstra` 函数（复用 `Algorithm.h`），获得路径节点序列，在图上高亮（绿色粗线）。
- **关键节点/边标识**：在图上用不同颜色/形状标出（例如关键节点外圈红色，关键边紫色加粗）。可通过另一个按钮“分析关键”触发，调用 `computeCritical`，然后刷新样式。
- **无交互式拖拽节点**（不强制，后续可选），但需要能够展示清晰布局。

### 4. 代码组织

```
CampusNavigation/
├── CMakeLists.txt          # 增加 option(BUILD_GUI) 和 GUI 目标
├── src/gui_main.cpp        # 包含 runGui() 入口，创建 QApplication 和主窗口
├── src/gui/                # 新增目录
│   ├── MainWindow.h/cpp    # 主窗口，包含菜单、工具栏、画布
│   ├── GraphWidget.h/cpp   # 继承 QGraphicsView，负责绘制节点、边、高亮
│   └── ForceLayout.h/cpp   # 力导向布局算法（静态函数）
└── include/                # 无修改，但需确保 LGraph 等头文件可见
```

### 5. 核心接口复用

- 直接使用现有的 `LGraph` 对象（由 `main` 创建并传递给 GUI）。
- 调用 `Algorithm` 中的 `dijkstra`、`computeCritical` 等函数（已完全解耦）。
- GUI 中不重复实现图算法。

### 6. 编译与运行说明

- 编译 GUI 版本：`cmake -B build -DBUILD_GUI=ON -G "MinGW Makefiles" && cmake --build build`
- 运行：`./build/CampusNavigation.exe --gui`
- 若不提供 `--gui`，则运行原有 CLI。

### 7. 测试与验收

- 使用现有的 `test_data/custom` 加载，验证显示正确性。
- 手动测试：选择起点终点，计算最短路径，观察高亮；点击“关键分析”，观察节点/边标识。
- 截图保存至少3张（展示不同状态），放入 `docs/gui_screenshots/`。