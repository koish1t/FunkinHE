#pragma once
#include <State.h>
#include "game/Conductor.h"

class FunkinState : public State {
public:
    FunkinState();
    virtual ~FunkinState() = default;

    virtual void create() override;
    virtual void update(float elapsed) override;
    virtual void render() override;
    virtual void destroy() override;

protected:
    float lastBeat;
    float lastStep;
    int curStep;
    int curBeat;

    virtual void updateBeat();
    virtual void updateCurStep();
    virtual void stepHit();
    virtual void beatHit();

private:
    static const std::string soundExt;
}; 