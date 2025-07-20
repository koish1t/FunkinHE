#pragma once
#include "../FunkinState.h"
#include "../../engine/graphics/AnimatedSprite.h"
#include "../../engine/graphics/Sprite.h"
#include "../../engine/audio/Sound.h"
#include <vector>
#include <string>
#include "../play/components/Alphabet.h"

class TitleState : public FunkinState {
public:
    TitleState();
    ~TitleState();
    void create() override;
    void update(float deltaTime) override;
    void render() override;
    void destroy() override;

    void beatHit() override;
    void createCoolText(const std::vector<std::string>& lines);
    void createMoreCoolText(const std::string& line);
    void removeText();
    void skipIntro();
private:
    AnimatedSprite* logo = nullptr;
    AnimatedSprite* gf = nullptr;
    AnimatedSprite* enter = nullptr;
    float whiteAlpha = 0.0f;
    Sound* confirm = nullptr;
    bool skippedIntro = false;
    bool f = false;
    std::vector<Alphabet*> alphabets;
};