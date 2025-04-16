#pragma once

#include "../../engine/State.h"
#include "../../engine/Sprite.h"
#include "../../engine/AnimatedSprite.h"
#include "../../engine/Input.h"
#include "../../engine/Sound.h"
#include "../substates/PauseSubState.h"
#include "../../engine/Log.h"
#include "../game/Song.h"
#include "../game/Note.h"
#include "../game/GameConfig.h"
#include "../FunkinState.h"
#include "../../engine/SDLManager.h"
#include "../../engine/Text.h"
#include "../../engine/Engine.h"
#ifdef __MINGW32__ 
#elif defined(__SWITCH__)
#else
#include <Discord.h>
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

private:
    std::string curSong;
    Sound* vocals = nullptr;
    std::vector<AnimatedSprite*> strumLineNotes;
    std::vector<Note*> notes;
    const float STRUM_X = 42.0f;
    const float STRUM_X_MIDDLESCROLL = -278.0f;
    const std::vector<std::string> NOTE_STYLES = {"arrow", "arrow", "arrow", "arrow"};
    const std::vector<std::string> NOTE_DIRS = {"LEFT", "DOWN", "UP", "RIGHT"};
    
    std::array<KeyBinding, 4> arrowKeys;
    std::array<NXBinding, 4> nxArrowKeys;

    void loadKeybinds();
    void loadSongConfig();
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
};
