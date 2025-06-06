#ifndef PLAYER_H
#define PLAYER_H

#include <SDL2/SDL.h>
#include <string>
#include "Constants.h"       // 使用核心常數
#include "AnimationData.h" // 需要 AnimationType
#include "AudioManager.h"

class Player {
public:
    // --- 狀態 ---
    enum class PlayerState {
        IDLE, WALKING, JUMPING, FALLING, ATTACKING, HURT, BLOCKING, DEATH, VICTORY, LYING
    };

    // --- 拳套類型 ---
    enum class GloveType {
        LIGHT_10OZ,    // 10oz 拳套
        MEDIUM_14OZ,   // 14oz 拳套
        HEAVY_18OZ     // 18oz 拳套
    };

    // --- 成員變數 ---
    float x, y;                     // 位置
    float vx, vy;                   // 速度
    int health;                   // 生命值
    int direction;                // 方向 (1: 右, -1: 左)
    PlayerState state;              // 目前狀態
    std::string characterId;        // 角色 ID (用於取得動畫和紋理)
    std::string textureId;          // 使用的紋理 ID (來自 TextureManager)
    GloveType currentGlove;         // 目前使用的拳套類型

    // 角色尺寸
    int logicWidth;                 // 角色的邏輯寬度
    int logicHeight;                // 角色的邏輯高度

    // 計時器與狀態旗標
    float attackTimer;
    float attackCooldownTimer;
    float hurtTimer;
    float invincibilityTimer;
    float blockCooldownTimer; 
    float attackRateCooldownTimer;
    float projectileCooldownTimer;
    float specialAttackCooldownTimer; // 新增：特殊攻擊冷卻計時器
    bool isOnGround;
    bool shouldFireProjectile = false;

    // 動畫相關
    int currentFrame;
    float frameTimer;
    AnimationType currentAnimationType; // 目前播放的動畫類型

    bool isSpecialAttacking = false;
    bool hasHitDuringDash = false;

    // --- 建構子 ---
    // 需要起始位置、方向，以及角色和紋理的 ID
    Player(float startX, float startY, int startDir,
           const std::string& charId, const std::string& texId);

    // --- 成員函數 (方法) ---
    void handleAction(const std::string& action); // 用 string 傳遞動作更靈活
    void update(float deltaTime);
    void render(SDL_Renderer* renderer); // 不再需要傳遞紋理，從 TextureManager 獲取
    void takeDamage(int damage);

    // 拳套相關方法
    void setGlove(GloveType gloveType);
    float getAttackCooldown() const;    // 根據拳套類型返回攻擊冷卻時間
    int getAttackDamage() const;        // 根據拳套類型返回攻擊傷害
    std::string getGloveName() const;   // 獲取拳套名稱

    SDL_Rect getBoundingBox() const; // 取得基於 PLAYER_LOGIC_WIDTH/HEIGHT 的碰撞盒
    SDL_Rect getHitboxWorld() const; // 取得世界座標的攻擊判定盒

    bool canFireProjectile() const; // 檢查是否能發射氣功
    void resetProjectileCooldown(); // 重置氣功冷卻
    bool isControllable() const;
    bool isAlive() const;           // 檢查是否存活 (方便碰撞檢測用)
    void updateAnimation(float deltaTime);
    void changeState(PlayerState newState); // 封裝狀態改變和動畫重置邏輯
    bool canUseSpecialAttack() const; // 新增：檢查是否可以使用特殊攻擊
    void resetSpecialAttackCooldown(); // 新增：重置特殊攻擊冷卻
private:
    // 內部輔助函數
    
    SDL_Rect calculateRelativeHitbox() const; // 計算相對於角色的攻擊框
};

#endif // PLAYER_H