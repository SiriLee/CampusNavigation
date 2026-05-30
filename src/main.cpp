#include "CsvIO.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

void printPlace(const Place* place) {
    if (!place) {
        std::cout << "  place = nullptr\n";
        return;
    }
    std::cout << "  place_id=" << place->place_id
              << ", display_name=" << place->display_name
              << ", category=" << place->category
              << ", stay_time=" << place->stay_time
              << ", open_time=" << place->open_time
              << ", close_time=" << place->close_time << '\n';
}

void printRoads(const std::vector<Road>& roads) {
    for (const auto& road : roads) {
        std::cout << "  road: " << road.from_id << " -> " << road.to_id
                  << ", distance=" << road.distance
                  << ", walk_time=" << road.walk_time
                  << ", status=" << road.status << '\n';
    }
}

void writeTestCsvFiles() {
    {
        std::ofstream placesFile("test_places.csv");
        placesFile << "place_id,display_name,category,stay_time,open_time,close_time\n";
        placesFile << "P001,Library,Teaching,30,08:00,22:00\n";
        placesFile << "P002,Canteen,Dining,45,07:00,20:30\n";
        placesFile << "P003,Gym,Sports,60,06:00,21:00\n";
    }
    {
        std::ofstream roadsFile("test_roads.csv");
        roadsFile << "from_id,to_id,distance,walk_time,status\n";
        roadsFile << "P001,P002,120,2,open\n";
        roadsFile << "P002,P003,180,3,open\n";
    }
}

void printCheck(const std::string& label, bool actual, bool expected = true) {
    std::cout << label << ": " << ((actual == expected) ? "PASS" : "FAIL") << '\n';
}

} // namespace

int main() {
    std::cout << "Campus Navigation interface smoke test" << std::endl;
    writeTestCsvFiles();

    LGraph graph;

    std::cout << "\n[1] Place operations\n";
    Place p1{"P001", "Library", "Teaching", 30, "08:00", "22:00"};
    Place p2{"P002", "Canteen", "Dining", 45, "07:00", "20:30"};
    Place p3{"P003", "Gym", "Sports", 60, "06:00", "21:00"};

    printCheck("addPlace(P001)", graph.addPlace(p1));
    printCheck("addPlace(P002)", graph.addPlace(p2));
    printCheck("addPlace(P003)", graph.addPlace(p3));
    printCheck("placeExists(P001)", graph.placeExists("P001"));
    std::cout << "P001 after add:\n";
    printPlace(graph.getPlace("P001"));

    printCheck("updatePlace(P001.display_name)", graph.updatePlace(p1, "display_name", "CentralLibrary"));
    printCheck("updatePlace(P002.stay_time)", graph.updatePlace(p2, "stay_time", "50"));
    std::cout << "P001 after update:\n";
    printPlace(graph.getPlace("P001"));
    std::cout << "P002 after update:\n";
    printPlace(graph.getPlace("P002"));

    printCheck("deletePlace(P003)", graph.deletePlace("P003"));
    printCheck("placeExists(P003) expected false", graph.placeExists("P003"), false);

    std::cout << "\n[2] Road operations\n";
    Road r1{"P001", "P002", 120, 2, "open"};
    Road r2{"P002", "P003", 180, 3, "open"};

    printCheck("addRoad(P001-P002)", graph.addRoad(r1));
    printCheck("addRoad(P002-P003 expected false because P003 deleted)", graph.addRoad(r2), false);
    std::cout << "Adjacent roads from P001:\n";
    printRoads(graph.getAdjacent("P001"));

    printCheck("updateRoad(P001-P002.distance)", graph.updateRoad("P001", "P002", "distance", "150"));
    printCheck("closeRoad(P001-P002)", graph.closeRoad("P001", "P002"));
    std::cout << "Adjacent roads from P001 after update:\n";
    printRoads(graph.getAdjacent("P001"));

    printCheck("openRoad(P001-P002)", graph.openRoad("P001", "P002"));
    printCheck("deleteRoad(P001-P002)", graph.deleteRoad("P001", "P002"));
    printCheck("roadExists(P001-P002) expected false", graph.roadExists("P001", "P002"), false);

    std::cout << "\n[3] CSV load/save\n";
    graph.clear();

    std::string errorMsg;
    printCheck("loadPlaces(test_places.csv)", loadPlaces(graph, "test_places.csv", errorMsg));
    if (!errorMsg.empty()) {
        std::cout << "loadPlaces error: " << errorMsg << '\n';
    }
    errorMsg.clear();

    printCheck("loadRoads(test_roads.csv)", loadRoads(graph, "test_roads.csv", errorMsg));
    if (!errorMsg.empty()) {
        std::cout << "loadRoads error: " << errorMsg << '\n';
    }

    std::cout << "Places loaded from CSV:\n";
    for (const auto& place : graph.getAllPlaces()) {
        printPlace(&place);
    }
    std::cout << "Roads loaded from CSV, from P001:\n";
    printRoads(graph.getAdjacent("P001"));
    std::cout << "Roads loaded from CSV, from P002:\n";
    printRoads(graph.getAdjacent("P002"));

    errorMsg.clear();
    printCheck("savePlaces(saved_places.csv)", savePlaces(graph, "saved_places.csv", errorMsg));
    if (!errorMsg.empty()) {
        std::cout << "savePlaces error: " << errorMsg << '\n';
    }
    errorMsg.clear();

    printCheck("saveRoads(saved_roads.csv)", saveRoads(graph, "saved_roads.csv", errorMsg));
    if (!errorMsg.empty()) {
        std::cout << "saveRoads error: " << errorMsg << '\n';
    }

    std::cout << "\n[4] Reload saved files into a fresh graph\n";
    LGraph reloadedGraph;
    errorMsg.clear();
    printCheck("reload savePlaces output", loadPlaces(reloadedGraph, "saved_places.csv", errorMsg));
    if (!errorMsg.empty()) {
        std::cout << "reload places error: " << errorMsg << '\n';
    }
    errorMsg.clear();
    printCheck("reload saveRoads output", loadRoads(reloadedGraph, "saved_roads.csv", errorMsg));
    if (!errorMsg.empty()) {
        std::cout << "reload roads error: " << errorMsg << '\n';
    }

    std::cout << "Reloaded graph places:\n";
    for (const auto& place : reloadedGraph.getAllPlaces()) {
        printPlace(&place);
    }
    std::cout << "Reloaded graph roads from P001:\n";
    printRoads(reloadedGraph.getAdjacent("P001"));

    std::cout << "\nSmoke test finished." << std::endl;
    return 0;
}