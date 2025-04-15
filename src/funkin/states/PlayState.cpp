#include "PlayState.h"
#include <Engine.h>
#include <Input.h>
#include <iostream>
#include "../game/Song.h"
#include "../game/Conductor.h"

PlayState* PlayState::instance = nullptr;
SwagSong PlayState::SONG;

PlayState::PlayState() {
    instance = this; // haxeflixel reference
}

PlayState::~PlayState() {
    destroy();
}

void PlayState::create() {
    Engine* engine = Engine::getInstance();

    if (!SONG.validScore) {
        generateSong("bopeebo-hard");
        Log::getInstance().info("Song doesn't have a valid score lmao!");
    }

    generateSong(SONG.song);
}

void PlayState::update(float deltaTime) {
    if (!_subStates.empty()) {
        _subStates.back()->update(deltaTime);
    } 
    else {
        Input::UpdateKeyStates();
        Input::UpdateControllerStates();

        if (Input::justPressed(SDL_SCANCODE_RETURN) || Input::isControllerButtonJustPressed(SDL_CONTROLLER_BUTTON_START)) {
            if (_subStates.empty()) {
                PauseSubState* pauseSubState = new PauseSubState();
                openSubState(pauseSubState);
                Log::getInstance().info("Pause SubState opened");
            } else {
                closeSubState();
                Log::getInstance().info("Pause SubState closed");
            }
        }
    }
}

void PlayState::generateSong(std::string dataPath) {
    try {
        SONG = Song::loadFromJson(dataPath, dataPath);
        if (!SONG.validScore) {
            throw std::runtime_error("Failed to load song data");
        }
        
        Conductor::changeBPM(SONG.bpm);
        curSong = SONG.song;

        std::cout << "Generated song: " << curSong 
                  << " BPM: " << SONG.bpm 
                  << " Speed: " << SONG.speed << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error generating song: " << e.what() << std::endl;
    }
}

void PlayState::render() {
    if (!_subStates.empty()) {
        _subStates.back()->render();
    }
}

void PlayState::destroy() {
}

void PlayState::openSubState(SubState* subState) {
    std::cout << "PlayState::openSubState called" << std::endl;
    State::openSubState(subState);
}