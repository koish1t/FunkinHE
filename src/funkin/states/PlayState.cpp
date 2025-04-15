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
    Note::loadAssets();
    scoreText = new Text();
    scoreText->setFormat("assets/fonts/vcr.ttf", 32, 0xFFFFFFFF);
    
    int windowWidth = Engine::getInstance()->getWindowWidth();
    int windowHeight = Engine::getInstance()->getWindowHeight();
    scoreText->setPosition(windowWidth / 2 - 100, windowHeight - 50);
    updateScoreText();
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
    
    for (auto note : notes) {
        delete note;
    }
    notes.clear();
    
    delete scoreText;
    Note::unloadAssets();
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
    generateNotes();
    startingSong = true;
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
        updateArrowAnimations();

        for (auto arrow : strumLineNotes) {
            if (arrow) {
                arrow->update(deltaTime);
            }
        }

        Conductor::songPosition += deltaTime * 1000.0f;

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
                
                bool noteHit = false;
                for (auto note : notes) {
                    if (note && note->mustPress && !note->wasGoodHit && note->noteData == i && note->canBeHit) {
                        goodNoteHit(note);
                        noteHit = true;
                        break;
                    }
                }
                
                if (!noteHit) {
                    noteMiss(i);
                }
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
    } else {
        for (auto arrow : strumLineNotes) {
            if (arrow && arrow->isVisible()) {
                arrow->render();
            }
        }

        for (auto note : notes) {
            if (note && note->isVisible()) {
                note->render();
            }
        }

        scoreText->render();

        static int lastNoteCount = 0;
        if (notes.size() != lastNoteCount) {
            Log::getInstance().info("Active notes: " + std::to_string(notes.size()));
            lastNoteCount = notes.size();
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
                              ", mustHitSection=" + std::to_string(section.mustHitSection) +
                              ", notes count=" + std::to_string(section.sectionNotes.size()));

        for (const auto& noteData : section.sectionNotes) {
            if (noteData.size() >= 2) {
                float strumTime = noteData[0];
                int rawNoteType = static_cast<int>(noteData[1]);
                int noteType = rawNoteType % 4;
                
                bool mustPress;
                if (rawNoteType >= 4) {
                    mustPress = true;
                    noteType = noteType % 4;
                } else {
                    mustPress = section.mustHitSection;
                }

                Log::getInstance().info("Creating note in section " + std::to_string(currentSection) + 
                                     ": time=" + std::to_string(strumTime) + 
                                     ", type=" + std::to_string(noteType) + 
                                     ", rawType=" + std::to_string(rawNoteType) +
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
                
                float baseX = mustPress ? (Engine::getInstance()->getWindowWidth() * 0.75f) 
                                      : (Engine::getInstance()->getWindowWidth() * 0.25f);
                float arrowSpacing = 120.0f;
                float totalWidth = arrowSpacing * 3;
                float xOffset = baseX - (totalWidth * 0.5f) + (noteType * arrowSpacing);
                note->setPosition(xOffset, 0);
                
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
    std::string text = "Score: " + std::to_string(score) + " Misses: " + std::to_string(misses);
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