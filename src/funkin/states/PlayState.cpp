#include <states/PlayState.h>
#include <Engine.h>
#include <Input.h>
#include <substates/PauseSubState.h>
#include <iostream>
#include "../game/Song.h"
#include "../game/Conductor.h"

PlayState* PlayState::instance = nullptr;

PlayState::PlayState() : backgroundSprite(nullptr), playerSprite(nullptr) {
    instance = this; // haxeflixel reference
}

PlayState::~PlayState() {
    destroy();
}

void PlayState::create() {
    Engine* engine = Engine::getInstance();

    try {
        SwagSong songData = Song::loadFromJson("bopeebo-hard", "bopeebo");
        if (!songData.validScore) {
            throw std::runtime_error("Failed to load song data");
        }
        
        Conductor::changeBPM(songData.bpm);
        Conductor::mapBPMChanges(songData);

        std::cout << "Loaded song: " << songData.song 
                  << " BPM: " << songData.bpm 
                  << " Speed: " << songData.speed << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error loading song: " << e.what() << std::endl;
    }
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