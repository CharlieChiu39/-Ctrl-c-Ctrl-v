#include <stdio.h> // for printf
#include "TextureManager.h"

// 初始化靜態成員變數
std::map<std::string, SDL_Texture*> TextureManager::textureMap;

bool TextureManager::loadTexture(SDL_Renderer* renderer, const std::string& id, const std::string& filePath) {
    // 檢查是否已載入相同 ID 的紋理
    if (textureMap.count(id)) {
        printf("Warning: Texture with ID '%s' already loaded.\n", id.c_str());
        return true; // 視為成功，因為已存在
    }

    SDL_Texture* newTexture = IMG_LoadTexture(renderer, filePath.c_str());
    if (newTexture == nullptr) {
        printf("Error: Failed to load texture '%s'! SDL_image Error: %s\n", filePath.c_str(), IMG_GetError());
        return false;
    }

    printf("Success: Loaded texture '%s' with ID '%s'\n", filePath.c_str(), id.c_str());
    textureMap[id] = newTexture; // 將紋理存入 map
    return true;
}

SDL_Texture* TextureManager::getTexture(const std::string& id) {
    if (textureMap.count(id)) {
        return textureMap[id];
    }
    printf("Warning: Texture with ID '%s' not found.\n", id.c_str());
    return nullptr; // 找不到返回 nullptr
}

void TextureManager::unloadTexture(const std::string& id) {
    if (textureMap.count(id)) {
        SDL_DestroyTexture(textureMap[id]);
        textureMap.erase(id); // 從 map 中移除
        printf("Unloaded texture with ID '%s'\n", id.c_str());
    } else {
        printf("Warning: Cannot unload texture. ID '%s' not found.\n", id.c_str());
    }
}

void TextureManager::unloadAllTextures() {
    printf("Unloading all textures...\n");
    for (auto const& [id, texture] : textureMap) {
        if (texture) {
            SDL_DestroyTexture(texture);
        }
         printf("  - Unloaded texture ID: %s\n", id.c_str());
    }
    textureMap.clear(); // 清空 map
    printf("All textures unloaded.\n");
}