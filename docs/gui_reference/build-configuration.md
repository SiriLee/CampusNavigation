# GUI 构建说明

## 环境要求

- Qt 6 (MinGW 64-bit)，安装路径 `D:\Work\Development\Runtimes\Qt\6.11.1\mingw_64\`
- Qt 自带 MinGW 13.1.0 (`Tools\mingw1310_64\`)，路径已写入 `CMakePresets.json`

> **注意**：系统上另有一套 MinGW GCC 10.3.0 (TDM64)，编译 CLI 版本使用该编译器。编译 GUI 版本必须使用 Qt 自带的 GCC 13.1.0（ABI 兼容性要求）。两个编译器通过 CMake preset 自动区分，不需要手动切换 PATH。

## 编译与运行

```bash
# CLI 版本（无需 Qt，使用系统 MinGW）
cmake --preset default
cmake --build build
./build/CampusNavigation.exe

# GUI 版本（需要 Qt，使用 Qt MinGW）
cmake --preset gui
cmake --build build-gui
./build-gui/CampusNavigation.exe --gui

# 运行测试
ctest --test-dir build --output-on-failure
```

> `cmake --preset gui` 无需设置任何环境变量，路径已硬编码在 `CMakePresets.json` 中。在其他机器上使用时修改该文件中的 Qt 路径即可。

## CMakePresets.json 说明

```json
{
  "configurePresets": [
    { "name": "default",  "binaryDir": "build",     "BUILD_GUI": "OFF" },
    { "name": "gui",      "binaryDir": "build-gui", "BUILD_GUI": "ON",
      "CMAKE_PREFIX_PATH":  "D:/.../Qt/6.11.1/mingw_64",
      "CMAKE_C_COMPILER":   "D:/.../Qt/Tools/mingw1310_64/bin/gcc.exe",
      "CMAKE_CXX_COMPILER": "D:/.../Qt/Tools/mingw1310_64/bin/g++.exe" }
  ]
}
```

CLI 和 GUI 使用独立的 `build` / `build-gui` 目录，互不干扰。

## 常见问题

**Q: 运行 GUI 时提示 "找不到 Qt6Widgets.dll"**

正常情况下不会出现——CMakeLists.txt 已配置 `windeployqt` 作为编译后步骤，每次 `cmake --build` 后自动将 Qt DLL 部署到 exe 同目录。如果手动移动了 exe，重新编译即可自动修复。
