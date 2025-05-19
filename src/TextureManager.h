#ifndef TEXTUREMANAGER_H
#define TEXTUREMANAGER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>
#include <map>

class TextureManager {
public:
    // 載入紋理，給它一個 ID 和檔案路徑
    static bool loadTexture(SDL_Renderer* renderer, const std::string& id, const std::string& filePath);

    // 透過 ID 取得紋理指標
    static SDL_Texture* getTexture(const std::string& id);

    // 釋放指定 ID 的紋理
    static void unloadTexture(const std::string& id);

    // 釋放所有已載入的紋理 (遊戲結束時呼叫)
    static void unloadAllTextures();

private:
    // 使用 map 來儲存紋理 ID 和對應的 SDL_Texture 指標
    static std::map<std::string, SDL_Texture*> textureMap;
};

#endif // TEXTUREMANAGER_H