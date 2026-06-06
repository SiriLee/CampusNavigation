#include "CsvIO.h"
#include "CommandProcessor.h"

#include <iostream>
#include <string>

#ifdef BUILD_GUI
int runGui(int argc, char* argv[]);
#endif

int main(int argc, char* argv[]) {
    // 检查命令行参数 --gui
    bool hasGui = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--gui") {
            hasGui = true;
            break;
        }
    }

#ifdef BUILD_GUI
    if (hasGui) {
        return runGui(argc, argv);
    }
#else
    if (hasGui) {
        // GUI 未编译支持，输出提示后继续进入 CLI
        std::cout << "WARNING: GUI support not compiled (BUILD_GUI=OFF). Starting CLI mode." << std::endl;
    }
#endif

    LGraph graph;
    CommandProcessor processor(graph);
    processor.run();
    return 0;
}
