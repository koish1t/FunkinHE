#pragma once
#include "../../engine/audio/Sound.h"
#include "../../engine/graphics/AnimatedSprite.h"
#include "../FunkinState.h"
#include <vector>
#include <string>

class MainMenuState : public FunkinState {
public:
    MainMenuState();
    ~MainMenuState();
    void create() override;
    void update(float deltaTime) override;
    void render() override;
    void destroy() override;
    void switchMenu(int direction);
private:
    Sprite* bg;
    std::vector<AnimatedSprite*> menuOptions;
    std::vector<std::string> optionLabels = {"STORY MODE", "FREEPLAY", "DONATE", "OPTIONS"};
    int selected = 0;
    Sound* scroll = nullptr;
    Sound* confirm = nullptr;
};
