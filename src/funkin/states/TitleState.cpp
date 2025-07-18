#include "TitleState.h"
#include "../../engine/core/Engine.h"
#include "../../engine/input/Input.h"
#include "../../engine/utils/Paths.h"
#include "../../engine/audio/SoundManager.h"
#include <SDL2/SDL.h>
#include <thread>
#include <chrono>
#include "../states/MainMenuState.h"

TitleState::TitleState() {}
TitleState::~TitleState() { destroy(); }

void TitleState::create() {
    Conductor::changeBPM(102);
    skippedIntro = false;
    f = false;
    whiteAlpha = 0.0f;
    Engine::getInstance()->getSoundManager().playMusic(Paths::music("freakymenu"));
    confirm = Engine::getInstance()->getSoundManager().loadSound(Paths::sound("confirmMenu"));
    gf = new AnimatedSprite();
    gf->loadFrames(Paths::image("gfDanceTitle"), Paths::xml("images/gfDanceTitle"));
    gf->addAnimation("gfDance", "gfDance", 24, true);
    gf->playAnimation("gfDance");
    gf->setPosition(Engine::getInstance()->getWindowWidth() * 0.4f, Engine::getInstance()->getWindowHeight() * 0.07f);
    gf->setAlpha(0);
    Engine::getInstance()->addAnimatedSprite(gf);

    logo = new AnimatedSprite();
    logo->loadFrames(Paths::image("logoBumpin"), Paths::xml("images/logoBumpin"));
    logo->addAnimation("logo bumpin", "logo bumpin", 24, true);
    logo->playAnimation("logo bumpin");
    logo->setPosition(-150, -100);
    logo->setAlpha(0);
    Engine::getInstance()->addAnimatedSprite(logo);

    enter = new AnimatedSprite();
    enter->loadFrames(Paths::image("titleEnter"), Paths::xml("images/titleEnter"));
    enter->addAnimation("ENTER PRESSED", "ENTER PRESSED", 24, true);
    enter->addAnimation("Press Enter to Begin", "Press Enter to Begin", 24, true);
    enter->playAnimation("Press Enter to Begin");
    enter->setPosition(100, Engine::getInstance()->getWindowHeight() * 0.8f);
    enter->setAlpha(0);
    Engine::getInstance()->addAnimatedSprite(enter);
}

void TitleState::update(float deltaTime) {
    Engine* engine = Engine::getInstance();
    FunkinState::update(deltaTime);
    if (gf) gf->update(deltaTime);
    if (logo) logo->update(deltaTime);
    if (enter) enter->update(deltaTime);

    static Uint32 musicStartTicks = 0;
    static bool musicStarted = false;
    if (Mix_PlayingMusic()) {
        if (!musicStarted) {
            musicStartTicks = SDL_GetTicks();
            musicStarted = true;
        }
        Conductor::songPosition = static_cast<float>(SDL_GetTicks() - musicStartTicks);
    } else {
        musicStarted = false;
    }
    int oldStep = curStep;
    updateCurStep();
    updateBeat();
    if (oldStep != curStep && curStep > 0) {
        stepHit();
    }

    if (whiteAlpha > 0.0f) {
        whiteAlpha -= deltaTime * 0.1f;
        if (whiteAlpha < 0.0f) whiteAlpha = 0.0f;
    }
    Input::UpdateKeyStates();
    if (Input::justPressed(SDL_SCANCODE_RETURN) && !skippedIntro) {
        skipIntro();
    }
    if (Input::justPressed(SDL_SCANCODE_RETURN) && skippedIntro && !f) {
        if (enter) enter->playAnim("ENTER PRESSED");
        if (confirm) confirm->play();
        f = true;
        Engine::getInstance()->switchState(new MainMenuState());
        Input::UpdateKeyStates();
    }
}

void TitleState::render() {
    SDL_Renderer* renderer = SDLManager::getInstance().getRenderer();
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    if (gf) gf->render();
    if (logo) logo->render();
    if (enter) enter->render();
    if (whiteAlpha > 0.0f) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, static_cast<Uint8>(whiteAlpha * 255));
        SDL_Rect rect = {0, 0, Engine::getInstance()->getWindowWidth(), Engine::getInstance()->getWindowHeight()};
        SDL_RenderFillRect(renderer, &rect);
    }

    if (!skippedIntro) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_Rect bgRect = {0, 0, Engine::getInstance()->getWindowWidth(), Engine::getInstance()->getWindowHeight()};
        SDL_RenderFillRect(renderer, &bgRect);
    }
    for (auto* alpha : alphabets) {
        if (alpha) alpha->render();
    }
}

void TitleState::destroy() {
    for (auto* alpha : alphabets) {
        if (alpha) {
            alpha->removeFromEngine();
        }
    }
    alphabets.clear();
    if (gf) { gf = nullptr; }
    if (logo) { logo = nullptr; }
    if (enter) { enter = nullptr; }
}

void TitleState::createCoolText(const std::vector<std::string>& lines) {
    int lineHeight = 60;
    int startY = 200;
    for (size_t i = 0; i < lines.size(); ++i) {
        Alphabet* alpha = new Alphabet(lines[i], 0, startY + static_cast<int>(i) * lineHeight);
        alpha->screenCenter();
        alpha->addToEngine();
        alphabets.push_back(alpha);
    }
}

void TitleState::createMoreCoolText(const std::string& line) {
    Alphabet* alpha = new Alphabet(line, 0, static_cast<int>(alphabets.size() * 60 + 200));
    alpha->screenCenter();
    alpha->addToEngine();
    alphabets.push_back(alpha);
}

void TitleState::removeText() {
    for (auto* alpha : alphabets) {
        if (alpha) {
            alpha->removeFromEngine();
        }
    }
    alphabets.clear();
}

void TitleState::skipIntro() {
    skippedIntro = true;
    removeText();
    whiteAlpha = 1.0f;
    if (gf) gf->setAlpha(1.0f);
    if (logo) logo->setAlpha(1.0f);
    if (enter) enter->setAlpha(1.0f);
}

void TitleState::beatHit() {
    if (skippedIntro) return;
    switch (curBeat) {
    case 1:
        createCoolText({ "NINJAMUFFIN99", "PHANTOMARCADE", "KAWAISPRITE", "EVILSKER" });
        break;
    case 3:
        createMoreCoolText("PRESENT");
        break;
    case 4:
        removeText();
        break;
    case 5:
        createCoolText({ "NOT IN ASSOCIATION", "WITH" });
        break;
    case 7:
        createMoreCoolText("NEWGROUNDS");
        break;
    case 8:
        removeText();
        break;
    case 9:
        createCoolText({ "POWERED BY" });
        break;
    case 11:
        removeText();
        createCoolText({ "HAMBURGER ENGINE" });
        break;
    case 12:
        removeText();
        break;
    case 13:
        createMoreCoolText("FRIDAY");
        break;
    case 14:
        createMoreCoolText("NIGHT");
        break;
    case 15:
        createMoreCoolText("FUNKIN");
        break;
    case 16:
        skipIntro();
        break;
    }
}