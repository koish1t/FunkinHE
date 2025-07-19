#include "PlayState.h"
#include <iostream>
#include <algorithm>
#include "../game/Song.h"
#include "../game/Conductor.h"
#include <fstream>
#include <map>
#ifdef __SWITCH__ 
#include "../backend/json.hpp"
#elif defined(__MINGW32__)
#include "../backend/json.hpp"
#else
#include "../backend/json.hpp"
#endif

PlayState* PlayState::instance = nullptr;
SwagSong PlayState::SONG;
Sound* PlayState::inst = nullptr;

PlayState::PlayState() {
    instance = this;
    inst = nullptr;
    vocals = nullptr;
    Note::loadAssets();
    scoreText = new Text();
    scoreText->setFormat("assets/fonts/vcr.ttf", 32, 0xFFFFFFFF);
    
    int windowWidth = Engine::getInstance()->getWindowWidth();
    int windowHeight = Engine::getInstance()->getWindowHeight();
    scoreText->setPosition(windowWidth / 2 - 100, windowHeight - 50);
    updateScoreText();

    loadKeybinds();
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
    
    if (currentStage) {
        delete currentStage;
        currentStage = nullptr;
    }
    
    if (camGame) {
        delete camGame;
        camGame = nullptr;
    }
    
    if (camHUD) {
        delete camHUD;
        camHUD = nullptr;
    }
    
    for (auto arrow : strumLineNotes) {
        delete arrow;
    }
    strumLineNotes.clear();
    
    for (auto note : notes) {
        delete note;
    }
    notes.clear();
    
    delete scoreText;
    Note::unloadAssets();
    destroy();
}

void PlayState::loadSongConfig() {
    std::ifstream configFile("assets/data/config.json");
    if (!configFile.is_open()) {
        Log::getInstance().error("Failed to open config.json");
        return;
    }

    try {
        nlohmann::json config;
        configFile >> config;

        if (config.contains("songConfig")) {
            auto songConfig = config["songConfig"];
            std::string songName = songConfig["songName"].get<std::string>();
            std::string difficulty = songConfig["difficulty"].get<std::string>();
            
            std::string fullSongName = difficulty.empty() ? songName : songName + "-" + difficulty;
            generateSong(fullSongName);
        } else {
            Log::getInstance().error("No songConfig found in config.json");
        }
    } catch (const std::exception& ex) {
        Log::getInstance().error("Failed to parse song config: " + std::string(ex.what()));
    }
}

void PlayState::loadStage() {
    if (currentStage) {
        delete currentStage;
        currentStage = nullptr;
    }

    std::string stageName = "stage";
    
    /*
    if (curSong.find("swag") != std::string::npos) {
        stageName = "swagstage";
    }
        */
    
    try {
        currentStage = new Stage(stageName);
        if (!currentStage->isStageLoaded()) {
            Log::getInstance().warning("Failed to load stage: " + stageName + ", using default stage");
            delete currentStage;
            currentStage = new Stage("stage");
        }
        
        Log::getInstance().info("Loaded stage: " + stageName);
        
        if (camGame) {
            currentStage->setCamera(camGame);
        }
    } catch (const std::exception& e) {
        Log::getInstance().error("Error loading stage: " + std::string(e.what()));
        currentStage = new Stage("stage");
        if (camGame) {
            currentStage->setCamera(camGame);
        }
    }
}

void PlayState::updateCameraZoom() {
    if (camGame && currentStage) {
        float targetZoom = currentStage->getDefaultZoom();
        if (camGame->getZoom() != targetZoom) {
            camGame->setZoom(targetZoom);
        }
    }
}

void PlayState::setupHUDCamera() {
    if (camHUD && scoreText) {
        scoreText->setCamera(camHUD);
    }
}

void PlayState::create() {
    Mix_HaltChannel(-1);
    Engine::getInstance()->getSoundManager().stopMusic();
    Conductor::songPosition = 0;
    startingSong = true;
    startedCountdown = false;
    Engine* engine = Engine::getInstance();
    camGame = new Camera();
    camHUD = new Camera();
    camHUD->setZoom(1.0f);
    
    setupHUDCamera();
    
    loadSongConfig();
    loadStage();
    startCountdown();
    generateNotes();
    #ifdef __SWITCH__
    // nun
    #elif defined(__MINGW32__)
    // nun
    #else
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "In Game - %s", curSong.c_str());
    Discord::GetInstance().SetState(buffer);
    Discord::GetInstance().Update();
    #endif
}

void PlayState::update(float deltaTime) {
    FunkinState::update(deltaTime);

    if (!_subStates.empty()) {
        _subStates.back()->update(deltaTime);
    } 
    else {
        Input::UpdateKeyStates();
        Input::UpdateControllerStates();

        handleInput();
        handleOpponentNoteHit(deltaTime);
        updateArrowAnimations();

        for (auto arrow : strumLineNotes) {
            if (arrow) {
                arrow->update(deltaTime);
            }
        }

        if (!startingSong && musicStartTicks > 0) {
            Conductor::songPosition = static_cast<float>(SDL_GetTicks() - musicStartTicks);
        }

        while (!unspawnNotes.empty()) {
            Note* nextNote = unspawnNotes[0];
            if (nextNote->strumTime - Conductor::songPosition > 1500) {
                break;
            }
            
            notes.push_back(nextNote);
            unspawnNotes.erase(unspawnNotes.begin());
        }

        for (auto it = notes.begin(); it != notes.end();) {
            Note* note = *it;
            if (note) {
                note->update(deltaTime);

                if (note->mustPress && note->tooLate && !note->wasGoodHit) {
                    noteMiss(note->noteData);
                    note->kill = true;
                }

                if (note->kill || note->strumTime < Conductor::songPosition - 5000) {
                    it = notes.erase(it);
                } else {
                    ++it;
                }
            } else {
                ++it;
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

        if (currentStage) {
            currentStage->update(deltaTime);
        }
        
        updateCameraZoom();

        if (pauseCooldown > 0) {
            pauseCooldown -= deltaTime;
        }

        if (Input::justPressed(SDL_SCANCODE_RETURN) || Input::isControllerButtonJustPressed(SDL_CONTROLLER_BUTTON_START)) {
            if (_subStates.empty()) {
                if (pauseCooldown <= 0) {
                    PauseSubState* pauseSubState = new PauseSubState();
                    openSubState(pauseSubState);
                    Log::getInstance().info("Pause SubState opened");
                    pauseCooldown = 0.5f;
                }
            } else {
                closeSubState();
                Log::getInstance().info("Pause SubState closed");
            }
        }
    }
}

void PlayState::handleInput() {
    for (size_t i = 0; i < 4; i++) {
        size_t arrowIndex = i + 4;
        if (arrowIndex < strumLineNotes.size() && strumLineNotes[arrowIndex]) {
            if (isKeyJustPressed(static_cast<int>(i)) || isNXButtonJustPressed(static_cast<int>(i))) {
                strumLineNotes[arrowIndex]->playAnimation("pressed");
                
                bool noteHit = false;
                for (auto note : notes) {
                    if (note && note->mustPress && !note->wasGoodHit && note->noteData == static_cast<int>(i) && note->canBeHit) {
                        goodNoteHit(note);
                        noteHit = true;
                        break;
                    }
                }
                
                if (!noteHit) {
                    if (GameConfig::getInstance()->isGhostTapping()) {
                        // do nun lol
                    } else {
                        noteMiss(static_cast<int>(i));
                    }
                }
            }
            else if (isKeyJustReleased(static_cast<int>(i)) || isNXButtonJustReleased(static_cast<int>(i))) {
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
        std::string songName = dataPath;
        std::string folder = dataPath;
        std::string baseSongName = dataPath;
        
        if (folder.length() >= 5 && folder.substr(folder.length() - 5) == "-easy" ||
            folder.length() >= 5 && folder.substr(folder.length() - 5) == "-hard") {
            size_t dashPos = folder.rfind("-");
            if (dashPos != std::string::npos) {
                folder = folder.substr(0, dashPos);
                baseSongName = folder;
            }
        }
        
        SONG = Song::loadFromJson(songName, folder);
        if (!SONG.validScore) {
            Log::getInstance().error("Failed to load song data");
            return;
        }
        
        Conductor::changeBPM(SONG.bpm);
        curSong = songName;

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
            std::string vocalsPath = "assets/songs/" + baseSongName + "/Voices" + soundExt;
            vocals = new Sound();
            if (!vocals->load(vocalsPath)) {
                Log::getInstance().error("Failed to load vocals: " + vocalsPath);
                delete vocals;
                vocals = nullptr;
            }
        }

        std::string instPath = "assets/songs/" + baseSongName + "/Inst" + soundExt;
        inst = new Sound();
        if (!inst->load(instPath)) {
            Log::getInstance().error("Failed to load instrumentals: " + instPath);
            delete inst;
            inst = nullptr;
        }
        
    } catch (const std::exception& ex) {
        std::cerr << "Error generating song: " << ex.what() << std::endl;
        Log::getInstance().error("Error generating song: " + std::string(ex.what()));
    }
}

void PlayState::startSong() {
    startingSong = false;
    musicStartTicks = SDL_GetTicks();
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
    if (camGame) {
        camGame->begin();
    }

    if (currentStage) {
        currentStage->render();
    }

    for (auto note : notes) {
        if (note && note->isVisible()) {
            note->render();
        }
    }

    if (camGame) {
        camGame->end();
    }

    if (camHUD) {
        camHUD->begin();
    }
    
    for (auto arrow : strumLineNotes) {
        if (arrow && arrow->isVisible()) {
            arrow->render();
        }
    }
    
    scoreText->render();
    
    if (camHUD) {
        camHUD->end();
    }

    static int lastNoteCount = 0;
    if (notes.size() != lastNoteCount) {
        Log::getInstance().info("Active notes: " + std::to_string(notes.size()));
        lastNoteCount = notes.size();
    }

    if (!_subStates.empty()) {
        _subStates.back()->render();
    }
}

void PlayState::generateStaticArrows(int player) {
    int windowWidth, windowHeight;
    SDL_GetWindowSize(SDLManager::getInstance().getWindow(), &windowWidth, &windowHeight);
    
    float cameraZoom = 1.0f;
    if (currentStage) {
        cameraZoom = currentStage->getDefaultZoom();
    }
    
    float startX = (player == 1) ? (windowWidth * 0.75f) : (windowWidth * 0.25f);
    float yPos = GameConfig::getInstance()->isDownscroll() ? (windowHeight - 150.0f) : 50.0f;
    
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
    
        if (camHUD) {
            babyArrow->setCamera(camHUD);
        }

        strumLineNotes.push_back(babyArrow);
        babyArrow->setVisible(true);
        
        xOffset += arrowSpacing;
    }
}

void PlayState::generateNotes() {
    unspawnNotes.clear();
    notes.clear();

    Log::getInstance().info("Generating notes from " + std::to_string(SONG.notes.size()) + " sections");

    int totalNotes = 0;
    int currentSection = 0;
    
    for (const auto& section : SONG.notes) {
        Log::getInstance().info("Section " + std::to_string(currentSection) + 
                              " has " + std::to_string(section.sectionNotes.size()) + " notes, " +
                              "mustHitSection=" + std::to_string(section.mustHitSection));
        currentSection++;
    }

    currentSection = 0;
    for (const auto& section : SONG.notes) {
        Log::getInstance().info("Processing section " + std::to_string(currentSection) + 
                              ", mustHitSection=" + std::to_string(section.mustHitSection));

        for (const auto& noteData : section.sectionNotes) {
            if (noteData.size() >= 2) {
                float strumTime = noteData[0];
                int noteType = static_cast<int>(noteData[1]);
                
                bool mustPress = section.mustHitSection;
                if (noteType >= 4) {
                    mustPress = !section.mustHitSection;
                    noteType = noteType % 4;
                }

                Log::getInstance().info("Creating note in section " + std::to_string(currentSection) + 
                                     ": time=" + std::to_string(strumTime) + 
                                     ", type=" + std::to_string(noteType) + 
                                     ", mustPress=" + std::to_string(mustPress));

                bool sustainNote = noteData.size() > 3 && noteData[3] > 0;
                float sustainLength = sustainNote ? noteData[3] : 0;

                Note* prevNote = nullptr;
                if (sustainNote && !unspawnNotes.empty()) {
                    prevNote = unspawnNotes.back();
                }

                Note* note = new Note(strumTime, noteType, prevNote, sustainNote);
                note->mustPress = mustPress;
                note->sustainLength = sustainLength;
                
                float cameraZoom = 1.0f;
                if (currentStage) {
                    cameraZoom = currentStage->getDefaultZoom();
                }
                
                float baseX = mustPress ? (Engine::getInstance()->getWindowWidth() * 0.75f) 
                                      : (Engine::getInstance()->getWindowWidth() * 0.25f);
                float arrowSpacing = 120.0f;
                float totalWidth = arrowSpacing * 3;
                float xOffset = baseX - (totalWidth * 0.5f) + (noteType * arrowSpacing);
                note->setPosition(xOffset, 0);
                
                if (camGame) {
                    note->setCamera(camGame);
                }
                
                unspawnNotes.push_back(note);
                totalNotes++;
            }
        }
        currentSection++;
    }

    Log::getInstance().info("Generated " + std::to_string(totalNotes) + " total notes from " + 
                          std::to_string(currentSection) + " sections");

    std::sort(unspawnNotes.begin(), unspawnNotes.end(),
        [](Note* a, Note* b) {
            return a->strumTime < b->strumTime;
        });

    int count = 0;
    for (const auto& note : unspawnNotes) {
        if (count >= 10) break;
        Log::getInstance().info("Sorted note " + std::to_string(count) + 
                              ": time=" + std::to_string(note->strumTime) + 
                              ", type=" + std::to_string(note->noteData) + 
                              ", mustPress=" + std::to_string(note->mustPress));
        count++;
    }
}

void PlayState::destroy() {
}

void PlayState::openSubState(SubState* subState) {
    std::cout << "PlayState::openSubState called" << std::endl;
    State::openSubState(subState);
}

void PlayState::updateScoreText() {
    std::string text = "Score: " + std::to_string(score);
    scoreText->setText(text);
}

void PlayState::goodNoteHit(Note* note) {
    if (!note->wasGoodHit) {
        note->wasGoodHit = true;
        
        if (note->noteData >= 0 && note->noteData < 4) {
            int arrowIndex = note->noteData + 4;
            if (arrowIndex < strumLineNotes.size() && strumLineNotes[arrowIndex]) {
                float currentX = strumLineNotes[arrowIndex]->getX();
                float currentY = strumLineNotes[arrowIndex]->getY();
                
                strumLineNotes[arrowIndex]->playAnimation("confirm");
                
                strumLineNotes[arrowIndex]->setPosition(currentX, currentY);
            }
        }

        combo++;
        score += 350;
        updateScoreText();
        
        note->kill = true;
    }
}

void PlayState::noteMiss(int direction) {
    combo = 0;
    misses++;
    score -= 10;
    if (score < 0) score = 0;
    updateScoreText();
}

void PlayState::loadKeybinds() {
    std::ifstream configFile("assets/data/config.json");
    if (!configFile.is_open()) {
        Log::getInstance().error("Failed to open config.json");
        return;
    }

    nlohmann::json config;
    try {
        configFile >> config;
    } catch (const std::exception& ex) {
        Log::getInstance().error("Failed to parse config.json: " + std::string(ex.what()));
        return;
    }

    if (config.contains("mainBinds")) {
        auto mainBinds = config["mainBinds"];
        arrowKeys[0].primary = getScancodeFromString(mainBinds["left"].get<std::string>());
        arrowKeys[1].primary = getScancodeFromString(mainBinds["down"].get<std::string>());
        arrowKeys[2].primary = getScancodeFromString(mainBinds["up"].get<std::string>());
        arrowKeys[3].primary = getScancodeFromString(mainBinds["right"].get<std::string>());
    }

    if (config.contains("altBinds")) {
        auto altBinds = config["altBinds"];
        arrowKeys[0].alternate = getScancodeFromString(altBinds["left"].get<std::string>());
        arrowKeys[1].alternate = getScancodeFromString(altBinds["down"].get<std::string>());
        arrowKeys[2].alternate = getScancodeFromString(altBinds["up"].get<std::string>());
        arrowKeys[3].alternate = getScancodeFromString(altBinds["right"].get<std::string>());
    }

    if (config.contains("nxBinds")) {
        auto nxBinds = config["nxBinds"];
        nxArrowKeys[0].primary = getButtonFromString(nxBinds["left"].get<std::string>());
        nxArrowKeys[1].primary = getButtonFromString(nxBinds["down"].get<std::string>());
        nxArrowKeys[2].primary = getButtonFromString(nxBinds["up"].get<std::string>());
        nxArrowKeys[3].primary = getButtonFromString(nxBinds["right"].get<std::string>());
    }

    if (config.contains("nxAltBinds")) {
        auto nxAltBinds = config["nxAltBinds"];
        nxArrowKeys[0].alternate = getButtonFromString(nxAltBinds["left"].get<std::string>());
        nxArrowKeys[1].alternate = getButtonFromString(nxAltBinds["down"].get<std::string>());
        nxArrowKeys[2].alternate = getButtonFromString(nxAltBinds["up"].get<std::string>());
        nxArrowKeys[3].alternate = getButtonFromString(nxAltBinds["right"].get<std::string>());
    }
}

SDL_Scancode PlayState::getScancodeFromString(const std::string& keyName) {
    static const std::map<std::string, SDL_Scancode> keyMap = {
        {"A", SDL_SCANCODE_A},
        {"B", SDL_SCANCODE_B},
        {"C", SDL_SCANCODE_C},
        {"D", SDL_SCANCODE_D},
        {"E", SDL_SCANCODE_E},
        {"F", SDL_SCANCODE_F},
        {"G", SDL_SCANCODE_G},
        {"H", SDL_SCANCODE_H},
        {"I", SDL_SCANCODE_I},
        {"J", SDL_SCANCODE_J},
        {"K", SDL_SCANCODE_K},
        {"L", SDL_SCANCODE_L},
        {"M", SDL_SCANCODE_M},
        {"N", SDL_SCANCODE_N},
        {"O", SDL_SCANCODE_O},
        {"P", SDL_SCANCODE_P},
        {"Q", SDL_SCANCODE_Q},
        {"R", SDL_SCANCODE_R},
        {"S", SDL_SCANCODE_S},
        {"T", SDL_SCANCODE_T},
        {"U", SDL_SCANCODE_U},
        {"V", SDL_SCANCODE_V},
        {"W", SDL_SCANCODE_W},
        {"X", SDL_SCANCODE_X},
        {"Y", SDL_SCANCODE_Y},
        {"Z", SDL_SCANCODE_Z},
        {"ArrowLeft", SDL_SCANCODE_LEFT},
        {"ArrowRight", SDL_SCANCODE_RIGHT},
        {"ArrowUp", SDL_SCANCODE_UP},
        {"ArrowDown", SDL_SCANCODE_DOWN},
        {"Space", SDL_SCANCODE_SPACE},
        {"Enter", SDL_SCANCODE_RETURN},
        {"Escape", SDL_SCANCODE_ESCAPE},
        {"Left", SDL_SCANCODE_LEFT},
        {"Right", SDL_SCANCODE_RIGHT},
        {"Up", SDL_SCANCODE_UP},
        {"Down", SDL_SCANCODE_DOWN}
    };

    auto it = keyMap.find(keyName);
    if (it != keyMap.end()) {
        return it->second;
    }

    Log::getInstance().warning("Unknown key name: " + keyName);
    return SDL_SCANCODE_UNKNOWN;
}

SDL_GameControllerButton PlayState::getButtonFromString(const std::string& buttonName) {
    static const std::map<std::string, SDL_GameControllerButton> buttonMap = {
        {"A", SDL_CONTROLLER_BUTTON_A},
        {"B", SDL_CONTROLLER_BUTTON_B},
        {"X", SDL_CONTROLLER_BUTTON_X},
        {"Y", SDL_CONTROLLER_BUTTON_Y},
        {"DPAD_LEFT", SDL_CONTROLLER_BUTTON_DPAD_LEFT},
        {"DPAD_RIGHT", SDL_CONTROLLER_BUTTON_DPAD_RIGHT},
        {"DPAD_UP", SDL_CONTROLLER_BUTTON_DPAD_UP},
        {"DPAD_DOWN", SDL_CONTROLLER_BUTTON_DPAD_DOWN},
        {"LEFT_SHOULDER", SDL_CONTROLLER_BUTTON_LEFTSHOULDER},
        {"RIGHT_SHOULDER", SDL_CONTROLLER_BUTTON_RIGHTSHOULDER},
        {"LEFT_TRIGGER", SDL_CONTROLLER_BUTTON_LEFTSHOULDER},
        {"RIGHT_TRIGGER", SDL_CONTROLLER_BUTTON_RIGHTSHOULDER},
        {"ZL", SDL_CONTROLLER_BUTTON_LEFTSHOULDER},
        {"ZR", SDL_CONTROLLER_BUTTON_RIGHTSHOULDER},
        {"LT", SDL_CONTROLLER_BUTTON_LEFTSHOULDER},
        {"RT", SDL_CONTROLLER_BUTTON_RIGHTSHOULDER},
        {"LEFT_STICK", SDL_CONTROLLER_BUTTON_LEFTSTICK},
        {"RIGHT_STICK", SDL_CONTROLLER_BUTTON_RIGHTSTICK},
        {"START", SDL_CONTROLLER_BUTTON_START},
        {"BACK", SDL_CONTROLLER_BUTTON_BACK}
    };

    auto it = buttonMap.find(buttonName);
    if (it != buttonMap.end()) {
        return it->second;
    }

    Log::getInstance().warning("Unknown button name: " + buttonName);
    return SDL_CONTROLLER_BUTTON_INVALID;
}

void PlayState::handleOpponentNoteHit(float deltaTime) {
    static float animationTimer = 0.0f;
    static bool isAnimating = false;
    static int currentArrowIndex = -1;

    for (auto note : notes) {
        if (note && !note->mustPress && !note->wasGoodHit) {
            float timeDiff = note->strumTime - Conductor::songPosition;
            
            if (timeDiff <= 45.0f && timeDiff >= -Conductor::safeZoneOffset) {
                note->canBeHit = true;
                
                int arrowIndex = note->noteData;
                if (arrowIndex < strumLineNotes.size() && strumLineNotes[arrowIndex]) {
                    strumLineNotes[arrowIndex]->playAnimation("confirm");
                    isAnimating = true;
                    currentArrowIndex = arrowIndex;
                    animationTimer = 0.0f;
                }
                
                note->wasGoodHit = true;
                note->kill = true;
            }
        }
    }

    if (isAnimating) {
        animationTimer += deltaTime;
        if (animationTimer >= 0.1f) {
            if (currentArrowIndex >= 0 && currentArrowIndex < strumLineNotes.size() && strumLineNotes[currentArrowIndex]) {
                strumLineNotes[currentArrowIndex]->playAnimation("static");
            }
            isAnimating = false;
            currentArrowIndex = -1;
        }
    }
}