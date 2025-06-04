#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <vector>
#include <string> 
#include "Player.h" // 包含 Player
#include "AudioManager.h"
#include <fstream>
#include <ctime>
#include <deque>

// --- 遊戲狀態列舉 ---
enum class GameState {
    START_SCREEN,    // 開始畫面
    CHARACTER_SELECTION, // 角色選擇介面
    GLOVE_SELECTION, // 拳套選擇介面
    ROUND_STARTING,  // 回合即將開始 (顯示 Round X) - 目前未使用，直接進入 PLAYING
    PLAYING,         // 回合進行中
    ROUND_OVER,      // 回合結束 (顯示結果)
    MATCH_OVER,      // 比賽結束 (顯示贏家)
    PAUSED,          // 遊戲暫停
    CHARACTER_INFO   // 角色介紹畫面
};

// --- 新增：混亂模式事件型別 ---
enum class ChaosEventType {
    NONE,
    CONTROL_REVERSE,
    HP_SWAP
};

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

// 遊戲記錄結構
struct GameRecord {
    std::string timestamp;
    std::string p1Character;
    std::string p2Character;
    int winnerIndex;  // -1: 平手, 0: P1勝, 1: P2勝
    float gameTime;
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

    // 繪製遊戲畫面
    void render();
    void renderStartScreen(); // 繪製開始畫面
    void renderCharacterSelection(); // 繪製角色選擇介面
    void renderGloveSelection(); // 繪製拳套選擇介面
    void renderPauseMenu();     // 繪製暫停選單
    void renderCharacterInfo(); // 繪製角色介紹畫面

    // 碰撞檢測
    void checkCollisions();
    bool checkProjectilePlayerCollision(const Projectile& proj, Player& player);
    void checkPlayerCollision(Player& p1, Player& p2);

    // --- 回合管理函式 ---
    void startNewRound();      // 開始新回合的準備工作
    void endRound(int winnerPlayerIndex); // 處理回合結束 -1:平手, 0:P1勝, 1:P2勝
    void resetPlayersForRound(); // 重置玩家位置、血量等
    void checkForMatchWinner(); // 檢查是否有人贏得整場比賽

    // --- 拳套選擇介面相關 ---
    void handleGloveSelection(); // 處理拳套選擇的輸入
    void startGameAfterGloveSelection(); // 拳套選擇完成後開始遊戲

    // --- 角色選擇介面相關 ---
    void handleCharacterSelection(); // 處理角色選擇的輸入
    void startGameAfterCharacterSelection(); // 角色選擇完成後進入拳套選擇

    // SDL 相關
    SDL_Window* window;
    SDL_Renderer* renderer;

    // 遊戲物件 (使用 vector 以便未來擴充)
    std::vector<Player> players; // 目前只有兩個玩家
    std::vector<Projectile> projectiles;

    // 遊戲狀態
    bool isRunning;
    Uint32 lastFrameTime;

    // --- 回合制相關變數 ---
    GameState currentGameState; // 目前的遊戲狀態
    int currentRound;           // 目前是第幾回合 (從 1 開始)
    int playerWins[2];          // 記錄 P1 和 P2 的勝利回合數 (索引 0 為 P1, 1 為 P2)
    float roundTimer;           // 目前回合的剩餘時間
    float roundOverTimer;       // 回合結束狀態的計時器
    int roundWinnerIndex;       // 記錄本回合勝利者的索引 (-1 表示平手或無)

    // --- 拳套選擇介面相關變數 ---
    int selectedGloveIndex[2];  // 兩個玩家選擇的拳套索引 (0: 10oz, 1: 14oz, 2: 18oz)
    bool gloveSelectionConfirmed[2]; // 兩個玩家是否已確認選擇
    bool isPaused;  // 是否處於暫停狀態

    // --- 角色選擇介面相關變數 ---
    int selectedCharacterIndex[2];  // 兩個玩家選擇的角色索引 (0: 統神, 1: 國動)
    bool characterSelectionConfirmed[2]; // 兩個玩家是否已確認選擇

    // --- 新增：暫停相關變數 ---
    SDL_Rect continueButton;    // 繼續遊戲按鈕
    SDL_Rect restartButton;     // 重新開始按鈕
    SDL_Rect exitButton;        // 結束遊戲按鈕
    SDL_Rect characterInfoButton; // 角色介紹按鈕
    SDL_Rect chaosModeButton;   // 混亂模式按鈕

    // --- 新增：暫停相關函式 ---
    void handlePauseMenu();     // 處理暫停選單的點擊

    // 生成氣功
    void spawnProjectile(float startX, float startY, int direction, int ownerIndex);
    
    // --- 新增：簡易 UI 繪製函式 ---
    void renderRoundInfo(); // 繪製回合數、計時器、勝利標記

    // 新增：遊戲記錄相關函式
    void saveGameRecord();
    void loadGameRecords();
    void renderGameRecords();  // 顯示遊戲記錄
    void addRecordButton();    // 新增記錄按鈕

    // --- 混亂模式相關 ---
    bool isChaosMode = false; // 是否啟用混亂模式
    float chaosEventTimer = 0.0f; // 混亂事件倒數計時
    ChaosEventType chaosEvent = ChaosEventType::NONE; // 目前混亂事件
    float chaosEventShowTimer = 0.0f; // 混亂事件名稱顯示倒數（秒）
    int chaosBgIndex = 0; // 混亂模式下背景交替（0: image0.png, 1: image.png）
    float chaosEventTimerMax = 15.0f; // 混亂事件冷卻條最大值

    // 新增：遊戲記錄相關變數
    std::deque<GameRecord> gameRecords;  // 使用 deque 來儲存最近的遊戲記錄
    SDL_Rect recordButton;     // 記錄按鈕
    bool showRecords;          // 是否顯示記錄
    const int MAX_RECORDS = 5; // 最多保存5局記錄
    float menuCooldownTimer;  // 新增：選單冷卻計時器
    static constexpr float MENU_COOLDOWN = 0.5f;  // 新增：冷卻時間（秒）

    // 新增：按鍵處理和冷卻條渲染
    void handleKeyPress(SDL_Keycode key);

private:
    // 處理事件
    void handleEvents();

    // 更新遊戲狀態
    void update(float deltaTime);
};

#endif // GAME_H