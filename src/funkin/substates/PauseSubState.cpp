#ifdef __MINGW32__ || defined(__SWITCH__)
#include "funkin/substates/PauseSubState.h"
#include "engine/Engine.h"
#include "engine/Input.h"
#include "engine/SDLManager.h"
#include "engine/SoundManager.h"
#include "funkin/states/PlayState.h"
#include <iostream>
#else
#include "../funkin/substates/PauseSubState.h"
#include <Engine.h>
#include <Input.h>
#include <SDLManager.h>
#include <SoundManager.h>
#include "../funkin/states/PlayState.h"
#include <iostream>
#endif

PauseSubState::PauseSubState() : pauseText(nullptr) {
}

PauseSubState::~PauseSubState() {
    destroy();
}

void PauseSubState::create() {
    pauseText = new Text(300, 250, 200);
    pauseText->setText("PAUSED");
    pauseText->setFormat("assets/fonts/Zero G.ttf", 36, 0xFFFFFFFF);

    SoundManager::getInstance().pauseMusic();
    if (PlayState::inst) {
        PlayState::inst->pause();
    }
    PlayState* playState = dynamic_cast<PlayState*>(getParentState());
    if (playState) {
        Sound* vocals = playState->getVocals();
        if (vocals) {
            vocals->pause();
        }
    }
}

void PauseSubState::update(float deltaTime) {
    Input::UpdateKeyStates();
    Input::UpdateControllerStates();

    if (Input::justPressed(SDL_SCANCODE_RETURN) || Input::isControllerButtonJustPressed(SDL_CONTROLLER_BUTTON_START)) {
        std::cout << "Start button pressed in PauseSubState, closing" << std::endl;
        SoundManager::getInstance().resumeMusic();
        if (PlayState::inst) {
            PlayState::inst->resume();
        }
        PlayState* playState = dynamic_cast<PlayState*>(getParentState());
        if (playState) {
            Sound* vocals = playState->getVocals();
            if (vocals) {
                vocals->resume();
            }
        }
        getParentState()->closeSubState();
    }
}

void PauseSubState::render() {
    SDL_SetRenderDrawBlendMode(SDLManager::getInstance().getRenderer(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(SDLManager::getInstance().getRenderer(), 0, 0, 0, 128);
    Engine* engine = Engine::getInstance();
    SDL_Rect overlay = {0, 0, engine->getWindowWidth(), engine->getWindowHeight()};
    SDL_RenderFillRect(SDLManager::getInstance().getRenderer(), &overlay);
    SDL_SetRenderDrawColor(SDLManager::getInstance().getRenderer(), 0, 0, 0, 0);

    pauseText->render();
}

void PauseSubState::destroy() {
    if (pauseText) {
        delete pauseText;
        pauseText = nullptr;
    }
}
