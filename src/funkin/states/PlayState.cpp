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

void PlayState::keyPressed(unsigned char key, int x, int y) {
    if (key == 'p') {
        if (instance->_subStates.empty()) {
            PauseSubState* pauseSubState = new PauseSubState();
            instance->openSubState(pauseSubState);
        } else {
            instance->closeSubState();
        }
    }
}

void PlayState::specialKeyPressed(int key, int x, int y) {
    std::cout << "Special key pressed: " << key << std::endl;
    
    int mappedKey;
    switch(key) {
        case GLUT_KEY_UP:    mappedKey = 128; break;
        case GLUT_KEY_DOWN:  mappedKey = 129; break;
        case GLUT_KEY_LEFT:  mappedKey = 130; break;
        case GLUT_KEY_RIGHT: mappedKey = 131; break;
        default: return;
    }
    
    Input::handleKeyPress(mappedKey);
}
