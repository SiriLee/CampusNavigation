#ifndef CSV_IO_H
#define CSV_IO_H

#include "LGraph.h"
#include <string>
#include <vector>

std::vector<std::string> splitCSVLine(const std::string& line);
std::vector<std::string> readLines(const std::string& filename);

bool loadPlaces(LGraph& graph, const std::string& filename, std::string& errorMsg);
bool loadRoads(LGraph& graph, const std::string& filename, std::string& errorMsg);
bool savePlaces(const LGraph& graph, const std::string& filename, std::string& errorMsg);
bool saveRoads(const LGraph& graph, const std::string& filename, std::string& errorMsg);

#endif // CSV_IO_H