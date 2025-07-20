#include "Stage.h"
#include "../../../engine/utils/Paths.h"
#include "../../../engine/utils/Log.h"
#include <fstream>
#include <sstream>
#include <iostream>

Stage::Stage(const std::string& stageName) 
    : curStage(stageName), defaultCamZoom(0.9f), stageLoaded(false), stageCamera(nullptr) {
    loadStageScript(stageName);
}

Stage::~Stage() {
    sprites.clear();
    namedSprites.clear();
}

void Stage::loadStageScript(const std::string& stageName) {
    try {
        std::string stagePath = Paths::json("data/stages/" + stageName);
        
        if (!Paths::exists(stagePath)) {
            Log::getInstance().warning("Stage JSON file not found: " + stagePath);
            stageLoaded = false;
            return;
        }

        std::ifstream file(stagePath);
        if (!file.is_open()) {
            Log::getInstance().error("Failed to open stage file: " + stagePath);
            stageLoaded = false;
            return;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();

        stageData = json::parse(buffer.str());
        parseStageData(stageData);
        
        Log::getInstance().info("Successfully loaded stage: " + stageName);
        stageLoaded = true;

    } catch (const json::exception& e) {
        Log::getInstance().error("JSON parsing error for stage " + stageName + ": " + e.what());
        stageLoaded = false;
    } catch (const std::exception& e) {
        Log::getInstance().error("Error loading stage " + stageName + ": " + e.what());
        stageLoaded = false;
    }
}

void Stage::parseStageData(const json& data) {
    try {
        if (data.contains("defaultCamZoom")) {
            defaultCamZoom = data["defaultCamZoom"].get<float>();
        }

        if (data.contains("camera")) {
            setupCamera(data["camera"]);
        }

        if (data.contains("sprites") && data["sprites"].is_array()) {
            for (const auto& spriteData : data["sprites"]) {
                createSprite(spriteData);
            }
        }

    } catch (const json::exception& e) {
        Log::getInstance().error("Error parsing stage data: " + std::string(e.what()));
    }
}

void Stage::createSprite(const json& spriteData) {
    try {
        if (!spriteData.contains("path")) {
            Log::getInstance().warning("Sprite data missing 'path' field");
            return;
        }

        std::string imagePath = "assets/" + spriteData["path"].get<std::string>();
        auto sprite = std::make_unique<Sprite>(imagePath);

        if (spriteData.contains("x")) {
            sprite->setPosition(spriteData["x"].get<float>(), sprite->getY());
        }
        if (spriteData.contains("y")) {
            sprite->setPosition(sprite->getX(), spriteData["y"].get<float>());
        }

        if (spriteData.contains("scaleX") && spriteData.contains("scaleY")) {
            sprite->setScale(spriteData["scaleX"].get<float>(), spriteData["scaleY"].get<float>());
        } else if (spriteData.contains("scale")) {
            float scale = spriteData["scale"].get<float>();
            sprite->setScale(scale, scale);
        }

        if (spriteData.contains("alpha")) {
            sprite->setAlpha(spriteData["alpha"].get<float>());
        }

        if (spriteData.contains("visible")) {
            sprite->setVisible(spriteData["visible"].get<bool>());
        }

        if (stageCamera) {
            sprite->setCamera(stageCamera);
        }

        std::string spriteName = "";
        if (spriteData.contains("name")) {
            spriteName = spriteData["name"].get<std::string>();
            namedSprites[spriteName] = sprite.get();
        }

        sprites.push_back(std::move(sprite));

    } catch (const json::exception& e) {
        Log::getInstance().error("Error creating sprite: " + std::string(e.what()));
    }
}

void Stage::setupCamera(const json& cameraData) {
    try {        
        if (cameraData.contains("zoom")) {
            defaultCamZoom = cameraData["zoom"].get<float>();
        }
    } catch (const json::exception& e) {
        Log::getInstance().error("Error setting up camera: " + std::string(e.what()));
    }
}

void Stage::update(float deltaTime) {
    for (auto& sprite : sprites) {
        sprite->update(deltaTime);
    }
}

void Stage::render() {
    for (auto& sprite : sprites) {
        if (sprite->isVisible()) {
            sprite->render();
        }
    }
}

void Stage::addSprite(Sprite* sprite, const std::string& name) {
    if (sprite) {
        if (stageCamera) {
            sprite->setCamera(stageCamera);
        }
        
        if (!name.empty()) {
            namedSprites[name] = sprite;
        }
        auto spriteCopy = std::make_unique<Sprite>(*sprite);
        sprites.push_back(std::move(spriteCopy));
    }
}

void Stage::removeSprite(Sprite* sprite) {
    for (auto it = namedSprites.begin(); it != namedSprites.end(); ++it) {
        if (it->second == sprite) {
            namedSprites.erase(it);
            break;
        }
    }

    sprites.erase(
        std::remove_if(sprites.begin(), sprites.end(),
            [sprite](const std::unique_ptr<Sprite>& s) {
                return s.get() == sprite;
            }),
        sprites.end()
    );
}

Sprite* Stage::getSprite(const std::string& name) {
    auto it = namedSprites.find(name);
    if (it != namedSprites.end()) {
        return it->second;
    }
    return nullptr;
}

void Stage::setCamera(Camera* camera) {
    stageCamera = camera;
    
    for (auto& sprite : sprites) {
        sprite->setCamera(camera);
    }
    
    if (stageCamera) {
        stageCamera->setZoom(defaultCamZoom);
    }
}

Camera* Stage::getCamera() const {
    return stageCamera;
}
