# Campus Navigation System

## 编译与运行

```bash
# 配置（使用 MinGW Makefiles）
cmake -B build -G "MinGW Makefiles"

# 编译
cmake --build build

# 运行（Windows + MinGW）
.\build\CampusNavigation.exe

# 也可以用命令文件做批处理输入
.\build\CampusNavigation < command.txt > output.txt
```

## 测试程序

项目内置了一个黑盒测试程序，会逐个运行 `test_data/must_do` 下的测试用例目录，并将程序输出与对应目录中的 `answer.txt` 做对比。

### 1. 先编译测试目标

```bash
cmake -B build -G "MinGW Makefiles"
cmake --build build
```

### 2. 直接运行测试程序

```bash
.\build\tests\CampusNavigationTests.exe .\build\CampusNavigation.exe .\test_data\must_do .\build\test_outputs
```

运行后会在控制台输出每个用例的 `PASS` / `FAIL`，并在 `build\test_outputs` 下保存每个用例的实际输出文件，便于你本地逐个查看和对照。

### 3. 通过 CTest 运行

如果你的环境已经生成了测试配置，也可以直接使用 CTest：

```bash
ctest --test-dir build --output-on-failure
```

### 4. 手动查看某个用例

每个测试用例都位于 `test_data/must_do/<case_name>/`，目录里通常包含：

- `command.txt`：输入命令
- `answer.txt`：规范输出或参考输出
- `places.csv`、`roads.csv`：该用例使用的数据

你可以单独进入某个目录，把 `command.txt` 作为标准输入喂给程序，再将输出与 `answer.txt` 对比：

```bash
.\build\CampusNavigation.exe < .\test_data\must_do\small_cases\case_01\command.txt > .\build\case_01_output.txt
```

如果只想看测试 runner 的结果，可以直接打开 `build\test_outputs` 里的输出文件，或在命令行中查看对应的 `*_output.txt`。

## 测试原理与判定准则

### 测试原理（黑盒）

测试程序不依赖内部实现，只验证命令行输入输出行为是否符合预期。核心流程如下：

1. 从 `test_data/must_do` 递归扫描测试目录，识别同时包含 `command.txt` 和 `answer.txt` 的目录作为测试用例。
2. 对每个测试用例：
	- 将该用例目录作为进程工作目录；
	- 将 `command.txt` 重定向到被测程序标准输入；
	- 将标准输出/标准错误重定向到 `build/test_outputs/<case_name>_output.txt`。
3. 程序退出码为 0 且输出比较通过，则记为 `PASS`；否则记为 `FAIL` 并附失败原因。

### 输出归一化规则

在比较输出前，测试程序会先做轻量归一化，减少平台差异带来的误报：

1. 换行归一化：统一为 `\n`（兼容 Windows 的 `\r\n`）。
2. 行尾空白归一化：去除每行末尾空格和制表符。

### 判定准则

默认采用”逐行比较”。同时为了避免多解场景导致的非必要失败，对以下输出做了规则化比较：

1. `PATH ...` 行：
	- 要求 `PATH <MODE> <TOTAL_COST> NODES ...` 的 `MODE` 与 `TOTAL_COST` 一致；
	- 路径首节点和末节点与参考输出一致；
	- 中间节点允许不同（用于兼容等代价多条可行路径）。
	- **已知局限**：`MUST_PASS` 的必经点是否确实被访问，无法仅从 PATH 输出行验证；
	  若需验证必经点，应解析 `command.txt` 识别 MUST_PASS 行并检查必经点在路径中的出现顺序。
2. `MST ...` 行：
	- 要求 `MST <TOTAL_DISTANCE> EDGES ...` 的 `TOTAL_DISTANCE` 一致；
	- 要求边数一致（连通图的任何合法 MST 都有 V-1 条边）；
	- 允许具体边集合/顺序与参考输出不同（用于兼容同权边导致的多种等价 MST）。
3. `CRITICAL ...` 行：
	- 要求 `CRITICAL NODES <n> <nodes...> EDGES <m> <edges...>` 的节点数和边数一致；
	- 使用集合比较验证割点集合和桥集合（与顺序无关）。
4. 其他行：
	- 仍要求与参考输出严格一致（在完成归一化之后）。

### 失败类型说明

测试程序当前会输出以下常见失败原因：

1. `cannot_open_command_file`：无法打开用例输入文件。
2. `cannot_create_output_file`：无法创建输出文件。
3. `cannot_launch_program`：测试程序无法启动被测程序。
4. `process_timeout`：被测程序超时未退出（当前超时阈值：30 秒）。
5. `process_exit_<code>`：被测程序非 0 退出。
6. `output_mismatch`：输出与判定准则不一致。

### 设计准则（维护建议）

1. 优先检验接口语义，不绑定实现细节（黑盒思想）。
2. 在“必须严格”的字段上严格，在“允许多解”的字段上放宽。
3. 保留每个用例实际输出文件，便于定位差异和回归分析。
4. 若后续新增命令，建议先定义该命令的”严格字段”和”可放宽字段”，再更新比较逻辑。

## 拓展功能测试（SHORTEST_K / 共享单车券）

拓展功能实现了分层图最短路算法，支持在路径中使用最多 K 张共享单车券来缩短通行时间。

### 运行拓展测试

```bash
.\build\tests\CampusNavigationExploreTests.exe .\build\CampusNavigation.exe .\test_data\explore .\build\test_outputs_explore
```

测试数据位于 `test_data/explore/`，与 `must_do` 类似，每个用例目录包含 `command.txt`、`answer.txt`、`places.csv`、`roads.csv`。

### SHORTEST_K 命令输出格式

```
PATH <totalTime> K_USED <usedK> NODES <id1> <id2> ... FAST <count> [<u1>-<v1> <u2>-<v2> ...]
```

### SHORTEST_K 判定准则

1. **严格字段**：
   - `totalTime`：总耗时（分钟），必须与参考输出一致（算法应返回最优解）
   - `usedK`：实际使用的券数，必须一致
   - `FAST <count>`：快速边计数，必须一致且等于 `usedK`
   - 路径首节点和末节点：必须与参考输出一致

2. **可放宽字段**：
   - 中间节点：允许不同（用于兼容等代价多条可行路径）
   - FAST 边集合：允许与参考输出不同（等代价替代方案可能使用不同的边集合）。比较时使用集合相等性（顺序无关），边已由算法标准化（`from_id <= to_id`）并字典序排列

3. **错误输出**：
   - `NO_PATH`：无可达路径（严格比较）
   - `ERROR invalid_k`：K 超出范围 [0, 10]（严格比较）
   - `ERROR place_not_found`：起点或终点不存在（严格比较）

### 共享单车券语义

每张券可将一条边的步行耗时缩短为 `walk_time / 3`（整数除法）。算法在 `K` 张券的限制下求总耗时最小的路径。同一路径中每条边最多使用一张券。

### 已知注意事项

- 券只能在 `open` 状态的道路上使用
- K=0 时退化为标准最短路（按步行时间），此时输出与其他 PATH 命令格式一致
- 参考答案的 `totalTime` 基于特定券时间公式生成；若算法实现中的公式（整除截断 vs. 向上取整等）不同，可能导致实际输出与参考输出不一致