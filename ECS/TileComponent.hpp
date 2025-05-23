#pragma once

#include "ECS.hpp"
#include "SDL2/SDL.h"

class TileComponent : public Component {
    public:

        SDL_Texture* texture;
        SDL_Rect srcRect, destRect;
        Vector2D position;

        TileComponent() = default;

        ~TileComponent() {}

        TileComponent(int srcX, int srcY, int xpos, int ypos, int tsize, int tscale, std::string id) {
            texture = Game::assets->GetTexture(id);

            position.x = xpos;
            position.y = ypos;

            srcRect.x = srcX;
            srcRect.y = srcY;
            srcRect.w = tsize;
            srcRect.h = tsize;

            destRect.x = xpos;
            destRect.y = ypos;
            destRect.w = tsize * tscale;
            destRect.h = tsize * tscale;
        }

        void update() override {
            destRect.x = position.x - Game::camera.x;
            destRect.y = position.y - Game::camera.y;
        }

        void draw() override {
            TextureManager::Draw(texture, srcRect, destRect, SDL_FLIP_NONE);
        }
};