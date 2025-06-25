#pragma once
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <functional>
#include "Sprite.h"

class AnimatedSprite : public Sprite {
public:
    struct Frame {
        std::string name;
        int x, y, width, height;
        int frameX, frameY, frameWidth, frameHeight;
    };

    struct Animation {
        std::string name;
        std::vector<Frame> frames;
        int frameRate;
        bool loop;

        void addFrame(const Frame& frame) { frames.push_back(frame); }
    };

    AnimatedSprite();
    AnimatedSprite(const std::string& path);
    virtual ~AnimatedSprite();

    virtual void update(float deltaTime) override;
    virtual void render() override;

    void setScale(float scaleX, float scaleY);
    void loadFrames(const std::string& imagePath, const std::string& xmlPath);
    void addAnimation(const std::string& name, const std::string& prefix, int fps, bool loop = true);
    void addAnimation(const std::string& name, const std::string& prefix, const std::vector<int>& indices, int fps, bool loop = true);
    void addAnimation(const std::string& name, const std::vector<std::string>& frameNames, int fps, bool loop = true);
    void playAnimation(const std::string& name);

    bool hasAnimation(const std::string& name) const {
        return animations.find(name) != animations.end();
    }
    
    using AnimationCallback = std::function<void()>;

    void setAnimationFinishedCallback(AnimationCallback callback) {
        onAnimationFinished = callback;
    }

    void playAnim(const std::string& name, bool force = false, AnimationCallback callback = nullptr);

    void setOffset(float x, float y) {
        offsetX = x;
        offsetY = y;
    }
    
    void setFlipX(bool flip) {
        scale.x = flip ? -1.0f : 1.0f;
    }

    bool isAnimationPlaying() const {
        return currentAnimation && 
               (!currentAnimation->loop && 
                currentFrame < currentAnimation->frames.size() - 1);
    }

    float alpha = 1.0f;
    void updateHitbox() {
        if (currentAnimation && !currentAnimation->frames.empty()) {
            const Frame& frame = currentAnimation->frames[currentFrame];
            width = frame.width;
            height = frame.height;
        }
    }

    const std::string& getCurrentAnimation() const {
        return currentAnimation ? currentAnimation->name : "";
    }

    const std::map<std::string, Animation>& getAnimations() const {
        return animations;
    }

    void copyAnimationsFrom(const AnimatedSprite& other) {
        animations = other.getAnimations();
    }

    SDL_Texture* shareTexture() const { return texture; }
    
    const std::map<std::string, Frame>& getFrames() const { return frames; }
    void copyFramesFrom(const AnimatedSprite& other) { frames = other.getFrames(); }

protected:
    std::map<std::string, Frame> frames;
    float offsetX = 0;
    float offsetY = 0;

private:
    std::map<std::string, Animation> animations;
    Animation* currentAnimation = nullptr;
    int currentFrame = 0;
    float frameTimer = 0;
    AnimationCallback onAnimationFinished = nullptr;

    void parseXML(const std::string& xmlPath);
    void loadTexture(const std::string& imagePath) override;
};
