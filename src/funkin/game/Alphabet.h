#pragma once
#include <vector>
#include <string>
#include "../../engine/graphics/AnimatedSprite.h"

class Alphabet {
public:
    Alphabet(const std::string& text, int x, int y);
    ~Alphabet();
    void screenCenter();
    void addToEngine();
    void removeFromEngine();
    void render();
private:
    std::vector<AnimatedSprite*> letters;
    int baseX, baseY;
};