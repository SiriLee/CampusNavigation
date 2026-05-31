#include "CsvIO.h"
#include "CommandProcessor.h"

int main() {
    LGraph graph;
    CommandProcessor processor(graph);
    processor.run();
    return 0;
}