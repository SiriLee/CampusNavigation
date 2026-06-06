cmake_minimum_required(VERSION 3.10)   # 设置最低 CMake 版本
project(CampusNavigation)              # 项目名称

# 使用 C++17 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 启用测试
enable_testing()

# ── GUI 构建选项 ──
option(BUILD_GUI "Build with Qt6 GUI support" OFF)

# ── GUI 前置配置（必须在 add_executable 之前，确保 AUTOMOC 生效） ──
if(BUILD_GUI)
    if(NOT CMAKE_PREFIX_PATH)
        message(FATAL_ERROR
            "BUILD_GUI=ON requires CMAKE_PREFIX_PATH to point to Qt6 installation.\n"
            "  Use cmake --preset gui, or set -DCMAKE_PREFIX_PATH=<qt_mingw_path>")
    endif()

    find_package(Qt6 REQUIRED COMPONENTS Widgets)

    # 启用 Qt 元对象编译器（MOC），必须在 add_executable 之前设置
    set(CMAKE_AUTOMOC ON)
endif()

# ── 核心源文件（CLI 和 GUI 共用） ──
set(CORE_SOURCES
    src/LGraph.cpp
    src/CsvIO.cpp
    src/CommandProcessor.cpp
    src/Algorithm.cpp
)

# ── GUI 源文件 ──
set(GUI_SOURCES
    src/gui_main.cpp
    src/gui/MainWindow.cpp
    src/gui/GraphWidget.cpp
    src/gui/ForceLayout.cpp
)

# ── 可执行文件 ──
add_executable(CampusNavigation
    src/main.cpp
    ${CORE_SOURCES}
)

# 指定头文件目录
target_include_directories(CampusNavigation PRIVATE include)

# ── GUI 后置配置 ──
if(BUILD_GUI)
    target_sources(CampusNavigation PRIVATE ${GUI_SOURCES})
    target_include_directories(CampusNavigation PRIVATE src/gui)
    target_link_libraries(CampusNavigation PRIVATE Qt6::Widgets)
    target_compile_definitions(CampusNavigation PRIVATE BUILD_GUI=1)

    # 编译后自动部署 Qt DLL 到构建目录
    if(WIN32)
        add_custom_command(TARGET CampusNavigation POST_BUILD
            COMMAND "${CMAKE_PREFIX_PATH}/bin/windeployqt.exe"
                --no-translations
                "$<TARGET_FILE:CampusNavigation>"
            COMMENT "Deploying Qt runtime DLLs..."
        )
    endif()
endif()

# 添加测试子目录
add_subdirectory(tests)
