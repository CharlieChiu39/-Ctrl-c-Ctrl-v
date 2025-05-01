
// main.cpp
// (請確保你有設定好 C++ 編譯器以及連結 SDL2 函式庫)
#include <SDL2/SDL.h>
#include <stdio.h>
#include <vector> // 會用到但這裡先不用
#include <cmath>  // for fabsf

// --- 基本設定 ---
const int SCREEN_WIDTH = 1024;
const int SCREEN_HEIGHT = 576;
const int GROUND_LEVEL = SCREEN_HEIGHT - 80;
const int PLAYER_WIDTH = 50;
const int PLAYER_HEIGHT = 100;
const float MOVE_SPEED = 300.0f;
const float JUMP_STRENGTH = 800.0f;
const float GRAVITY = 2500.0f;
const float ATTACK_DURATION = 0.3f;   // 攻擊狀態持續多久
const float ATTACK_COOLDOWN = 0.1f;  // 攻擊後的短暫冷卻
const float HURT_DURATION = 0.4f;     // 受傷硬直時間
const float HURT_INVINCIBILITY = 0.6f;// 受傷後無敵時間
const int ATTACK_DAMAGE = 15;         // 每次攻擊傷害

// --- 簡化狀態 ---
enum class PlayerState {
    IDLE,
    WALKING,
    JUMPING,
    FALLING,
    ATTACKING,
    HURT
};

// --- 簡化 Player 類別 ---
class Player {
public:
    float x, y;           // 位置
    float vx, vy;         // 速度
    int health;         // 血量
    int direction;      // 1 = right, -1 = left
    PlayerState state;    // 目前狀態
    SDL_Rect bounds;      // 代表身體的碰撞框 (相對於 x, y)
    SDL_Rect hitbox;      // 攻擊框 (相對於 x, y, 只有攻擊時有效)

    // 計時器與狀態旗標
    float attackTimer;     // > 0 表示正在攻擊
    float attackCooldownTimer; // > 0 表示攻擊剛結束，不能馬上再攻
    float hurtTimer;       // > 0 表示正在受傷硬直
    float invincibilityTimer; // > 0 表示無敵

    bool isOnGround;

    // 建構子
    Player(float startX, float startY, int startDir) :
        x(startX), y(startY), vx(0.0f), vy(0.0f), health(100), direction(startDir),
        state(PlayerState::FALLING),
        bounds({0, 0, PLAYER_WIDTH, PLAYER_HEIGHT}), // 身體框相對位置(0,0)，大小固定
        hitbox({0, 0, 0, 0}), // 初始攻擊框無效
        attackTimer(0.0f), attackCooldownTimer(0.0f), hurtTimer(0.0f), invincibilityTimer(0.0f),
        isOnGround(false)
        {}

    // 處理單一動作（左右、跳、攻擊）
    void handleAction(const char* action) {
        // 不能行動的狀態
        if (state == PlayerState::HURT || health <= 0) return;

        if (strcmp(action, "LEFT") == 0 && state != PlayerState::ATTACKING) {
            vx = -MOVE_SPEED; direction = -1;
            if (isOnGround) state = PlayerState::WALKING;
        } else if (strcmp(action, "RIGHT") == 0 && state != PlayerState::ATTACKING) {
            vx = MOVE_SPEED; direction = 1;
            if (isOnGround) state = PlayerState::WALKING;
        } else if (strcmp(action, "JUMP") == 0 && isOnGround && state != PlayerState::ATTACKING) {
            vy = -JUMP_STRENGTH; isOnGround = false;
            state = PlayerState::JUMPING;
        } else if (strcmp(action, "ATTACK") == 0 && state != PlayerState::ATTACKING && attackCooldownTimer <= 0) {
            state = PlayerState::ATTACKING;
            attackTimer = ATTACK_DURATION;
            attackCooldownTimer = ATTACK_DURATION + ATTACK_COOLDOWN; // 攻擊結束後才能再按
            vx = 0; // 攻擊時停下 (可選)
            // 設定攻擊框相對位置和大小 (範例：往前伸)
            if (direction == 1) {
                 hitbox = { PLAYER_WIDTH, PLAYER_HEIGHT / 4, 40, 20 }; // x, y, w, h 相對 player 左上角
            } else {
                 hitbox = { -40, PLAYER_HEIGHT / 4, 40, 20 };
            }
        } else if (strcmp(action, "STOP_X") == 0 && state == PlayerState::WALKING) {
             vx = 0;
             if(isOnGround) state = PlayerState::IDLE;
        }
    }

    // 更新狀態與物理
    void update(float deltaTime) {
        if (health <= 0) {
             vx = 0; vy = 0; // 死亡不動
             return;
        }

        // 更新計時器
        if (attackTimer > 0) attackTimer -= deltaTime;
        if (attackCooldownTimer > 0) attackCooldownTimer -= deltaTime;
        if (hurtTimer > 0) hurtTimer -= deltaTime;
        if (invincibilityTimer > 0) invincibilityTimer -= deltaTime;

        // 狀態轉換 (基於計時器)
        if (state == PlayerState::ATTACKING && attackTimer <= 0) {
            state = PlayerState::IDLE; // 攻擊結束回站立
            hitbox = {0, 0, 0, 0}; // 清除攻擊框
        }
        if (state == PlayerState::HURT && hurtTimer <= 0) {
            state = PlayerState::IDLE; // 硬直結束回站立
        }

        // 物理更新 (硬直時不移動)
        if (state != PlayerState::HURT) {
            // 重力
            if (!isOnGround) {
                vy += GRAVITY * deltaTime;
            }
            // 更新位置
            x += vx * deltaTime;
            y += vy * deltaTime;
        } else {
             // 硬直時可能被擊退 (這裡先簡單不動)
             vx = 0;
             // 但仍然受重力影響
             if (!isOnGround) {
                vy += GRAVITY * deltaTime;
                y += vy * deltaTime;
             }
        }


        // 地面檢測
        if (y + bounds.h >= GROUND_LEVEL && vy >= 0) {
            y = GROUND_LEVEL - bounds.h;
            vy = 0;
            if (!isOnGround) { // 剛落地
                isOnGround = true;
                if (state == PlayerState::JUMPING || state == PlayerState::FALLING) {
                     state = (fabsf(vx) < 1.0f) ? PlayerState::IDLE : PlayerState::WALKING;
                } else if (state == PlayerState::HURT && hurtTimer <= 0) { // 硬直時落地且時間到
                     state = PlayerState::IDLE;
                }
                // 如果攻擊時落地? 可能需要取消攻擊或讓攻擊繼續? 這裡簡化，落地就變 IDLE/WALK
                 if (state == PlayerState::ATTACKING && attackTimer > 0) {
                      state = (fabsf(vx) < 1.0f) ? PlayerState::IDLE : PlayerState::WALKING;
                      attackTimer = 0; // 取消攻擊
                      hitbox = {0,0,0,0};
                 }
            }
        } else if (y + bounds.h < GROUND_LEVEL) {
            isOnGround = false;
            if (state == PlayerState::IDLE || state == PlayerState::WALKING) {
                 state = PlayerState::FALLING; // 從平台掉落
            }
        }

        // 邊界檢測
        if (x < 0) x = 0;
        if (x + bounds.w > SCREEN_WIDTH) x = SCREEN_WIDTH - bounds.w;
    }

    // 繪製 (簡單矩形)
    void render(SDL_Renderer* renderer, Uint8 r, Uint8 g, Uint8 b) {
        if (health <= 0) return; // 死亡不畫

        SDL_Rect worldBounds = getBoundingBox(); // 取得世界座標的身體框

        // 無敵時閃爍 (簡單範例)
        bool drawPlayer = true;
        if (invincibilityTimer > 0) {
             // 每 0.1 秒閃爍一次
             if (fmod(invincibilityTimer, 0.2f) < 0.1f) {
                 drawPlayer = false;
             }
        }

        if(drawPlayer){
            // 根據狀態畫不同顏色?
            if (state == PlayerState::HURT) {
                SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255); // 受傷紅色
            } else {
                 SDL_SetRenderDrawColor(renderer, r, g, b, 255); // 正常顏色
            }
            SDL_RenderFillRect(renderer, &worldBounds);
        }


        // 如果在攻擊狀態且攻擊框有效，畫出攻擊框
        if (state == PlayerState::ATTACKING && attackTimer > 0) {
             SDL_Rect worldHitbox = getHitboxWorld();
             SDL_SetRenderDrawColor(renderer, 255, 255, 0, 180); // 黃色半透明
             SDL_RenderFillRect(renderer, &worldHitbox);
        }
    }

    // 受到傷害
    void takeDamage(int damage) {
        if (invincibilityTimer > 0 || health <= 0) return; // 無敵或已死亡

        health -= damage;
        printf("Player took %d damage, health: %d\n", damage, health);

        if (health <= 0) {
            health = 0;
            printf("Player defeated!\n");
            // 可以在這裡加死亡狀態，但目前只會停止動作
        } else {
            state = PlayerState::HURT;
            hurtTimer = HURT_DURATION;
            invincibilityTimer = HURT_INVINCIBILITY;
            vx = 0; // 受傷時停止移動
            vy = -100.0f; // 輕微向上彈 (可選)
            isOnGround = false;
            // 清除攻擊狀態
            isAttacking = false;
            attackTimer = 0;
            hitbox = {0,0,0,0};
        }
    }

    // 取得世界座標的身體框
    SDL_Rect getBoundingBox() const {
        return {(int)x + bounds.x, (int)y + bounds.y, bounds.w, bounds.h};
    }

    // 取得世界座標的攻擊框 (如果有效)
    SDL_Rect getHitboxWorld() const {
         if (state == PlayerState::ATTACKING && attackTimer > 0) {
             // (這裡可以加入更精確的 Hitbox 生效時間判斷，但先簡化)
             return {(int)x + hitbox.x, (int)y + hitbox.y, hitbox.w, hitbox.h};
         } else {
             return {0, 0, 0, 0}; // 無效攻擊框
         }
    }
};


// --- 主函數 ---
int main(int argc, char* argv[]) {
    // 初始化 SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL 初始化失敗: %s\n", SDL_GetError());
        return -1;
    }

    // 創建視窗
    SDL_Window* window = SDL_CreateWindow("Simple Fighter C++", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("窗口創建失敗: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    // 創建渲染器
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        printf("渲染器創建失敗: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // 創建玩家物件
    Player player1(100.0f, GROUND_LEVEL - PLAYER_HEIGHT, 1); // P1 在左邊，面向右
    Player player2(SCREEN_WIDTH - 100.0f - PLAYER_WIDTH, GROUND_LEVEL - PLAYER_HEIGHT, -1); // P2 在右邊，面向左

    // 時間控制
    Uint32 lastFrameTime = SDL_GetTicks();
    float deltaTime = 0.0f;

    // 主循環標誌
    bool isRunning = true;
    SDL_Event event;

    // 主循環
    while (isRunning) {
        // --- 計算 Delta Time ---
        Uint32 currentFrameTime = SDL_GetTicks();
        deltaTime = (currentFrameTime - lastFrameTime) / 1000.0f;
        lastFrameTime = currentFrameTime;
        // 防止 deltaTime 過大 (例如視窗被拖動或除錯中斷)
        if (deltaTime > 0.05f) {
            deltaTime = 0.05f;
        }

        // --- 事件處理 ---
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            }
            // (可以加入按鍵按下的事件處理，用於觸發一次性動作如跳躍/攻擊)
            // 但這裡使用狀態檢查，更適合按住移動
        }

        // --- 輸入處理 (持續按住) ---
        const Uint8* keystates = SDL_GetKeyboardState(NULL);

        // P1 控制 (WASD + J攻擊)
        bool p1Left = keystates[SDL_SCANCODE_A];
        bool p1Right = keystates[SDL_SCANCODE_D];
        if (keystates[SDL_SCANCODE_W]) player1.handleAction("JUMP");
        if (keystates[SDL_SCANCODE_J]) player1.handleAction("ATTACK");

        if (p1Left) player1.handleAction("LEFT");
        else if (p1Right) player1.handleAction("RIGHT");
        else player1.handleAction("STOP_X"); // 沒有左右鍵按下時停止水平移動

        // P2 控制 (方向鍵 + Numpad 1攻擊)
         bool p2Left = keystates[SDL_SCANCODE_LEFT];
         bool p2Right = keystates[SDL_SCANCODE_RIGHT];
         if (keystates[SDL_SCANCODE_UP]) player2.handleAction("JUMP");
         if (keystates[SDL_SCANCODE_KP_1]) player2.handleAction("ATTACK");

         if (p2Left) player2.handleAction("LEFT");
         else if (p2Right) player2.handleAction("RIGHT");
         else player2.handleAction("STOP_X");


        // --- 更新遊戲狀態 ---
        player1.update(deltaTime);
        player2.update(deltaTime);

        // --- 碰撞檢測 ---
        SDL_Rect p1Hitbox = player1.getHitboxWorld();
        SDL_Rect p2Bounds = player2.getBoundingBox();
        if (p1Hitbox.w > 0 && SDL_HasIntersection(&p1Hitbox, &p2Bounds)) {
            player2.takeDamage(ATTACK_DAMAGE); // P1 攻擊命中 P2
        }

        SDL_Rect p2Hitbox = player2.getHitboxWorld();
        SDL_Rect p1Bounds = player1.getBoundingBox();
         if (p2Hitbox.w > 0 && SDL_HasIntersection(&p2Hitbox, &p1Bounds)) {
            player1.takeDamage(ATTACK_DAMAGE); // P2 攻擊命中 P1
        }


        // --- 繪圖 ---
        // 清空畫面 (深藍色背景)
        SDL_SetRenderDrawColor(renderer, 0, 0, 50, 255);
        SDL_RenderClear(renderer);

        // 畫地面線 (白色)
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawLine(renderer, 0, GROUND_LEVEL, SCREEN_WIDTH, GROUND_LEVEL);

        // 畫玩家 (P1 紅色, P2 藍色)
        player1.render(renderer, 255, 0, 0);
        player2.render(renderer, 0, 0, 255);

        // 畫血條 (簡單版)
        SDL_Rect p1HealthBg = {10, 10, 200, 15};
        SDL_Rect p1HealthFg = {10, 10, (int)(200 * (float)player1.health / 100.0f), 15};
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255); SDL_RenderFillRect(renderer, &p1HealthBg);
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); SDL_RenderFillRect(renderer, &p1HealthFg);

        SDL_Rect p2HealthBg = {SCREEN_WIDTH - 10 - 200, 10, 200, 15};
        int p2FgWidth = (int)(200 * (float)player2.health / 100.0f);
        SDL_Rect p2HealthFg = {SCREEN_WIDTH - 10 - p2FgWidth, 10, p2FgWidth, 15};
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255); SDL_RenderFillRect(renderer, &p2HealthBg);
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); SDL_RenderFillRect(renderer, &p2HealthFg);


        // 更新畫面
        SDL_RenderPresent(renderer);
    }

    // --- 清理資源 ---
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
message.txt
15 KB