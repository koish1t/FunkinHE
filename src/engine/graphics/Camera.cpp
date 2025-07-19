#include "Camera.h"
#include "../core/SDLManager.h"

Camera::Camera() {}

void Camera::begin() {
    if (!visible) return;
    
    SDL_Renderer* renderer = SDLManager::getInstance().getRenderer();
    SDL_RenderSetViewport(renderer, nullptr);
    SDL_RenderSetClipRect(renderer, nullptr);
}

void Camera::end() {
    if (!visible) return;
}

void Camera::applyTransform(SDL_Rect& rect) {
    if (!visible) return;
    
    rect.x = static_cast<int>(rect.x * zoom);
    rect.y = static_cast<int>(rect.y * zoom);
    rect.w = static_cast<int>(rect.w * zoom);
    rect.h = static_cast<int>(rect.h * zoom);    
    rect.x -= static_cast<int>(x * zoom);
    rect.y -= static_cast<int>(y * zoom);
}
