#include "Alphabet.h"
#include "../../engine/utils/Paths.h"
#include "../../engine/core/Engine.h"
#include <cctype>
#include <vector>
#include <sstream>

Alphabet::Alphabet(const std::string& text, int x, int y) : baseX(x), baseY(y) {
    static AnimatedSprite* baseSpr = nullptr;
    if (!baseSpr) {
        baseSpr = new AnimatedSprite();
        baseSpr->loadFrames(Paths::image("alphabet"), Paths::xml("images/alphabet"));
    }
    int curX = x;
    int curY = y;
    const int letterSpacing = 48;
    const int spaceWidth = 48;
    const int lineHeight = 60;
    for (size_t i = 0; i < text.size(); ++i) {
        char c = text[i];
        if (c == '\n') {
            curX = x;
            curY += lineHeight;
            continue;
        }
        if (c == ' ') {
            letters.push_back(nullptr);
            curX += spaceWidth;
            continue;
        }
        AnimatedSprite* spr = new AnimatedSprite();
        spr->copyFramesFrom(*baseSpr);
        spr->copyAnimationsFrom(*baseSpr);
        spr->setTexture(baseSpr->shareTexture());
        std::string animName;
        std::string frameName;
        if (std::isdigit(c)) {
            animName = std::string(1, c);
            frameName = animName + "0000";
        } else if (std::isalpha(c)) {
            animName = std::string(1, c);
            if (std::isupper(c)) {
                animName += " bold";
            } else {
                animName += " lowercase";
            }
            frameName = animName + "0000";
        } else {
            animName = std::string(1, c);
            frameName = animName + "0000";
        }
        std::vector<std::string> frames = { frameName };
        spr->addAnimation(animName, frames, 0, false);
        spr->playAnimation(animName);
        spr->setPosition(curX, curY);
        spr->setAlpha(1.0f);
        letters.push_back(spr);
        curX += letterSpacing;
    }
}

Alphabet::~Alphabet() {
    for (auto* spr : letters) {
        delete spr;
    }
    letters.clear();
}

void Alphabet::screenCenter() {
    int totalWidth = static_cast<int>(letters.size()) * 48;
    int startX = (Engine::getInstance()->getWindowWidth() / 2) - (totalWidth / 2);
    int curX = startX;
    int y = baseY;
    for (auto* spr : letters) {
        if (spr) {
            spr->setPosition(curX, y);
            curX += 48;
        } else {
            curX += 48;
        }
    }
}

void Alphabet::addToEngine() {
    for (auto* spr : letters) {
        if (spr) Engine::getInstance()->addAnimatedSprite(spr);
    }
}

void Alphabet::removeFromEngine() {
    for (auto* spr : letters) {
        if (spr) spr->setVisible(false);
    }
}

void Alphabet::render() {
    for (auto* spr : letters) {
        if (spr) spr->render();
    }
}