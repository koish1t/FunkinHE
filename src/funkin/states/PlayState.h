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
#include "../FunkinState.h"
#include "../../engine/SDLManager.h"
#include "../../engine/Text.h"
#include <vector>
#include <array>

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
    
    struct KeyBinding {
        SDL_Scancode primary;
        SDL_Scancode alternate;
    };
    
    const std::array<KeyBinding, 4> ARROW_KEYS = {{
        {SDL_SCANCODE_LEFT, SDL_SCANCODE_A},
        {SDL_SCANCODE_DOWN, SDL_SCANCODE_S},
        {SDL_SCANCODE_UP, SDL_SCANCODE_W},
        {SDL_SCANCODE_RIGHT, SDL_SCANCODE_D}
    }};

    bool isKeyPressed(int keyIndex) const {
        const auto& binding = ARROW_KEYS[keyIndex];
        return Input::pressed(binding.primary) || Input::pressed(binding.alternate);
    }

    bool isKeyJustPressed(int keyIndex) const {
        const auto& binding = ARROW_KEYS[keyIndex];
        return Input::justPressed(binding.primary) || Input::justPressed(binding.alternate);
    }

    bool isKeyJustReleased(int keyIndex) const {
        const auto& binding = ARROW_KEYS[keyIndex];
        return Input::justReleased(binding.primary) || Input::justReleased(binding.alternate);
    }

    void handleInput();
    void updateArrowAnimations();
    Text* scoreText;
    void updateScoreText();
    float pauseCooldown = 0.0f;
};
