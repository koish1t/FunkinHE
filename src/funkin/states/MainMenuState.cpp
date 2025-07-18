#include "MainMenuState.h"
#include "../states/PlayState.h"
#include "../../engine/core/Engine.h"
#include "../../engine/input/Input.h"
#include "../../engine/utils/Paths.h"
#include "../../engine/audio/SoundManager.h"
#include "../../engine/graphics/AnimatedSprite.h"
#include "../../engine/graphics/Sprite.h"
#include <SDL2/SDL.h>
#include <iostream>

static AnimatedSprite* baseMenuAssets = nullptr;

MainMenuState::MainMenuState() : bg(nullptr) {}
MainMenuState::~MainMenuState() { destroy(); }

void MainMenuState::create() {
    selected = 0;
    scroll = Engine::getInstance()->getSoundManager().loadSound(Paths::sound("scrollMenu"));
    confirm = Engine::getInstance()->getSoundManager().loadSound(Paths::sound("confirmMenu"));
    bg = new Sprite(Paths::image("menuBG"));
    bg->setPosition(0, 0);
    Engine::getInstance()->addSprite(bg);

    int lineHeight = 160;
    if (!baseMenuAssets) {
        baseMenuAssets = new AnimatedSprite();
        baseMenuAssets->loadFrames(Paths::image("FNF_main_menu_assets"), Paths::xml("images/FNF_main_menu_assets"));
    }
    std::vector<std::string> optionAnims = {"story mode", "freeplay", "donate", "options"};
    for (size_t i = 0; i < optionAnims.size(); ++i) {
        AnimatedSprite* option = new AnimatedSprite();
        option->copyFramesFrom(*baseMenuAssets);
        option->copyAnimationsFrom(*baseMenuAssets);
        option->setTexture(baseMenuAssets->shareTexture());
        option->addAnimation(optionAnims[i] + " basic", optionAnims[i] + " basic", 24, true);
        option->addAnimation(optionAnims[i] + " white", optionAnims[i] + " white", 24, true);
        std::string animName = optionAnims[i] + (i == selected ? " white" : " basic");
        option->playAnimation(animName);
        option->update(0);
        menuOptions.push_back(option);
    }
    int totalHeight = static_cast<int>(menuOptions.size()) * lineHeight;
    int startY = (Engine::getInstance()->getWindowHeight() / 2) - (totalHeight / 2);
    int offsetX = 750;
    for (size_t i = 0; i < menuOptions.size(); ++i) {
        AnimatedSprite* option = menuOptions[i];
        if (option) {
            int optionWidth = option->getWidth();
            int x = (Engine::getInstance()->getWindowWidth() / 2) - (optionWidth / 2) + offsetX;
            int y = startY + static_cast<int>(i) * lineHeight;
            option->setPosition(static_cast<float>(x), static_cast<float>(y));
        }
    }
}

void MainMenuState::update(float deltaTime) {
    FunkinState::update(deltaTime);
    Input::UpdateKeyStates();
    if (Input::justPressed(SDL_SCANCODE_UP) || Input::justPressed(SDL_SCANCODE_W)) {
        switchMenu(-1);
    }
    if (Input::justPressed(SDL_SCANCODE_DOWN) || Input::justPressed(SDL_SCANCODE_S)) {
        switchMenu(1);
    }
    if (Input::justPressed(SDL_SCANCODE_RETURN)) {
        if (confirm) confirm->play();
        if (selected == 0) { // story mode
            Engine::getInstance()->getSoundManager().stopMusic();
            Engine::getInstance()->switchState(new PlayState());
        }
        if (selected == 1) { // freeplay
            Engine::getInstance()->getSoundManager().stopMusic();
            Engine::getInstance()->switchState(new PlayState());
        }
        if (selected == 2) { // donate
        }
        if (selected == 3) { // options
            std::cout << "Options" << std::endl;
        }
    }
    std::vector<std::string> optionAnims = {"story mode", "freeplay", "donate", "options"};
    for (size_t i = 0; i < menuOptions.size(); ++i) {
        std::string animName = optionAnims[i] + (i == selected ? " white" : " basic");
        menuOptions[i]->playAnimation(animName);
    }
    for (auto* option : menuOptions) {
        if (option) option->update(deltaTime);
    }
}

void MainMenuState::render() {
    if (bg) bg->render();
    for (auto* option : menuOptions) {
        if (option) option->render();
    }
}

void MainMenuState::destroy() {
    for (auto* option : menuOptions) {
        if (option) option = nullptr;
    }
    menuOptions.clear();
    if (bg) { bg = nullptr; }
}

void MainMenuState::switchMenu(int direction) {
    int oldSelected = selected;
    selected += direction;
    if (selected < 0) selected = static_cast<int>(optionLabels.size()) - 1;
    if (selected >= (int)optionLabels.size()) selected = 0;
    if (scroll && selected != oldSelected) scroll->play();
}
