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

默认采用“逐行比较”。同时为了避免多解场景导致的非必要失败，对以下输出做了规则化比较：

1. `PATH ...` 行：
	- 要求 `PATH <MODE> <TOTAL_COST> NODES ...` 的 `MODE` 与 `TOTAL_COST` 一致；
	- 路径首节点和末节点与参考输出一致；
	- 中间节点允许不同（用于兼容等代价多条可行路径）。
2. `MST ...` 行：
	- 要求 `MST <TOTAL_DISTANCE> EDGES ...` 的 `TOTAL_DISTANCE` 一致；
	- 允许具体边集合/顺序与参考输出不同（用于兼容同权边导致的多种等价 MST）。
3. 其他行：
	- 仍要求与参考输出严格一致（在完成归一化之后）。

### 失败类型说明

测试程序当前会输出以下常见失败原因：

1. `cannot_open_command_file`：无法打开用例输入文件。
2. `cannot_create_output_file`：无法创建输出文件。
3. `cannot_launch_program`：测试程序无法启动被测程序。
4. `process_exit_<code>`：被测程序非 0 退出。
5. `output_mismatch`：输出与判定准则不一致。

### 设计准则（维护建议）

1. 优先检验接口语义，不绑定实现细节（黑盒思想）。
2. 在“必须严格”的字段上严格，在“允许多解”的字段上放宽。
3. 保留每个用例实际输出文件，便于定位差异和回归分析。
4. 若后续新增命令，建议先定义该命令的“严格字段”和“可放宽字段”，再更新比较逻辑。