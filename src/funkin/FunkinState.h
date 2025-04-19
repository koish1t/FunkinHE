#pragma once

#include "../engine/core/State.h"
#include "../engine/audio/Sound.h"
#include "game/Conductor.h"

class FunkinState : public State {
public:
    FunkinState();
    virtual ~FunkinState();

    void create() override;
    void update(float deltaTime) override;
    void render() override;
    void destroy() override;

    void updateBeat();
    void updateCurStep();
    void stepHit();
    void beatHit();

    static const std::string soundExt;
    int curStep = 0;
    int curBeat = 0;
    float lastBeat = 0.0f;
    float lastStep = 0.0f;
}; 