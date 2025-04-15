#include "PlayState.h"
#include <Engine.h>
#include <Input.h>
#include <iostream>
#include <algorithm>
#include "../game/Song.h"
#include "../game/Conductor.h"
#include <SDLManager.h>

PlayState* PlayState::instance = nullptr;
SwagSong PlayState::SONG;
Sound* PlayState::inst = nullptr;

PlayState::PlayState() {
    instance = this;
    inst = nullptr;
    vocals = nullptr;
}

PlayState::~PlayState() {
    if (vocals != nullptr) {
        delete vocals;
        vocals = nullptr;
    }
    if (inst != nullptr) {
        delete inst;
        inst = nullptr;
    }
    
    for (auto arrow : strumLineNotes) {
        delete arrow;
    }
    strumLineNotes.clear();
    
    destroy();
}

void PlayState::create() {
    Engine* engine = Engine::getInstance();

    if (!SONG.validScore) {
        generateSong("bopeebo-hard");
        Log::getInstance().info("Song doesn't have a valid score lmao!");
    }

    startCountdown();
    generateSong(SONG.song);
    startingSong = true;
}

void PlayState::update(float deltaTime) {
    if (!_subStates.empty()) {
        _subStates.back()->update(deltaTime);
    } 
    else {
        Input::UpdateKeyStates();
        Input::UpdateControllerStates();

        handleInput();
        updateArrowAnimations();

        for (auto arrow : strumLineNotes) {
            if (arrow) {
                arrow->update(deltaTime);
            }
        }

        if (startingSong) {
            if (startedCountdown) {
                Conductor::songPosition += deltaTime * 1000;
                if (Conductor::songPosition >= 0) {
                    startSong();
                }
            }
        }

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

void PlayState::handleInput() {
    for (int i = 0; i < 4; i++) {
        int arrowIndex = i + 4;
        if (arrowIndex < strumLineNotes.size() && strumLineNotes[arrowIndex]) {
            if (isKeyJustPressed(i)) {
                strumLineNotes[arrowIndex]->playAnimation("pressed");
            }
            else if (isKeyJustReleased(i)) {
                strumLineNotes[arrowIndex]->playAnimation("static");
            }
        }
    }
}

void PlayState::updateArrowAnimations() {
    for (size_t i = 4; i < strumLineNotes.size(); i++) {
        auto arrow = strumLineNotes[i];
        if (arrow) {
            int keyIndex = (i - 4) % 4;
            if (arrow->getCurrentAnimation() == "pressed" && !isKeyPressed(keyIndex)) {
                arrow->playAnimation("static");
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

        if (vocals != nullptr) {
            delete vocals;
            vocals = nullptr;
        }
        if (inst != nullptr) {
            delete inst;
            inst = nullptr;
        }

        if (SONG.needsVoices) {
            std::string vocalsPath = "assets/songs/" + curSong + "/Voices" + soundExt;
            vocals = new Sound();
            if (!vocals->load(vocalsPath)) {
                Log::getInstance().error("Failed to load vocals: " + vocalsPath);
                delete vocals;
                vocals = nullptr;
            }
        }

        std::string instPath = "assets/songs/" + curSong + "/Inst" + soundExt;
        inst = new Sound();
        if (!inst->load(instPath)) {
            Log::getInstance().error("Failed to load instrumentals: " + instPath);
            delete inst;
            inst = nullptr;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error generating song: " << e.what() << std::endl;
    }
}

void PlayState::startSong() {
    startingSong = false;
    if (vocals != nullptr) {
        vocals->play();
    }
    if (inst != nullptr) {
        inst->play();
    }
}

void PlayState::startCountdown() {
    startedCountdown = true;

    generateStaticArrows(0);
    generateStaticArrows(1);
}

void PlayState::render() {
    if (!_subStates.empty()) {
        _subStates.back()->render();
    }

    for (auto arrow : strumLineNotes) {
        if (arrow && arrow->isVisible()) {
            arrow->render();
        }
    }
}

void PlayState::generateStaticArrows(int player) {
    int windowWidth, windowHeight;
    SDL_GetWindowSize(SDLManager::getInstance().getWindow(), &windowWidth, &windowHeight);
    
    float startX = (player == 1) ? (windowWidth * 0.75f) : (windowWidth * 0.25f);
    float yPos = 50.0f;
    
    float arrowSpacing = 120.0f;
    float totalWidth = arrowSpacing * 3;
    float xOffset = startX - (totalWidth * 0.5f);
        
    for (int i = 0; i < 4; i++) {
        AnimatedSprite* babyArrow = new AnimatedSprite();
        
        babyArrow->loadFrames("assets/images/NOTE_assets.png", "assets/images/NOTE_assets.xml");

        std::string staticFrame = NOTE_STYLES[i] + NOTE_DIRS[i] + "0000";
        
        std::string lowerDir = NOTE_DIRS[i];
        std::transform(lowerDir.begin(), lowerDir.end(), lowerDir.begin(), ::tolower);
        std::string pressPrefix = lowerDir + " press";
        std::string confirmPrefix = lowerDir + " confirm";

        babyArrow->addAnimation("static", staticFrame, 24, false);
        babyArrow->addAnimation("pressed", pressPrefix, 24, false);
        babyArrow->addAnimation("confirm", confirmPrefix, 24, false);

        babyArrow->playAnimation("static");
        babyArrow->setPosition(xOffset, yPos);
        
        babyArrow->setScale(0.7f, 0.7f);

        strumLineNotes.push_back(babyArrow);
        babyArrow->setVisible(true);
        
        xOffset += arrowSpacing;
    }
}

void PlayState::destroy() {
}

void PlayState::openSubState(SubState* subState) {
    std::cout << "PlayState::openSubState called" << std::endl;
    State::openSubState(subState);
}