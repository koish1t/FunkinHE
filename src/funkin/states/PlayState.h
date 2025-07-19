#pragma once

#include "../../engine/core/State.h"
#include "../../engine/graphics/Sprite.h"
#include "../../engine/graphics/AnimatedSprite.h"
#include "../../engine/input/Input.h"
#include "../../engine/audio/Sound.h"
#include "../substates/PauseSubState.h"
#include "../../engine/utils/Log.h"
#include "../game/Song.h"
#include "../game/Note.h"
#include "../game/GameConfig.h"
#include "../game/Stage.h"
#include "../FunkinState.h"
#include "../../engine/graphics/Camera.h"
#include "../../engine/core/SDLManager.h"
#include "../../engine/graphics/Text.h"
#include "../../engine/core/Engine.h"
#ifdef __MINGW32__ 
#elif defined(__SWITCH__)
#else
#include <utils/Discord.h>
#endif
#include <vector>
#include <array>
#include <string>
#include <map>

void playStateKeyboardCallback(unsigned char key, int x, int y);

struct KeyBinding {
    SDL_Scancode primary;
    SDL_Scancode alternate;
};

struct NXBinding {
    SDL_GameControllerButton primary;
    SDL_GameControllerButton alternate;
};

class PlayState : public FunkinState {
public:
    PlayState();
    ~PlayState();

    void create() override;
    void update(float deltaTime) override;
    void render() override;
    void destroy() override;

    void openSubState(SubState* subState);
    void generateSong(std::string dataPath);
    void startSong();
    void startCountdown();
    void generateStaticArrows(int player);
    void generateNotes();
    void goodNoteHit(Note* note);
    void noteMiss(int direction);

    static PlayState* instance;
    static SwagSong SONG;
    static Sound* inst;
    bool startingSong = false;
    bool startedCountdown = false;

    std::vector<Note*> unspawnNotes;
    int combo = 0;
    int score = 0;
    int misses = 0;
    Sound* getVocals() const { return vocals; }
    Stage* getCurrentStage() const { return currentStage; }
    Camera* getCamGame() const { return camGame; }
    Camera* getCamHUD() const { return camHUD; }

private:
    std::string curSong;
    Sound* vocals = nullptr;
    std::vector<AnimatedSprite*> strumLineNotes;
    std::vector<Note*> notes;
    Stage* currentStage = nullptr;
    Camera* camGame = nullptr;
    Camera* camHUD = nullptr;
    const float STRUM_X = 42.0f;
    const float STRUM_X_MIDDLESCROLL = -278.0f;
    const std::vector<std::string> NOTE_STYLES = {"arrow", "arrow", "arrow", "arrow"};
    const std::vector<std::string> NOTE_DIRS = {"LEFT", "DOWN", "UP", "RIGHT"};
    
    std::array<KeyBinding, 4> arrowKeys;
    std::array<NXBinding, 4> nxArrowKeys;

    void loadKeybinds();
    void loadSongConfig();
    void loadStage();
    void updateCameraZoom();
    void setupHUDCamera();
    void handleOpponentNoteHit(float deltaTime);
    SDL_Scancode getScancodeFromString(const std::string& keyName);
    SDL_GameControllerButton getButtonFromString(const std::string& buttonName);

    bool isKeyPressed(int keyIndex) const {
        const auto& binding = arrowKeys[keyIndex];
        return Input::pressed(binding.primary) || Input::pressed(binding.alternate);
    }

    bool isKeyJustPressed(int keyIndex) const {
        const auto& binding = arrowKeys[keyIndex];
        return Input::justPressed(binding.primary) || Input::justPressed(binding.alternate);
    }

    bool isKeyJustReleased(int keyIndex) const {
        const auto& binding = arrowKeys[keyIndex];
        return Input::justReleased(binding.primary) || Input::justReleased(binding.alternate);
    }

    bool isNXButtonPressed(int keyIndex) const {
        const auto& binding = nxArrowKeys[keyIndex];
        return Input::isControllerButtonPressed(binding.primary) || Input::isControllerButtonPressed(binding.alternate);
    }

    bool isNXButtonJustPressed(int keyIndex) const {
        const auto& binding = nxArrowKeys[keyIndex];
        return Input::isControllerButtonJustPressed(binding.primary) || Input::isControllerButtonJustPressed(binding.alternate);
    }

    bool isNXButtonJustReleased(int keyIndex) const {
        const auto& binding = nxArrowKeys[keyIndex];
        return Input::isControllerButtonJustReleased(binding.primary) || Input::isControllerButtonJustReleased(binding.alternate);
    }

    void handleInput();
    void updateArrowAnimations();
    Text* scoreText;
    void updateScoreText();
    float pauseCooldown = 0.0f;
    Uint32 musicStartTicks = 0;
};
