#include "Game.h"
#include "Constants.h"
#include "TextureManager.h"
#include "AnimationData.h"
#include "AudioManager.h"
#include <stdio.h>
#include <vector>
#include <algorithm>

Game::Game() : window(nullptr), renderer(nullptr), isRunning(false), lastFrameTime(0) {}

Game::~Game() {
    // cleanup() 應該已被呼叫，但以防萬一
    cleanup();
}

bool Game::initialize() {
    printf("Initializing Game...\n");

    // --- SDL 初始化 ---
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL Init Error: %s\n", SDL_GetError());
        return false;
    }

    // --- 初始化 SDL_mixer (透過 AudioManager) ---
    if (!AudioManager::init()) { // 使用預設參數初始化
        // 即使 AudioManager 初始化失敗，遊戲可能仍能繼續 (沒有聲音)
        printf("Warning: AudioManager initialization failed. Continuing without sound.\n");
    }

    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        printf("SDL_image Init Warning: %s\n", IMG_GetError());
        // 不退出，但圖片可能無法載入
    }

    // --- 創建視窗 ---
    window = SDL_CreateWindow("Advanced Fighter Structure", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window Creation Error: %s\n", SDL_GetError());
        return false;
    }

    // --- 創建渲染器 ---
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        printf("Renderer Creation Error: %s\n", SDL_GetError());
        return false;
    }

    // --- 載入紋理 (使用 TextureManager) ---
    printf("Loading textures...\n");
    // **注意:** Texture ID 和 Character ID 可以相同或不同
    bool success = true;
    success &= TextureManager::loadTexture(renderer, "background", "assets/image.png");
    success &= TextureManager::loadTexture(renderer, "blockman_sprites", "assets/asiagodton.png");
    // success &= TextureManager::loadTexture(renderer, "ninja_sprites", "assets/ninja.png"); // 未來新增角色
    success &= TextureManager::loadTexture(renderer, "projectile_sprites", "assets/asiagodton.png");

    if (!success) {
        printf("Failed to load essential textures.\n");
        return false;
    }

    // --- 載入音訊 ---
    printf("Loading audio...\n");
    bool audioSuccess = true;
    // vvvvv 請換成你實際的檔案路徑和想要的 ID vvvvv
    audioSuccess &= AudioManager::loadMusic("bgm", "assets/bgm.wav");
    audioSuccess &= AudioManager::loadSound("hurt", "assets/hurt0.wav");
    audioSuccess &= AudioManager::loadSound("fire", "assets/lose0.wav");
    audioSuccess &= AudioManager::loadSound("death", "assets/lose1.wav");
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    if (!audioSuccess) {
         printf("Warning: Failed to load some audio files.\n");
         // 不一定要 return false，看你是否允許無聲進行遊戲
    }
     // 設定音量 (可選, 0-128)
    AudioManager::setMusicVolume(64); // BGM 音量設為一半
    // AudioManager::setSoundVolume("hurt", 100); // 可以單獨設 SFX 音量

    // --- 初始化動畫數據 ---
    printf("Initializing animations...\n");
    AnimationDataManager::initializeBlockManAnimations();
    // AnimationDataManager::initializeNinjaAnimations(); // 未來新增角色

    // --- 創建玩家物件 ---
    printf("Creating players...\n");
    // 使用 Player 建構子指定 Character ID 和 Texture ID
    players.emplace_back(100.0f, GROUND_LEVEL - PLAYER_LOGIC_HEIGHT, 1, "BlockMan", "blockman_sprites");
    players.emplace_back(SCREEN_WIDTH - 100.0f - PLAYER_LOGIC_WIDTH, GROUND_LEVEL - PLAYER_LOGIC_HEIGHT, -1, "BlockMan", "blockman_sprites");
    // players.emplace_back(200.0f, GROUND_LEVEL - PLAYER_LOGIC_HEIGHT, 1, "Ninja", "ninja_sprites"); // 未來新增角色

    isRunning = true;
    lastFrameTime = SDL_GetTicks();
    printf("Game Initialization Successful.\n");

    // --- 開始播放 BGM ---
    AudioManager::playMusic("bgm", -1); // -1 表示無限循環

    printf("Game Initialization Successful.\n");

    return true;
}

void Game::run() {
    printf("Starting Game Loop...\n");
    while (isRunning) {
        // --- 計算 Delta Time ---
        Uint32 currentFrameTime = SDL_GetTicks();
        float deltaTime = (currentFrameTime - lastFrameTime) / 1000.0f;
        lastFrameTime = currentFrameTime;
        if (deltaTime > 0.05f) deltaTime = 0.05f; // Delta time capping

        // --- 處理事件 ---
        handleEvents();

        // --- 更新狀態 ---
        update(deltaTime);

        // --- 繪製畫面 ---
        render();
    }
    printf("Exiting Game Loop.\n");
}

void Game::handleEvents() {
    SDL_Event event;
    // --- 先處理單次觸發事件 (例如鍵盤放開) ---
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            isRunning = false;
        }
        // --- 處理按鍵放開來結束格擋 ---
        if (event.type == SDL_KEYUP) {
            if (players.size() >= 1 && event.key.keysym.sym == SDLK_k) { // P1 放開 K
                if (players[0].state == Player::PlayerState::BLOCKING) {
                    players[0].handleAction("STOP_BLOCK");
                }
            }
            if (players.size() >= 2 && event.key.keysym.sym == SDLK_KP_2) { // P2 放開 2 
                 if (players[1].state == Player::PlayerState::BLOCKING) {
                    players[1].handleAction("STOP_BLOCK");
                }
            }
        }
        if (event.type == SDL_KEYDOWN) {
            // Player 1 按 U 發射
            if (players.size() >= 1 && event.key.keysym.sym == SDLK_u && !event.key.repeat) { // !event.key.repeat 避免按住連續觸發
                players[0].handleAction("FIRE_PROJECTILE");
            }
            // Player 2 按 Numpad 4 發射
            if (players.size() >= 2 && event.key.keysym.sym == SDLK_KP_4 && !event.key.repeat) {
                players[1].handleAction("FIRE_PROJECTILE");
            }
        }
    }

    // --- 再處理持續按壓的狀態 ---
    const Uint8* keystates = SDL_GetKeyboardState(NULL);

    // --- Player 1 控制 ---
    if (players.size() >= 1) {
        Player& p1 = players[0];
        // 格擋優先判斷 (按下 K 且不在冷卻/受傷/空中等)
        if (keystates[SDL_SCANCODE_K] && p1.blockCooldownTimer <= 0 && p1.isOnGround &&
            p1.state != Player::PlayerState::HURT && p1.state != Player::PlayerState::ATTACKING)
        {
            printf("[Input] P1 trying to BLOCK. Current state: %d\n", static_cast<int>(p1.state));
            p1.handleAction("BLOCK");
        }
        // 只有在非格擋狀態下才處理其他動作
        else if (p1.state != Player::PlayerState::BLOCKING) {
            bool p1Left = keystates[SDL_SCANCODE_A];
            bool p1Right = keystates[SDL_SCANCODE_D];
            if (keystates[SDL_SCANCODE_W]) p1.handleAction("JUMP");
            if (keystates[SDL_SCANCODE_J]) p1.handleAction("ATTACK");
            if (p1Left) p1.handleAction("LEFT");
            else if (p1Right) p1.handleAction("RIGHT");
            else p1.handleAction("STOP_X");
        }
        // 如果正在格擋，確保速度為 0 (雖然 Player::update 也會做)
         else if (p1.state == Player::PlayerState::BLOCKING) {
             p1.vx = 0;
         }
    }

    // --- Player 2 控制 ---
    if (players.size() >= 2) {
        Player& p2 = players[1];
        // 格擋優先判斷 (按下 2 且不在冷卻/受傷/空中等)
         if (keystates[SDL_SCANCODE_KP_2] && p2.blockCooldownTimer <= 0 && p2.isOnGround &&
             p2.state != Player::PlayerState::HURT && p2.state != Player::PlayerState::ATTACKING)
         {
            p2.handleAction("BLOCK");
         }
         // 只有在非格擋狀態下才處理其他動作
         else if (p2.state != Player::PlayerState::BLOCKING) {
            bool p2Left = keystates[SDL_SCANCODE_LEFT];
            bool p2Right = keystates[SDL_SCANCODE_RIGHT];
            if (keystates[SDL_SCANCODE_UP]) p2.handleAction("JUMP");
             // 假設 P2 攻擊是用小鍵盤 1
            if (keystates[SDL_SCANCODE_KP_1]) p2.handleAction("ATTACK");
            if (p2Left) p2.handleAction("LEFT");
            else if (p2Right) p2.handleAction("RIGHT");
            else p2.handleAction("STOP_X");
        }
         // 如果正在格擋，確保速度為 0
         else if (p2.state == Player::PlayerState::BLOCKING) {
             p2.vx = 0;
         }
    }
}

void Game::update(float deltaTime) {
    // 更新所有玩家
    for (Player& player : players) {
        player.update(deltaTime);
    }

    for (int i = 0; i < players.size(); ++i) {
        if (players[i].shouldFireProjectile) {
            // 檢查是否真的處於攻擊狀態 (以防萬一狀態被其他邏輯改變)
            // 或者可以在動畫特定幀生成氣功，這裡先簡單處理：只要標記為真就生成
            printf("Game detected Player %d wants to fire projectile.\n", i);
            spawnProjectile(players[i].x, players[i].y, players[i].direction, i);
            players[i].shouldFireProjectile = false; // <-- 重要：重置標記！
        }
    }
    
    for (Projectile& proj : projectiles) {
        proj.update(deltaTime);
    }
    // 移除不再活躍的氣功 (使用 erase-remove idiom)
    projectiles.erase(
        std::remove_if(projectiles.begin(), projectiles.end(),
                       [](const Projectile& p){ return !p.isActive; }),
        projectiles.end()
    );

    // 碰撞檢測
    checkCollisions();
}

void Game::checkCollisions() {
    // 簡易的玩家互毆碰撞檢測
    if (players.size() < 2) return; // 至少需要兩個玩家

    Player& p1 = players[0];
    Player& p2 = players[1];

    SDL_Rect p1Hitbox = p1.getHitboxWorld();
    SDL_Rect p2Bounds = p2.getBoundingBox();

    // P1 攻擊 P2
    if (p1Hitbox.w > 0 && p2.invincibilityTimer <= 0 && SDL_HasIntersection(&p1Hitbox, &p2Bounds)) {
        p2.takeDamage(ATTACK_DAMAGE);
        // 可以加點擊退效果或讓 P1 的攻擊失效一次
        // p1.attackTimer = 0; // 如果攻擊命中一次就結束
    }

    SDL_Rect p2Hitbox = p2.getHitboxWorld();
    SDL_Rect p1Bounds = p1.getBoundingBox();

    // P2 攻擊 P1
    if (p2Hitbox.w > 0 && p1.invincibilityTimer <= 0 && SDL_HasIntersection(&p2Hitbox, &p1Bounds)) {
        p1.takeDamage(ATTACK_DAMAGE);
        // p2.attackTimer = 0;
    }

    for (Projectile& proj : projectiles) {
        if (!proj.isActive) continue; // 只檢查活躍的氣功

        SDL_Rect projBox = proj.getBoundingBox();

        // 檢查氣功是否打到 P1
        if (proj.ownerPlayerIndex != 0 && p1.isAlive() && p1.invincibilityTimer <= 0) { // 氣功不是P1發的，P1還活著且非無敵
            SDL_Rect p1Bounds = p1.getBoundingBox();
            if (SDL_HasIntersection(&projBox, &p1Bounds)) {
                printf("Projectile hit Player 1!\n");
                p1.takeDamage(PROJECTILE_DAMAGE); // 使用常數
                proj.isActive = false; // 氣功消失
                continue; // 這個氣功的任務完成了，檢查下一個氣功
            }
        }

        // 檢查氣功是否打到 P2
        if (proj.ownerPlayerIndex != 1 && p2.isAlive() && p2.invincibilityTimer <= 0) { // 氣功不是P2發的，P2還活著且非無敵
             SDL_Rect p2Bounds = p2.getBoundingBox();
            if (SDL_HasIntersection(&projBox, &p2Bounds)) {
                printf("Projectile hit Player 2!\n");
                p2.takeDamage(PROJECTILE_DAMAGE); // 使用常數
                proj.isActive = false; // 氣功消失
                // continue; // 不需要，因為內層循環會結束
            }
        }
    }

    // 未來可以加入玩家與場景物件、玩家與飛行道具等的碰撞
}


void Game::render() {
    // 1. 清空畫面
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // 2. 繪製背景
    SDL_Texture* bgTex = TextureManager::getTexture("background");
    if (bgTex) {
         SDL_RenderCopy(renderer, bgTex, NULL, NULL);
    } else {
         SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
         SDL_RenderDrawLine(renderer, 0, GROUND_LEVEL, SCREEN_WIDTH, GROUND_LEVEL);
    }

    // 3. 繪製所有玩家
    for (Player& player : players) {
        player.render(renderer); // 呼叫 Player 自己的 render
    }

    //繪製氣功
    SDL_Texture* projTexture = TextureManager::getTexture("projectile_sprites"); // 取得氣功紋理
    if (projTexture) { // 確保紋理存在
        for (Projectile& proj : projectiles) {
            proj.render(renderer, projTexture); // 呼叫氣功自己的 render
        }
    } else {
        // 如果紋理不存在，可以印個錯誤訊息或畫個方塊代替
        // printf("Warning: Projectile texture not found!\n");
    }


    // 4. 繪製 UI (例如血條)
    if (players.size() >= 1) {
        Player& p1 = players[0];
        SDL_Rect p1HealthBg = {10, 10, 200, 15};
        SDL_Rect p1HealthFg = {10, 10, (int)(200.0f * (float)p1.health / PLAYER_DEFAULT_HEALTH), 15};
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255); SDL_RenderFillRect(renderer, &p1HealthBg);
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); SDL_RenderFillRect(renderer, &p1HealthFg);
    }
     if (players.size() >= 2) {
        Player& p2 = players[1];
        SDL_Rect p2HealthBg = {SCREEN_WIDTH - 10 - 200, 10, 200, 15};
        int p2FgWidth = (int)(200.0f * (float)p2.health / PLAYER_DEFAULT_HEALTH);
        SDL_Rect p2HealthFg = {SCREEN_WIDTH - 10 - p2FgWidth, 10, p2FgWidth, 15};
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255); SDL_RenderFillRect(renderer, &p2HealthBg);
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); SDL_RenderFillRect(renderer, &p2HealthFg);
    }
    // 可以加入分數、計時器等其他 UI 元素

    // 5. 更新畫面到螢幕
    SDL_RenderPresent(renderer);
}

void Game::spawnProjectile(float startX, float startY, int direction, int ownerIndex) {
    Projectile p;

    // 計算初始位置 (例如：從玩家中心靠前一點發射)
    float spawnOffsetX = (direction == 1) ? (PLAYER_LOGIC_WIDTH * 0.8f) : (-PROJECTILE_HITBOX_W - PLAYER_LOGIC_WIDTH * 0.2f);
    float spawnOffsetY = PLAYER_LOGIC_HEIGHT / 2.0f - PROJECTILE_HITBOX_H / 2.0f; // 垂直置中

    p.x = startX + spawnOffsetX;
    p.y = startY + spawnOffsetY;
    p.vx = PROJECTILE_SPEED * direction; // 設定水平速度和方向
    p.ownerPlayerIndex = ownerIndex;
    p.isActive = true;
    p.textureId = "projectile_sprites"; // 確保這個 ID 和載入時一致

    // 設定氣功在精靈圖上的來源矩形 (使用 Constants.h 的值)
    p.srcRect = {PROJECTILE_SRC_X, PROJECTILE_SRC_Y, PROJECTILE_SRC_W, PROJECTILE_SRC_H};

    projectiles.push_back(p); // 加入到遊戲的氣功列表中
    printf("Spawned projectile for player %d at (%.1f, %.1f) with vx=%.1f\n", ownerIndex, p.x, p.y, p.vx);
}


void Game::cleanup() {
    printf("Cleaning up Game...\n");
    // 釋放紋理 (透過 TextureManager)
    TextureManager::unloadAllTextures();

    //清理音訊
    AudioManager::cleanup();

    // 清理 SDL 資源
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    // 退出 SDL 子系統
    IMG_Quit();
    SDL_Quit();
    printf("Game cleanup complete.\n");
}

