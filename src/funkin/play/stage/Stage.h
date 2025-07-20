#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include "../../../engine/graphics/Sprite.h"
#include "../../../engine/graphics/Camera.h"
#include "../../backend/json.hpp"

using json = nlohmann::json;

class Stage {
private:
    std::string curStage;
    json stageData;
    std::vector<std::unique_ptr<Sprite>> sprites;
    std::map<std::string, Sprite*> namedSprites;
    float defaultCamZoom;
    bool stageLoaded;

    void loadStageScript(const std::string& stageName);
    void parseStageData(const json& data);
    void createSprite(const json& spriteData);
    void setupCamera(const json& cameraData);

public:
    Stage(const std::string& stageName);
    ~Stage();

    void update(float deltaTime);
    void render();
    std::string getCurStage() const { return curStage; }
    float getDefaultZoom() const { return defaultCamZoom; }
    bool isStageLoaded() const { return stageLoaded; }
    void addSprite(Sprite* sprite, const std::string& name = "");
    void removeSprite(Sprite* sprite);
    Sprite* getSprite(const std::string& name);    
    void setCamera(Camera* camera);
    Camera* getCamera() const;

private:
    Camera* stageCamera;
};
