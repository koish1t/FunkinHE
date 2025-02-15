#include "Song.h"
#include "../backend/json.hpp"
#include <fstream>
#include <iostream>
#include <filesystem>

using json = nlohmann::json;

Song::Song(const std::string& song, const std::vector<SwagSection>& notes, int bpm)
    : song(song), notes(notes), bpm(bpm) {
}

SwagSong Song::loadFromJson(const std::string& jsonInput, const std::string& folder) {
    std::string path = "assets/data/";
    if (!folder.empty()) {
        path += folder + "/";
    }
    path += jsonInput + ".json";

    std::string rawJson;
    try {
        std::ifstream file(path);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open file: " + path);
        }
        
        rawJson = std::string(
            (std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>()
        );
        file.close();
    } catch (const std::exception& e) {
        std::cerr << "Error loading song file: " << e.what() << std::endl;
        return SwagSong(); // should return empty on error
    }

    while (!rawJson.empty() && std::isspace(rawJson.back())) {
        rawJson.pop_back();
    }

    while (!rawJson.empty() && rawJson.back() != '}') {
        rawJson.pop_back();
    }

    return parseJSONshit(rawJson);
}

SwagSong Song::parseJSONshit(const std::string& rawJson) {
    SwagSong swagShit;
    try {
        json j = json::parse(rawJson);
        json songData = j["song"];

        swagShit.song = songData["song"].get<std::string>();
        swagShit.bpm = static_cast<int>(songData["bpm"].get<float>());
        swagShit.needsVoices = songData["needsVoices"].get<bool>();
        swagShit.speed = songData["speed"].get<float>();
        swagShit.player1 = songData["player1"].get<std::string>();
        swagShit.player2 = songData["player2"].get<std::string>();
        
        for (const auto& noteJson : songData["notes"]) {
            SwagSection section;
            section.lengthInSteps = noteJson["lengthInSteps"].get<int>();
            section.mustHitSection = noteJson.value("mustHitSection", true);
            
            section.typeOfSection = noteJson.value("typeOfSection", 0);
            section.bpm = noteJson.value("bpm", 0);
            section.changeBPM = noteJson.value("changeBPM", false);
            section.altAnim = noteJson.value("altAnim", false);
            
            if (noteJson.contains("sectionNotes")) {
                for (const auto& noteArray : noteJson["sectionNotes"]) {
                    std::vector<float> note;
                    for (const auto& value : noteArray) {
                        if (value.is_number()) {
                            note.push_back(value.get<float>());
                        }
                    }
                    section.sectionNotes.push_back(note);
                }
            }
            
            swagShit.notes.push_back(section);
        }

        swagShit.validScore = true;
    } catch (const json::exception& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
        return SwagSong(); // Return empty song on error
    }

    return swagShit;
} 