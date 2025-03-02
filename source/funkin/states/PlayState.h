#pragma once
#include "../../engine/State.h"
#include "../../engine/Sprite.h"
#include "../../engine/AnimatedSprite.h"
#include "../FunkinState.h"

void playStateKeyboardCallback(unsigned char key, int x, int y);

class PlayState : public FunkinState {
public:
    PlayState();
    ~PlayState();

    void create() override;
    void update(float deltaTime) override;
    void render() override;
    void destroy() override;

    void openSubState(SubState* subState);

    virtual void keyPressed(unsigned char key, int x, int y) override;
    virtual void specialKeyPressed(int key, int x, int y) override;
    static PlayState* instance;

private:
};
