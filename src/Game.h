#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <vector>
#include <string> 
#include "Player.h" // 包含 Player
#include "AudioManager.h"

// --- 在 class Game 之前定義 Projectile ---
struct Projectile {
    float x = 0.0f;
    float y = 0.0f;
    float vx = 0.0f;
    int ownerPlayerIndex = -1; // 是哪個玩家發射的 (0 或 1)
    bool isActive = false;
    std::string textureId = "projectile_sprites"; // <-- 使用的紋理 ID (要跟載入時一致)
    SDL_Rect srcRect = {0, 0, 0, 0}; // <-- 氣功在精靈圖上的來源矩形

    // 更新位置
    void update(float deltaTime) {
        if (isActive) {
            x += vx * deltaTime;
            // 簡單的邊界檢查
            if (x > SCREEN_WIDTH + 50 || x < -50) { // 稍微超出邊界才消失
                isActive = false;
            }
        }
    }

    // 取得碰撞框
    SDL_Rect getBoundingBox() const {
        // 使用 Constants.h 中定義的碰撞大小
        return {(int)x, (int)y, PROJECTILE_HITBOX_W, PROJECTILE_HITBOX_H};
    }

    // 繪製 (Renderer 和 Texture 需要從 Game 傳入)
    void render(SDL_Renderer* renderer, SDL_Texture* texture) {
        if (isActive && texture) {
            SDL_Rect destRect = {(int)x, (int)y, PROJECTILE_HITBOX_W, PROJECTILE_HITBOX_H};
            // 注意：這裡的翻轉通常不需要，氣功圖本身應該是中性的
            SDL_RenderCopy(renderer, texture, &srcRect, &destRect);
        }
    }
};

// 核心遊戲類別
class Game {
public:
    Game();
    ~Game();

    // 初始化 SDL、視窗、渲染器、資源等
    bool initialize();

    // 運行遊戲主迴圈
    void run();

    // 清理資源
    void cleanup();

private:
    // 處理事件
    void handleEvents();

    // 更新遊戲狀態
    void update(float deltaTime);

    // 繪製遊戲畫面
    void render();

    // 碰撞檢測
    void checkCollisions();

    // SDL 相關
    SDL_Window* window;
    SDL_Renderer* renderer;

    // 遊戲物件 (使用 vector 以便未來擴充)
    std::vector<Player> players; // 目前只有兩個玩家
    std::vector<Projectile> projectiles;

    // 遊戲狀態
    bool isRunning;
    Uint32 lastFrameTime;

     // 生成氣功
     void spawnProjectile(float startX, float startY, int direction, int ownerIndex);
};

#endif // GAME_H