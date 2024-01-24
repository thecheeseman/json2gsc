// SPDX-FileCopyrightText: 2023 thecheeseman
//
// SPDX-License-Identifier: BSD-3-Clause

#include <argh.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
namespace fs = std::filesystem;

// Helper for writing GSC
float pick_correct_vertex_component(const std::array<float, 6> &two_vertices, std::size_t i);

// global settings
struct Settings {
    std::string exe{};
    std::string inputFolder{};
    std::string outputFileName{};
    bool verbose{false};
} g_settings;

// individual map settings
struct MapSettings {
    std::string mapName{};
    std::string mapTitle{};
    std::string mapAuthor{"unknown"};
    int timeLimit{30};
    std::string difficulty{"unknown"};

    struct LadderJump {
        std::array<float, 3> from{};
        std::array<float, 3> to{};
        std::array<float, 3> angles{0, 0, 0};
        float distance{64};
    };

    std::vector<LadderJump> ladderJumps{};

    struct HealSpot {
        std::array<float, 3> origin{};
    };

    std::vector<HealSpot> healSpots{};

    struct GrenadeSpot {
        std::array<float, 3> origin{};
    };

    std::vector<GrenadeSpot> grenadeSpots{};

    struct PanzerSpot {
        std::array<float, 3> origin{};
    };

    std::vector<PanzerSpot> panzerSpots{};

    struct AABB {
        std::array<float, 6> two_vertices{};
    };

    std::vector<AABB> disableSaveSpots{};
    std::vector<AABB> enableSaveSpots{};
};

void from_json(const json& j, MapSettings& m);

// from_json overload for MapSettings
void from_json(const json& j, MapSettings& m)
{
    j.at("map").at("name").get_to(m.mapName);

    if (j.at("map").contains("title"))
        j.at("map").at("title").get_to(m.mapTitle);
    else
        m.mapTitle = m.mapName;

    if (j.at("map").contains("author"))
        j.at("map").at("author").get_to(m.mapAuthor);

    j.at("timelimit").get_to(m.timeLimit);
    j.at("difficulty").get_to(m.difficulty);

    if (j.contains("ladderjumps")) {
        for (const auto& jump : j.at("ladderjumps")) {
            MapSettings::LadderJump ladderJump;
            jump.at("from").get_to(ladderJump.from);
            jump.at("to").get_to(ladderJump.to);

            if (jump.contains("angles"))
                jump.at("angles").get_to(ladderJump.angles);

            if (jump.contains("distance"))
                jump.at("distance").get_to(ladderJump.distance);

            m.ladderJumps.push_back(ladderJump);
        }
    }

    if (j.contains("healspots")) {
        for (const auto& spot : j.at("healspots")) {
            if (spot.size() != 3)
                continue;

            MapSettings::HealSpot healSpot;
            spot.get_to(healSpot.origin);
            m.healSpots.push_back(healSpot);
        }
    }

    if (j.contains("grenadespots")) {
        for (const auto& spot : j.at("grenadespots")) {
            if (spot.size() != 3)
                continue;

            MapSettings::GrenadeSpot grenadeSpot;
            spot.get_to(grenadeSpot.origin);
            m.grenadeSpots.push_back(grenadeSpot);
        }
    }

    if (j.contains("panzerspots")) {
        for (const auto& spot : j.at("panzerspots")) {
            if (spot.size() != 3)
                continue;

            MapSettings::PanzerSpot panzerSpot;
            spot.get_to(panzerSpot.origin);
            m.panzerSpots.push_back(panzerSpot);
        }
    }

    if (j.contains("disablesavespots")) {
        for (const auto& spot : j.at("disablesavespots")) {
            if (spot.size() != 6)
                continue;

            MapSettings::AABB disableSaveSpot;
            spot.get_to(disableSaveSpot.two_vertices);
            m.disableSaveSpots.push_back(disableSaveSpot);
        }
    }

    if (j.contains("enablesavespots")) {
        for (const auto& spot : j.at("enablesavespots")) {
            if (spot.size() != 6)
                continue;

            MapSettings::AABB enableSaveSpot;
            spot.get_to(enableSaveSpot.two_vertices);
            m.enableSaveSpots.push_back(enableSaveSpot);
        }
    }
}

std::vector<MapSettings> g_maps;

// --------------------------------
// ParseFile
// --------------------------------
static std::pair<bool, std::string> ParseFile(const fs::path& path)
{
    if (g_settings.verbose)
        std::cout << "Parsing file: " << path << std::endl;

    try {
        std::ifstream file(path);
        const json data = json::parse(file);
        g_maps.push_back(data.get<MapSettings>());
    } catch (json::exception& e) {
        return {false, e.what()};
    } catch (std::exception& e) {
        return {false, e.what()};
    } catch (...) {
        return {false, "Unknown exception"};
    }

    return {true, ""};
}

// --------------------------------
// WriteGSC
// --------------------------------
static bool WriteGSC(const fs::path& path)
{
    std::ofstream file(path);
    if (!file)
        return false;

    file << "// Autogenerated by json2gsc\n"
        << "// DO NOT EDIT\n\n";

    //
    // set_level_difficulties
    //
    file << "set_level_difficulties()\n"
        << "{\n";

    for (const auto& map : g_maps)
        file << "    level.difficulty[\"" << map.mapName << "\"] = \"" << map.difficulty << "\";\n";

    file << "}\n\n";

    //
    // map_setup
    //
    file << "map_setup()\n"
        << "{\n"
        << "    mapname = toLower(level.mapname);\n"
        << "    switch(mapname) {\n";

    for (const auto& map : g_maps) {
        file << "        case \"" << map.mapName << "\":\n"
            << "            level.maptitle = \"" << map.mapTitle << "\";\n"
            << "            level.mapauthor = \"" << map.mapAuthor << "\";\n"
            << "            level.timelimit = " << map.timeLimit << ";\n";

        for (std::size_t i = 0; i < map.ladderJumps.size(); i++) {
            file << "            ladderjumps[" << i << "][\"from\"] = ("
                << map.ladderJumps[i].from[0] << ", "
                << map.ladderJumps[i].from[1] << ", "
                << map.ladderJumps[i].from[2] << ");\n"
                << "            ladderjumps[" << i << "][\"to\"] = ("
                << map.ladderJumps[i].to[0] << ", "
                << map.ladderJumps[i].to[1] << ", "
                << map.ladderJumps[i].to[2] << ");\n"
                << "            ladderjumps[" << i << "][\"angles\"] = ("
                << map.ladderJumps[i].angles[0] << ", "
                << map.ladderJumps[i].angles[1] << ", "
                << map.ladderJumps[i].angles[2] << ");\n"
                << "            ladderjumps[" << i << "][\"distance\"] = "
                << map.ladderJumps[i].distance << ";\n";
        }

        for (std::size_t i = 0; i < map.healSpots.size(); i++) {
            file << "            healspots[" << i << "] = ("
                << map.healSpots[i].origin[0] << ", "
                << map.healSpots[i].origin[1] << ", "
                << map.healSpots[i].origin[2] << ");\n";
        }

        for (std::size_t i = 0; i < map.grenadeSpots.size(); i++) {
            file << "            grenadespots[" << i << "] = ("
                << map.grenadeSpots[i].origin[0] << ", "
                << map.grenadeSpots[i].origin[1] << ", "
                << map.grenadeSpots[i].origin[2] << ");\n";
        }

        for (std::size_t i = 0; i < map.panzerSpots.size(); i++) {
            file << "            panzerspots[" << i << "] = ("
                << map.panzerSpots[i].origin[0] << ", "
                << map.panzerSpots[i].origin[1] << ", "
                << map.panzerSpots[i].origin[2] << ");\n";
        }

        for (std::size_t i = 0; i < map.disableSaveSpots.size(); i++) {
            file << "            save_disable_aabb[" << i << "][0] = ("
                << pick_correct_vertex_component(map.disableSaveSpots[i].two_vertices, 0) << ", "
                << pick_correct_vertex_component(map.disableSaveSpots[i].two_vertices, 1) << ", "
                << pick_correct_vertex_component(map.disableSaveSpots[i].two_vertices, 2) << ");\n"
                << "            save_disable_aabb[" << i << "][1] = ("
                << pick_correct_vertex_component(map.disableSaveSpots[i].two_vertices, 3) << ", "
                << pick_correct_vertex_component(map.disableSaveSpots[i].two_vertices, 4) << ", "
                << pick_correct_vertex_component(map.disableSaveSpots[i].two_vertices, 5) << ");\n";
        }

        for (std::size_t i = 0; i < map.enableSaveSpots.size(); i++) {
            file << "            save_enable_aabb[" << i << "][0] = ("
                << pick_correct_vertex_component(map.enableSaveSpots[i].two_vertices, 0) << ", "
                << pick_correct_vertex_component(map.enableSaveSpots[i].two_vertices, 1) << ", "
                << pick_correct_vertex_component(map.enableSaveSpots[i].two_vertices, 2) << ");\n"
                << "            save_enable_aabb[" << i << "][1] = ("
                << pick_correct_vertex_component(map.enableSaveSpots[i].two_vertices, 3) << ", "
                << pick_correct_vertex_component(map.enableSaveSpots[i].two_vertices, 4) << ", "
                << pick_correct_vertex_component(map.enableSaveSpots[i].two_vertices, 5) << ");\n";
        }

        file << "        break;\n";
    }

    file << "    }\n\n"
        << "    level.mapsettings = [];\n\n"
        << "    if(isDefined(ladderjumps))\n"
        << "        level.mapsettings[\"ladderjumps\"] = ladderjumps;\n"
        << "    if(isDefined(healspots))\n"
        << "        level.mapsettings[\"healspots\"] = healspots;\n"
        << "    if(isDefined(grenadespots))\n"
        << "        level.mapsettings[\"grenadespots\"] = grenadespots;\n"
        << "    if(isDefined(panzerspots))\n"
        << "        level.mapsettings[\"panzerspots\"] = panzerspots;\n"
        << "    if(isDefined(save_disable_aabb))\n"
        << "        level.mapsettings[\"save_disable_aabb\"] = save_disable_aabb;\n"
        << "    if(isDefined(save_enable_aabb))\n"
        << "        level.mapsettings[\"save_enable_aabb\"] = save_enable_aabb;\n\n"
        << "    thread maps\\mp\\gametypes\\jmp::mapfixes();\n"
        << "}\n\n"
        << "// Autogenerated by json2gsc\n"
        << "// DO NOT EDIT\n";

    return true;
}

float pick_correct_vertex_component(const std::array<float, 6> &two_vertices, std::size_t i) {
    if(i > 5) {
        return 0;
    }

    if(i < 3) {
        return two_vertices[i] < two_vertices[i+3] ? two_vertices[i] : two_vertices[i+3];
    } else {
        return two_vertices[i-3] > two_vertices[i] ? two_vertices[i-3] : two_vertices[i];
    }
}

// --------------------------------
// main
// --------------------------------
int main(const int argc, char* argv[])
{
    const argh::parser commandLine(argc, argv);
    commandLine(0) >> g_settings.exe;
    commandLine(1, "mapconfigs") >> g_settings.inputFolder;
    commandLine(2, "settings.gsc") >> g_settings.outputFileName;

    if (!fs::exists(g_settings.inputFolder)) {
        std::cerr << "Input folder does not exist: " << g_settings.inputFolder << std::endl;
        return 1;
    }

    std::vector<fs::path> files;
    for (const auto& entry : fs::directory_iterator(g_settings.inputFolder)) {
        if (entry.is_regular_file())
            files.push_back(entry.path());
    }

    if (files.empty()) {
        std::cerr << "No files found in input folder: " << g_settings.inputFolder << std::endl;
        return 2;
    }

    for (const auto& file : files) {
        if (auto [result, error] = ParseFile(file); !result)
            std::cerr << "WARNING: Failed to parse file " << file << ": " << error << std::endl;
    }

    if (g_maps.empty()) {
        std::cerr << "Unable to parse any files in input folder: " << g_settings.inputFolder << std::endl;
        return 3;
    }

    std::cout << "Successfully parsed " << g_maps.size() << "/" << files.size() << " files" << std::endl;
    std::ranges::sort(g_maps.begin(),
                      g_maps.end(),
                      [](const MapSettings& a, const MapSettings& b) { return a.mapName < b.mapName; });

    if (!WriteGSC(g_settings.outputFileName)) {
        std::cerr << "Failed to write output file: " << g_settings.outputFileName << std::endl;
        return 4;
    }

    return 0;
}
