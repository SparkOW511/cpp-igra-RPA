#include "AssetManager.hpp"
#include "ECS/Components.hpp"

AssetManager::AssetManager(Manager* man) : manager(man), currentMusic("") {}

AssetManager::~AssetManager() {
    for (auto& texture : textures) {
        if (texture.second != nullptr) {
            SDL_DestroyTexture(texture.second);
        }
    }
    textures.clear();
    
    for (auto& font : fonts) {
        if (font.second != nullptr) {
            TTF_CloseFont(font.second);
        }
    }
    fonts.clear();
    
    for (auto& sound : sounds) {
        if (sound.second != nullptr) {
            Mix_FreeChunk(sound.second);
        }
    }
    sounds.clear();
    
    for (auto& track : music) {
        if (track.second != nullptr) {
            Mix_FreeMusic(track.second);
        }
    }
    music.clear();
}

void AssetManager::CreateProjectile(Vector2D pos, Vector2D vel, int range, int speed, std::string id, SDL_RendererFlip bulletFlip) {
    auto& projectile(manager->addEntity());
    projectile.addComponent<TransformComponent>(pos.x, pos.y, 32 , 32, 1);
    projectile.addComponent<SpriteComponent>(id, false);
    projectile.addComponent<ProjectileComponent>(range, speed, vel);
    projectile.addComponent<ColliderComponent>("projectile");
    projectile.addGroup(Game::groupProjectiles);
    projectile.getComponent<SpriteComponent>().SetFlip(bulletFlip);
}

void AssetManager::CreateObject(int posX, int posY, std::string id) {
    auto& obj(manager->addEntity());
    obj.addComponent<TransformComponent>(posX, posY, 32, 32, 1);
    obj.addComponent<SpriteComponent>(id, true);
    obj.addComponent<ColliderComponent>(id);
    obj.addGroup(Game::groupObjects);
}

void AssetManager::AddTexture(std::string id, const char* path) {
    textures.emplace(id, TextureManager::LoadTexture(path));
}

SDL_Texture* AssetManager::GetTexture(std::string id) {
    return textures[id];
}

void AssetManager::AddFont(std::string id, std::string path, int fontSize) {
    fonts.emplace(id, TTF_OpenFont(path.c_str(), fontSize));
}

TTF_Font* AssetManager::GetFont(std::string id) {
    return fonts[id];
}

void AssetManager::AddSound(std::string id, const char* path) {
    Mix_Chunk* chunk = Mix_LoadWAV(path);
    if (!chunk) {
        printf("Failed to load sound effect: %s\n", Mix_GetError());
        return;
    }
    sounds.emplace(id, chunk);
}

Mix_Chunk* AssetManager::GetSound(std::string id) {
    if (sounds.find(id) == sounds.end()) {
        printf("Sound not found: %s\n", id.c_str());
        return nullptr;
    }
    return sounds[id];
}

void AssetManager::PlaySound(std::string id, int volume) {
    Mix_Chunk* sound = GetSound(id);
    if (sound) {
        int scaledVolume = (volume * MIX_MAX_VOLUME) / 100;
        Mix_VolumeChunk(sound, scaledVolume);
        Mix_PlayChannel(-1, sound, 0);
    }
}

void AssetManager::AddMusic(std::string id, const char* path) {
    Mix_Music* mus = Mix_LoadMUS(path);
    
    if (!mus) {
        printf("Failed to load music file '%s': %s\n", path, Mix_GetError());
        printf("Creating silent placeholder music instead\n");
        
        music.emplace(id, nullptr);
        return;
    }
    
    music.emplace(id, mus);
    printf("Successfully loaded music: %s\n", id.c_str());
}

Mix_Music* AssetManager::GetMusic(std::string id) {
    auto it = music.find(id);
    if (it == music.end()) {
        printf("Music not found: %s\n", id.c_str());
        return nullptr;
    }
    return it->second;
}

void AssetManager::PlayMusic(std::string id, int volume, int loops) {
    if (currentMusic == id && Mix_PlayingMusic()) {
        return;
    }
    
    StopMusic();
    
    currentMusic = id;
    
    Mix_Music* mus = GetMusic(id);
    if (!mus) {
        printf("Music '%s' not available - using silent placeholder\n", id.c_str());
        return;
    }
    
    int scaledVolume = (volume * MIX_MAX_VOLUME) / 100;
    Mix_VolumeMusic(scaledVolume);
    
    int result = Mix_PlayMusic(mus, loops);
    if (result == -1) {
        printf("Failed to play music '%s': %s\n", id.c_str(), Mix_GetError());
        
        std::string error = Mix_GetError();
        if (error.find("MP3") != std::string::npos || 
            error.find("mp3") != std::string::npos ||
            error.find("format") != std::string::npos ||
            error.find("decode") != std::string::npos) {
            printf("Error appears to be related to MP3 format or decoding.\n");
            printf("Consider converting your music files to OGG format for better compatibility.\n");
            printf("Example: ffmpeg -i input.mp3 -c:a libvorbis -q:a 4 output.ogg\n");
        }
    } else {
        printf("Successfully playing music: %s\n", id.c_str());
    }
}

void AssetManager::StopMusic() {
    if (Mix_PlayingMusic()) {
        Mix_HaltMusic();
        currentMusic = "";
    }
}

void AssetManager::PauseMusic() {
    if (Mix_PlayingMusic() && !Mix_PausedMusic()) {
        Mix_PauseMusic();
    }
}

void AssetManager::ResumeMusic() {
    if (Mix_PausedMusic()) {
        Mix_ResumeMusic();
    }
}

void AssetManager::SetMasterVolume(int volume) {
    int scaledVolume = (volume * MIX_MAX_VOLUME) / 100;
    
    Mix_VolumeMusic(scaledVolume);
    
    for (auto& sound : sounds) {
        if (sound.second) {
            Mix_VolumeChunk(sound.second, scaledVolume);
        }
    }
}