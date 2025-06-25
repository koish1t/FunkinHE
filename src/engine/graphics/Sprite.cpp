#include "Sprite.h"
#include "Camera.h"
#include "../core/SDLManager.h"
#include <iostream>

Sprite::Sprite() 
    : imagePath("")
    , visible(true)  
{
}

Sprite::Sprite(const std::string& path) 
    : imagePath(path)
    , visible(true)
{
    loadTexture(path);  
}

Sprite::~Sprite() {
    if (texture) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
}

void Sprite::render() {
    if (!visible || !texture) return; 

    SDL_SetTextureAlphaMod(texture, static_cast<Uint8>(alpha * 255));

    SDL_Rect destRect;
    destRect.x = static_cast<int>(x);
    destRect.y = static_cast<int>(y);
    destRect.w = static_cast<int>(width * scale.x);
    destRect.h = static_cast<int>(height * scale.y);

    SDL_RenderCopy(SDLManager::getInstance().getRenderer(), texture, nullptr, &destRect);
}

void Sprite::loadTexture(const std::string& imagePath) {
    SDL_Surface* surface = IMG_Load(imagePath.c_str());
    if (!surface) {
        Log::getInstance().error("Failed to load image: " + imagePath);
        return;
    }

    width = surface->w;
    height = surface->h;

    texture = SDL_CreateTextureFromSurface(SDLManager::getInstance().getRenderer(), surface);
    SDL_FreeSurface(surface);

    if (!texture) {
        Log::getInstance().error("Failed to create texture from surface: " + std::string(SDL_GetError()));
        return;
    }
}

void Sprite::setScale(float scaleX, float scaleY) {
    scale.x = scaleX;
    scale.y = scaleY;
}
