#include "Game.h"
#include "Constants.h"
#include "TextureManager.h"
#include "AnimationData.h"
#include "AudioManager.h"
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <cmath>
#include <SDL2/SDL_ttf.h>
#include <ctime>
#include <fstream>

// 添加字體相關的全局變量
TTF_Font* buttonFont = nullptr;
SDL_Color textColor = {0, 0, 0, 255}; // 黑色文字

Game::Game() :
    window(nullptr), renderer(nullptr), isRunning(false), lastFrameTime(0),
    currentGameState(GameState::START_SCREEN),
    currentRound(1),
    roundTimer(ROUND_DURATION),
    roundOverTimer(0.0f),
    roundWinnerIndex(-1),
    selectedGloveIndex{0, 0},
    gloveSelectionConfirmed{false, false},
    selectedCharacterIndex{0, 0},
    characterSelectionConfirmed{false, false},
    isPaused(false),
    showRecords(false),
    menuCooldownTimer(0.0f),  // 新增：選單冷卻計時器
    isChaosMode(false),
    chaosEvent(ChaosEventType::NONE),
    chaosEventTimer(0.0f),
    chaosEventShowTimer(0.0f),
    chaosBgIndex(0)
{
    // 初始化玩家勝利回合數
    playerWins[0] = 0;
    playerWins[1] = 0;
    printf("Game constructor: Initial state set to START_SCREEN\n");

    // 初始化暫停選單按鈕位置
    int buttonWidth = 200;
    int buttonHeight = 50;
    int buttonGap = 20; // 更小的間距
    int totalButtons = 4;
    int totalHeight = buttonHeight * totalButtons + buttonGap * (totalButtons - 1);
    int centerX = SCREEN_WIDTH / 2 - buttonWidth / 2;
    int startY = SCREEN_HEIGHT / 2 - totalHeight / 2;

    continueButton = {centerX, startY, buttonWidth, buttonHeight};
    chaosModeButton = {centerX, startY + (buttonHeight + buttonGap) * 1, buttonWidth, buttonHeight};
    characterInfoButton = {centerX, startY + (buttonHeight + buttonGap) * 2, buttonWidth, buttonHeight};
    recordButton = {centerX, startY + (buttonHeight + buttonGap) * 3, buttonWidth, buttonHeight};

    // 繪製按鈕底色與邊框
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &continueButton);
    SDL_RenderFillRect(renderer, &chaosModeButton);
    SDL_RenderFillRect(renderer, &characterInfoButton);
    SDL_RenderFillRect(renderer, &recordButton);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &continueButton);
    SDL_RenderDrawRect(renderer, &chaosModeButton);
    SDL_RenderDrawRect(renderer, &characterInfoButton);
    SDL_RenderDrawRect(renderer, &recordButton);

    // 繪製按鈕文字
    SDL_Surface* startText = TTF_RenderUTF8_Blended(buttonFont, "開始遊戲", textColor);
    SDL_Surface* chaosText = TTF_RenderUTF8_Blended(buttonFont, "混亂模式", textColor);
    SDL_Surface* infoText = TTF_RenderUTF8_Blended(buttonFont, "角色介紹", textColor);
    SDL_Surface* recordText = TTF_RenderUTF8_Blended(buttonFont, "查看遊戲記錄", textColor);
    
    if (startText && chaosText && infoText && recordText) {
        SDL_Texture* startTexture = SDL_CreateTextureFromSurface(renderer, startText);
        SDL_Texture* chaosTexture = SDL_CreateTextureFromSurface(renderer, chaosText);
        SDL_Texture* infoTexture = SDL_CreateTextureFromSurface(renderer, infoText);
        SDL_Texture* recordTexture = SDL_CreateTextureFromSurface(renderer, recordText);
        
        if (startTexture && chaosTexture && infoTexture && recordTexture) {
            SDL_Rect startTextRect = {
                continueButton.x + (continueButton.w - startText->w) / 2,
                continueButton.y + (continueButton.h - startText->h) / 2,
                startText->w,
                startText->h
            };
            SDL_Rect chaosTextRect = {
                chaosModeButton.x + (chaosModeButton.w - chaosText->w) / 2,
                chaosModeButton.y + (chaosModeButton.h - chaosText->h) / 2,
                chaosText->w,
                chaosText->h
            };
            SDL_Rect infoTextRect = {
                characterInfoButton.x + (characterInfoButton.w - infoText->w) / 2,
                characterInfoButton.y + (characterInfoButton.h - infoText->h) / 2,
                infoText->w,
                infoText->h
            };
            SDL_Rect recordTextRect = {
                recordButton.x + (recordButton.w - recordText->w) / 2,
                recordButton.y + (recordButton.h - recordText->h) / 2,
                recordText->w,
                recordText->h
            };
            
            SDL_RenderCopy(renderer, startTexture, nullptr, &startTextRect);
            SDL_RenderCopy(renderer, chaosTexture, nullptr, &chaosTextRect);
            SDL_RenderCopy(renderer, infoTexture, nullptr, &infoTextRect);
            SDL_RenderCopy(renderer, recordTexture, nullptr, &recordTextRect);
            
            SDL_DestroyTexture(startTexture);
            SDL_DestroyTexture(chaosTexture);
            SDL_DestroyTexture(infoTexture);
            SDL_DestroyTexture(recordTexture);
        }
        
        SDL_FreeSurface(startText);
        SDL_FreeSurface(chaosText);
        SDL_FreeSurface(infoText);
        SDL_FreeSurface(recordText);
    }
}

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

    // 初始化 SDL_ttf
    if (TTF_Init() < 0) {
        printf("TTF Init Error: %s\n", TTF_GetError());
        return false;
    }

    // 載入字體
    buttonFont = TTF_OpenFont("assets/fonts/msjh.ttf", 24); // 使用微軟正黑體，大小24
    if (!buttonFont) {
        printf("Font Load Error: %s\n", TTF_GetError());
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
    window = SDL_CreateWindow("StreetFighter", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
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
    success &= TextureManager::loadTexture(renderer, "start_screen", "assets/start_picture.jpg");
    success &= TextureManager::loadTexture(renderer, "background", "assets/image.png");
    success &= TextureManager::loadTexture(renderer, "background0", "assets/image0.png"); // 新增
    success &= TextureManager::loadTexture(renderer, "blockman_sprites", "assets/asiagodton.png");
    success &= TextureManager::loadTexture(renderer, "projectile_sprites", "assets/asiagodton.png");
    success &= TextureManager::loadTexture(renderer, "victory_screen", "assets/asiagodtonewin.png"); 
    success &= TextureManager::loadTexture(renderer, "godon_sprites", "assets/godon.png");

    if (!success) {
        printf("Failed to load essential textures.\n");
        return false;
    }

    // --- 載入音訊 ---
    printf("Loading audio...\n");
    bool audioSuccess = true;
    // 載入 BGM
    audioSuccess &= AudioManager::loadMusic("bgm", "assets/bgm.wav");
    // 載入多個受傷音效
    audioSuccess &= AudioManager::loadSound("hurt0", "assets/hurt0.wav");
    audioSuccess &= AudioManager::loadSound("hurt1", "assets/hurt1.wav");
    // 載入多個發射音效
    audioSuccess &= AudioManager::loadSound("fire0", "assets/lose0.wav");
    audioSuccess &= AudioManager::loadSound("fire1", "assets/lose1.wav");
    // 載入多個死亡音效
    audioSuccess &= AudioManager::loadSound("death0", "assets/lose0.wav");
    audioSuccess &= AudioManager::loadSound("death1", "assets/lose1.wav");
    audioSuccess &= AudioManager::loadSound("victory_sfx", "assets/victory0.wav");

    if (!audioSuccess) {
        printf("Warning: Some audio files failed to load.\n");
    }

    // 播放背景音樂
    AudioManager::playMusic("bgm", -1); // -1 表示無限循環

    // 設定音量 (可選, 0-128)
    AudioManager::setMusicVolume(64); // BGM 音量設為一半
    AudioManager::setSoundVolume("hurt0", 127);

    // --- 初始化動畫數據 ---
    printf("Initializing animations...\n");
    AnimationDataManager::initializeBlockManAnimations();
    AnimationDataManager::initializeGodonAnimations();

    // --- 創建玩家物件 ---
    printf("Creating players...\n");
    // 使用 Player 建構子指定 Character ID 和 Texture ID
    players.emplace_back(100.0f, GROUND_LEVEL - PLAYER_LOGIC_HEIGHT, 1, "BlockMan", "blockman_sprites");
    players.emplace_back(SCREEN_WIDTH - 100.0f - PLAYER_LOGIC_WIDTH, GROUND_LEVEL - PLAYER_LOGIC_HEIGHT, -1, "Godon", "godon_sprites");

    isRunning = true;
    lastFrameTime = SDL_GetTicks();

    // 保持 START_SCREEN 狀態，不立即開始遊戲
    printf("Game Initialization Successful. Waiting at start screen.\n");

    // --- 在initialize()音效與紋理載入區段後面加： ---
    // BlockMan音效
    AudioManager::loadSound("blockman_hurt0", "assets/hurt0.wav");
    AudioManager::loadSound("blockman_hurt1", "assets/hurt1.wav");
    AudioManager::loadSound("blockman_hurt2", "assets/hurt2.wav");
    AudioManager::loadSound("blockman_fire0", "assets/fire0.wav");
    AudioManager::loadSound("blockman_death0", "assets/lose0.wav");
    AudioManager::loadSound("blockman_death1", "assets/lose1.wav");
    AudioManager::loadSound("blockman_victory0", "assets/victory0.wav");
    TextureManager::loadTexture(renderer, "blockman_win_screen", "assets/asiagodtonewin.png");
    // Godon音效
    AudioManager::loadSound("godon_hurt0", "assets/1hurt0.wav");
    AudioManager::loadSound("godon_fire0", "assets/1fire0.wav");
    AudioManager::loadSound("godon_fire1", "assets/1fire1.wav");
    AudioManager::loadSound("godon_death0", "assets/1lose0.wav");
    AudioManager::loadSound("godon_death1", "assets/1lose1.wav");
    AudioManager::loadSound("godon_victory0", "assets/1victory0.wav");
    TextureManager::loadTexture(renderer, "godon_win_screen", "assets/godonwin.png");

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
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            isRunning = false;
            return;
        }

        // 處理 ESC 鍵按下事件
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
            if (currentGameState == GameState::PLAYING) {
                currentGameState = GameState::PAUSED;
                printf("Game paused\n");
            } else if (currentGameState == GameState::PAUSED) {
                currentGameState = GameState::PLAYING;
                printf("Game resumed\n");
            } else if (currentGameState == GameState::MATCH_OVER) {
                // 停止所有音效
                AudioManager::stopAllSounds();
                // 停止背景音樂
                AudioManager::stopMusic();
                // 恢復背景音樂
                AudioManager::playMusic("bgm", -1);
                currentGameState = GameState::START_SCREEN;
                printf("Returned to start screen from match over\n");
            } else if (currentGameState == GameState::CHARACTER_INFO) {
                currentGameState = GameState::START_SCREEN;
                printf("Returned to start screen from character info\n");
            } else if (showRecords) {
                showRecords = false;
                printf("Closed game records\n");
            }
        }

        // 處理暫停選單的點擊
        if (currentGameState == GameState::PAUSED && event.type == SDL_MOUSEBUTTONDOWN) {
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);

            // 檢查點擊了哪個按鈕
            if (mouseX >= continueButton.x && mouseX <= continueButton.x + continueButton.w &&
                mouseY >= continueButton.y && mouseY <= continueButton.y + continueButton.h) {
                // 繼續遊戲
                currentGameState = GameState::PLAYING;
                printf("Game resumed\n");
            }
            else if (mouseX >= restartButton.x && mouseX <= restartButton.x + restartButton.w &&
                     mouseY >= restartButton.y && mouseY <= restartButton.y + restartButton.h) {
                // 重新開始遊戲，回到角色選擇畫面
                currentRound = 1;
                playerWins[0] = 0;
                playerWins[1] = 0;
                // 重置角色選擇狀態
                selectedCharacterIndex[0] = 0;
                selectedCharacterIndex[1] = 0;
                characterSelectionConfirmed[0] = false;
                characterSelectionConfirmed[1] = false;
                currentGameState = GameState::CHARACTER_SELECTION;
                printf("Game restarted, returning to character selection\n");
            }
            else if (mouseX >= exitButton.x && mouseX <= exitButton.x + exitButton.w &&
                     mouseY >= exitButton.y && mouseY <= exitButton.y + exitButton.h) {
                // 回到開始畫面
                currentGameState = GameState::START_SCREEN;
                menuCooldownTimer = MENU_COOLDOWN;  // 設置冷卻時間
                printf("Returned to start screen\n");
            }
        }

        // 處理開始畫面的按鈕點擊
        if (currentGameState == GameState::START_SCREEN && menuCooldownTimer <= 0.0f && !showRecords) {  // 添加冷卻檢查，且未顯示記錄時才處理
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                // --- 修正：先計算主選單按鈕座標，確保和 renderStartScreen 一致 ---
                int buttonWidth = 200;
                int buttonHeight = 50;
                int buttonGap = 20;
                int totalButtons = 4;
                int totalHeight = buttonHeight * totalButtons + buttonGap * (totalButtons - 1);
                int centerX = SCREEN_WIDTH / 2 - buttonWidth / 2;
                int startY = SCREEN_HEIGHT / 2 - totalHeight / 2;
                continueButton = {centerX, startY, buttonWidth, buttonHeight};
                chaosModeButton = {centerX, startY + (buttonHeight + buttonGap) * 1, buttonWidth, buttonHeight};
                characterInfoButton = {centerX, startY + (buttonHeight + buttonGap) * 2, buttonWidth, buttonHeight};
                recordButton = {centerX, startY + (buttonHeight + buttonGap) * 3, buttonWidth, buttonHeight};
                // --- End 修正 ---
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);

                // 檢查點擊了開始遊戲按鈕
                if (mouseX >= continueButton.x && mouseX <= continueButton.x + continueButton.w &&
                    mouseY >= continueButton.y && mouseY <= continueButton.y + continueButton.h) {
                    // 重置混亂模式狀態
                    isChaosMode = false;
                    chaosEvent = ChaosEventType::NONE;
                    chaosEventTimer = 0.0f;
                    // 重置角色選擇狀態
                    selectedCharacterIndex[0] = 0;
                    selectedCharacterIndex[1] = 0;
                    characterSelectionConfirmed[0] = false;
                    characterSelectionConfirmed[1] = false;
                    currentGameState = GameState::CHARACTER_SELECTION;
                    printf("Starting game, entering character selection\n");
                }
                // 檢查點擊了角色介紹按鈕
                else if (mouseX >= characterInfoButton.x && mouseX <= characterInfoButton.x + characterInfoButton.w &&
                         mouseY >= characterInfoButton.y && mouseY <= characterInfoButton.y + characterInfoButton.h) {
                    currentGameState = GameState::CHARACTER_INFO;
                    printf("Entering character info screen\n");
                }

                // 檢查是否點擊了記錄按鈕
                if (mouseX >= recordButton.x && mouseX <= recordButton.x + recordButton.w &&
                    mouseY >= recordButton.y && mouseY <= recordButton.y + recordButton.h) {
                    showRecords = !showRecords;
                    if (showRecords) {
                        loadGameRecords();
                    }
                }
                
                // 如果正在顯示記錄，檢查是否點擊了關閉按鈕
                if (showRecords) {
                    SDL_Rect closeRect = {
                        SCREEN_WIDTH/2 - 50,
                        SCREEN_HEIGHT/4 + SCREEN_HEIGHT/2 - 50,
                        100,
                        30
                    };
                    if (mouseX >= closeRect.x && mouseX <= closeRect.x + closeRect.w &&
                        mouseY >= closeRect.y && mouseY <= closeRect.y + closeRect.h) {
                        showRecords = false;
                    }
                }
                // 檢查點擊了混亂模式按鈕
                if (mouseX >= chaosModeButton.x && mouseX <= chaosModeButton.x + chaosModeButton.w &&
                    mouseY >= chaosModeButton.y && mouseY <= chaosModeButton.y + chaosModeButton.h) {
                    isChaosMode = true;
                    chaosEvent = ChaosEventType::NONE;
                    chaosEventTimer = 0.0f;
                    selectedCharacterIndex[0] = 0;
                    selectedCharacterIndex[1] = 0;
                    characterSelectionConfirmed[0] = false;
                    characterSelectionConfirmed[1] = false;
                    currentGameState = GameState::CHARACTER_SELECTION;
                    printf("Chaos mode start! Entering character selection\n");
                }
            }
        }

        // 處理角色選擇介面的輸入
        if (currentGameState == GameState::CHARACTER_SELECTION) {
            if (event.type == SDL_KEYDOWN) {
                // 玩家1控制 (W/S)
                if (!characterSelectionConfirmed[0]) {
                    if (event.key.keysym.sym == SDLK_w) {
                        selectedCharacterIndex[0] = (selectedCharacterIndex[0] - 1 + 2) % 2;
                        SDL_Delay(200);
                        printf("Player 1 selected character %d\n", selectedCharacterIndex[0]);
                    } else if (event.key.keysym.sym == SDLK_s) {
                        selectedCharacterIndex[0] = (selectedCharacterIndex[0] + 1) % 2;
                        SDL_Delay(200);
                        printf("Player 1 selected character %d\n", selectedCharacterIndex[0]);
                    } else if (event.key.keysym.sym == SDLK_RETURN && !event.key.repeat) {
                        characterSelectionConfirmed[0] = true;
                        printf("Player 1 confirmed character selection\n");
                    }
                }

                // 玩家2控制 (上下方向鍵)
                if (!characterSelectionConfirmed[1]) {
                    if (event.key.keysym.sym == SDLK_UP) {
                        selectedCharacterIndex[1] = (selectedCharacterIndex[1] - 1 + 2) % 2;
                        SDL_Delay(200);
                        printf("Player 2 selected character %d\n", selectedCharacterIndex[1]);
                    } else if (event.key.keysym.sym == SDLK_DOWN) {
                        selectedCharacterIndex[1] = (selectedCharacterIndex[1] + 1) % 2;
                        SDL_Delay(200);
                        printf("Player 2 selected character %d\n", selectedCharacterIndex[1]);
                    } else if (event.key.keysym.sym == SDLK_KP_ENTER && !event.key.repeat) {
                        characterSelectionConfirmed[1] = true;
                        printf("Player 2 confirmed character selection\n");
                    }
                }

                // 如果兩個玩家都確認了選擇，進入拳套選擇階段
                if (characterSelectionConfirmed[0] && characterSelectionConfirmed[1]) {
                    // 先重置所有選擇狀態
                    selectedGloveIndex[0] = 0;
                    selectedGloveIndex[1] = 0;
                    gloveSelectionConfirmed[0] = false;
                    gloveSelectionConfirmed[1] = false;
                    
                    // 清除所有按鍵事件
                    SDL_FlushEvents(SDL_KEYDOWN, SDL_KEYUP);
                    
                    // 添加延遲
                    SDL_Delay(200);
                    
                    // 進入拳套選擇階段
                    startGameAfterCharacterSelection();
                    return; // 立即返回，避免處理其他事件
                }
            }
        }

        // 處理拳套選擇的按鍵事件
        if (currentGameState == GameState::GLOVE_SELECTION) {
            if (event.type == SDL_KEYDOWN) {
                // 玩家1選擇 (W/S 鍵)
                if (!gloveSelectionConfirmed[0]) {
                    if (event.key.keysym.sym == SDLK_w) {
                        selectedGloveIndex[0] = (selectedGloveIndex[0] - 1 + 3) % 3;
                        SDL_Delay(200);
                        printf("Player 1 selected glove %d\n", selectedGloveIndex[0]);
                    }
                    else if (event.key.keysym.sym == SDLK_s) {
                        selectedGloveIndex[0] = (selectedGloveIndex[0] + 1) % 3;
                        SDL_Delay(200);
                        printf("Player 1 selected glove %d\n", selectedGloveIndex[0]);
                    }
                    else if (event.key.keysym.sym == SDLK_RETURN && !event.key.repeat) {
                        gloveSelectionConfirmed[0] = true;
                        printf("Player 1 confirmed glove selection\n");
                    }
                }
                // 玩家2選擇 (上下鍵)
                if (!gloveSelectionConfirmed[1]) {
                    if (event.key.keysym.sym == SDLK_UP) {
                        selectedGloveIndex[1] = (selectedGloveIndex[1] - 1 + 3) % 3;
                        SDL_Delay(200);
                        printf("Player 2 selected glove %d\n", selectedGloveIndex[1]);
                    }
                    else if (event.key.keysym.sym == SDLK_DOWN) {
                        selectedGloveIndex[1] = (selectedGloveIndex[1] + 1) % 3;
                        SDL_Delay(200);
                        printf("Player 2 selected glove %d\n", selectedGloveIndex[1]);
                    }
                    else if (event.key.keysym.sym == SDLK_KP_ENTER && !event.key.repeat) {
                        gloveSelectionConfirmed[1] = true;
                        printf("Player 2 confirmed glove selection\n");
                    }
                }
                // 只有兩人都確認才進入遊戲
                if (gloveSelectionConfirmed[0] && gloveSelectionConfirmed[1]) {
                    startGameAfterGloveSelection();
                    return; // 立即返回，避免處理其他事件
                }
            }
        }

        // --- 只有在 PLAYING 狀態下才處理玩家的按鍵放開事件 (如停止格擋、發射氣功) ---
        if (currentGameState == GameState::PLAYING) {
            if (event.type == SDL_KEYUP) {
                // Player 1 停止格擋
                if (players.size() >= 1 && event.key.keysym.sym == SDLK_k) {
                    if (players[0].state == Player::PlayerState::BLOCKING) {
                        players[0].handleAction("STOP_BLOCK");
                    }
                }
                // Player 2 停止格擋
                if (players.size() >= 2 && event.key.keysym.sym == SDLK_KP_2) {
                    if (players[1].state == Player::PlayerState::BLOCKING) {
                        players[1].handleAction("STOP_BLOCK");
                    }
                }
            }
            // 處理按下按鍵的瞬間觸發事件 (發射氣功/技能)
            if (event.type == SDL_KEYDOWN) {
                // Player 1 發射氣功
                if (players.size() >= 1 && event.key.keysym.sym == SDLK_u && !event.key.repeat) {
                    players[0].handleAction("FIRE_PROJECTILE");
                }
                // Player 1 特殊攻擊
                if (players.size() >= 1 && event.key.keysym.sym == SDLK_i && !event.key.repeat) {
                    if (players[0].canUseSpecialAttack()) {
                        if (players[0].characterId == "BlockMan") {
                            players[0].handleAction("SPECIAL_ATTACK");
                            if (players.size() >= 2) {
                                players[1].changeState(Player::PlayerState::LYING);
                                players[1].hurtTimer = 0.5f;
                            }
                        } else if (players[0].characterId == "Godon") {
                            players[0].handleAction("SPECIAL_ATTACK");
                        }
                    }
                }
                // Player 2 發射氣功
                if (players.size() >= 2 && event.key.keysym.sym == SDLK_KP_4 && !event.key.repeat) {
                    players[1].handleAction("FIRE_PROJECTILE");
                }
                // Player 2 特殊攻擊
                if (players.size() >= 2 && event.key.keysym.sym == SDLK_KP_5 && !event.key.repeat) {
                    if (players[1].canUseSpecialAttack()) {
                        if (players[1].characterId == "BlockMan") {
                            players[1].handleAction("SPECIAL_ATTACK");
                            if (players.size() >= 1) {
                                players[0].changeState(Player::PlayerState::LYING);
                                players[0].hurtTimer = 0.5f;
                            }
                        } else if (players[1].characterId == "Godon") {
                            players[1].handleAction("SPECIAL_ATTACK");
                        }
                    }
                }
            }
        }
    }

    // --- 只有在 PLAYING 狀態下才處理持續按壓的移動/攻擊等 ---
    if (currentGameState == GameState::PLAYING) {
        const Uint8* keystates = SDL_GetKeyboardState(NULL);
        // Player 1 控制 (活著且不在受傷/死亡狀態才能控制)
        if (players.size() >= 1 && players[0].isAlive()) {
            Player& p1 = players[0];
            bool isBlocking = keystates[SDL_SCANCODE_K];
            if (isBlocking) {
                p1.handleAction("BLOCK");
                p1.handleAction("STOP_X");
            } else {
                if (p1.state == Player::PlayerState::BLOCKING) {
                    p1.handleAction("STOP_BLOCK");
                }
                // --- 控制反轉 ---
                bool reverse = (isChaosMode && chaosEvent == ChaosEventType::CONTROL_REVERSE);
                bool p1Left = reverse ? keystates[SDL_SCANCODE_D] : keystates[SDL_SCANCODE_A];
                bool p1Right = reverse ? keystates[SDL_SCANCODE_A] : keystates[SDL_SCANCODE_D];
                bool p1Up = reverse ? keystates[SDL_SCANCODE_S] : keystates[SDL_SCANCODE_W];
                bool p1Down = reverse ? keystates[SDL_SCANCODE_W] : keystates[SDL_SCANCODE_S];
                bool p1Attack = reverse ? keystates[SDL_SCANCODE_U] : keystates[SDL_SCANCODE_J];
                bool p1Fire = reverse ? keystates[SDL_SCANCODE_J] : keystates[SDL_SCANCODE_U];
                if (p1Up) p1.handleAction("JUMP");
                if (p1Attack) p1.handleAction("ATTACK");
                if (p1Down) {
                    p1.handleAction("LYING");
                } else if (p1.state == Player::PlayerState::LYING) {
                    p1.handleAction("IDLE");
                }
                if (p1Left) p1.handleAction("LEFT");
                else if (p1Right) p1.handleAction("RIGHT");
                else p1.handleAction("STOP_X");
                // 反轉時，普攻/氣功鍵互換
                if (reverse && p1Fire && !p1Attack) p1.handleAction("FIRE_PROJECTILE");
                else if (!reverse && keystates[SDL_SCANCODE_U]) p1.handleAction("FIRE_PROJECTILE");
            }
        }
        // Player 2 控制 (活著且不在受傷/死亡狀態才能控制)
        if (players.size() >= 2 && players[1].isAlive()) {
            Player& p2 = players[1];
            bool isBlocking = keystates[SDL_SCANCODE_KP_2];
            if (isBlocking) {
                p2.handleAction("BLOCK");
                p2.handleAction("STOP_X");
            } else {
                if (p2.state == Player::PlayerState::BLOCKING) {
                    p2.handleAction("STOP_BLOCK");
                }
                bool reverse = (isChaosMode && chaosEvent == ChaosEventType::CONTROL_REVERSE);
                bool p2Left = reverse ? keystates[SDL_SCANCODE_RIGHT] : keystates[SDL_SCANCODE_LEFT];
                bool p2Right = reverse ? keystates[SDL_SCANCODE_LEFT] : keystates[SDL_SCANCODE_RIGHT];
                bool p2Up = reverse ? keystates[SDL_SCANCODE_DOWN] : keystates[SDL_SCANCODE_UP];
                bool p2Down = reverse ? keystates[SDL_SCANCODE_UP] : keystates[SDL_SCANCODE_DOWN];
                bool p2Attack = reverse ? keystates[SDL_SCANCODE_KP_4] : keystates[SDL_SCANCODE_KP_1];
                bool p2Fire = reverse ? keystates[SDL_SCANCODE_KP_1] : keystates[SDL_SCANCODE_KP_4];
                if (p2Up) p2.handleAction("JUMP");
                if (p2Attack) p2.handleAction("ATTACK");
                if (p2Down) {
                    p2.handleAction("LYING");
                } else if (p2.state == Player::PlayerState::LYING) {
                    p2.handleAction("IDLE");
                }
                if (p2Left) p2.handleAction("LEFT");
                else if (p2Right) p2.handleAction("RIGHT");
                else p2.handleAction("STOP_X");
                if (reverse && p2Fire && !p2Attack) p2.handleAction("FIRE_PROJECTILE");
                else if (!reverse && keystates[SDL_SCANCODE_KP_4]) p2.handleAction("FIRE_PROJECTILE");
            }
        }
    }
}

void Game::render() {
    // 清空畫面
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // 宣告所有需要的變數
    SDL_Texture* bgTex = nullptr;
    SDL_Texture* victoryTex = nullptr;
    SDL_Color textColor = {0, 0, 0, 255};
    SDL_Rect overlay = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    std::string winTexId; // <-- 提前宣告

    switch (currentGameState) {
        case GameState::START_SCREEN:
            renderStartScreen();
            break;
        case GameState::CHARACTER_SELECTION:
            renderCharacterSelection();
            break;
        case GameState::GLOVE_SELECTION:
            renderGloveSelection();
            break;
        case GameState::PLAYING:
        case GameState::PAUSED:  // 暫停狀態需要先渲染遊戲畫面
            // 繪製背景
            if (isChaosMode) {
                bgTex = TextureManager::getTexture(chaosBgIndex == 0 ? "background" : "background0");
            } else {
                bgTex = TextureManager::getTexture("background");
            }
            if (bgTex) {
                SDL_RenderCopy(renderer, bgTex, NULL, NULL);
            }

            // 繪製玩家
            for (Player& player : players) {
                player.render(renderer);
        }

        // 繪製氣功
            for (const Projectile& proj : projectiles) {
                if (proj.isActive) {
                    SDL_Rect destRect = {
                        static_cast<int>(proj.x),
                        static_cast<int>(proj.y),
                        PROJECTILE_HITBOX_W,
                        PROJECTILE_HITBOX_H
                    };
                    SDL_Texture* projTex = TextureManager::getTexture(proj.textureId);
                    if (projTex) {
                        SDL_RenderCopy(renderer, projTex, &proj.srcRect, &destRect);
                    }
                }
            }

            // 繪製回合資訊（血條、計時器等）
        renderRoundInfo();

            

            // 如果是暫停狀態，添加半透明遮罩和暫停選單
        if (currentGameState == GameState::PAUSED) {
                // 繪製半透明黑色遮罩
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);  // 半透明黑色
                SDL_RenderFillRect(renderer, &overlay);
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

                // 繪製暫停選單
                // 重新設定按鈕位置（與建構子一致）
                {
                    int buttonWidth = 200;
                    int buttonHeight = 50;
                    int buttonGap = 30;
                    int centerX = SCREEN_WIDTH / 2 - buttonWidth / 2;
                    int startY = SCREEN_HEIGHT / 2 - (buttonHeight * 3 + buttonGap * 2) / 2;
                    continueButton = {centerX, startY, buttonWidth, buttonHeight};
                    restartButton = {centerX, startY + buttonHeight + buttonGap, buttonWidth, buttonHeight};
                    exitButton = {centerX, startY + (buttonHeight + buttonGap) * 2, buttonWidth, buttonHeight};
                }
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderFillRect(renderer, &continueButton);
                SDL_RenderFillRect(renderer, &restartButton);
                SDL_RenderFillRect(renderer, &exitButton);
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderDrawRect(renderer, &continueButton);
                SDL_RenderDrawRect(renderer, &restartButton);
                SDL_RenderDrawRect(renderer, &exitButton);
                // 繪製按鈕文字
                if (buttonFont) {
                    SDL_Surface* continueText = TTF_RenderUTF8_Blended(buttonFont, "繼續遊戲", textColor);
                    SDL_Surface* restartText = TTF_RenderUTF8_Blended(buttonFont, "重新開始", textColor);
                    SDL_Surface* exitText = TTF_RenderUTF8_Blended(buttonFont, "回到主選單", textColor);
                    if (continueText && restartText && exitText) {
                        SDL_Texture* continueTex = SDL_CreateTextureFromSurface(renderer, continueText);
                        SDL_Texture* restartTex = SDL_CreateTextureFromSurface(renderer, restartText);
                        SDL_Texture* exitTex = SDL_CreateTextureFromSurface(renderer, exitText);
                        SDL_Rect continueRect = {
                            continueButton.x + (continueButton.w - continueText->w) / 2,
                            continueButton.y + (continueButton.h - continueText->h) / 2,
                            continueText->w,
                            continueText->h
                        };
                        SDL_Rect restartRect = {
                            restartButton.x + (restartButton.w - restartText->w) / 2,
                            restartButton.y + (restartButton.h - restartText->h) / 2,
                            restartText->w,
                            restartText->h
                        };
                        SDL_Rect exitRect = {
                            exitButton.x + (exitButton.w - exitText->w) / 2,
                            exitButton.y + (exitButton.h - exitText->h) / 2,
                            exitText->w,
                            exitText->h
                        };
                        SDL_RenderCopy(renderer, continueTex, NULL, &continueRect);
                        SDL_RenderCopy(renderer, restartTex, NULL, &restartRect);
                        SDL_RenderCopy(renderer, exitTex, NULL, &exitRect);
                        SDL_DestroyTexture(continueTex);
                        SDL_DestroyTexture(restartTex);
                        SDL_DestroyTexture(exitTex);
                    }
                    SDL_FreeSurface(continueText);
                    SDL_FreeSurface(restartText);
                    SDL_FreeSurface(exitText);
                }
            }
            // --- 混亂模式事件提示 ---
            if (isChaosMode && chaosEvent != ChaosEventType::NONE && chaosEventShowTimer > 0.0f && buttonFont) {
                const char* chaosMsg = nullptr;
                if (chaosEvent == ChaosEventType::CONTROL_REVERSE) chaosMsg = "超級控制大混亂! (鍵位全部顛倒)";
                else if (chaosEvent == ChaosEventType::HP_SWAP) chaosMsg = "血條交換! (血量百分比互換)";
                if (chaosMsg) {
                    SDL_Color c = {255, 0, 0, 255};
                    SDL_Surface* surf = TTF_RenderUTF8_Blended(buttonFont, chaosMsg, c);
                    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
                    // 顯示在混亂冷卻條下方
                    int chaosBarWidth = 80;
                    int chaosBarHeight = 10;
                    int chaosBarY = 10 + 20 + 8; // timerPosY + timerHeight + 8
                    int msgY = chaosBarY + chaosBarHeight + 36; // 再下移一點
                    SDL_Rect rect = {SCREEN_WIDTH/2 - surf->w/2, msgY, surf->w, surf->h};
                    SDL_RenderCopy(renderer, tex, nullptr, &rect);
                    SDL_DestroyTexture(tex);
                    SDL_FreeSurface(surf);
                }
            }
            break;
        case GameState::ROUND_OVER:
            // 繪製背景
            bgTex = TextureManager::getTexture("background");
            if (bgTex) {
                SDL_RenderCopy(renderer, bgTex, NULL, NULL);
            }

            // 繪製玩家
            for (Player& player : players) {
                player.render(renderer);
            }

            // 繪製回合結束資訊
            renderRoundInfo();
            break;
        case GameState::MATCH_OVER:
            winTexId = "victory_screen";
            if (roundWinnerIndex >= 0 && roundWinnerIndex < players.size()) {
                std::string cid = players[roundWinnerIndex].characterId;
                if (cid == "BlockMan") winTexId = "blockman_win_screen";
                else if (cid == "Godon") winTexId = "godon_win_screen";
            }
            victoryTex = TextureManager::getTexture(winTexId);
            if (victoryTex) {
                SDL_RenderCopy(renderer, victoryTex, NULL, NULL);
            }

            // 添加返回提示
            if (buttonFont) {
                SDL_Color hintColor = {255, 255, 255, 255}; // 白色
                SDL_Surface* hintText = TTF_RenderUTF8_Blended(buttonFont, "按 ESC 返回主選單", hintColor);
                if (hintText) {
                    SDL_Texture* hintTexture = SDL_CreateTextureFromSurface(renderer, hintText);
                    SDL_Rect hintRect = {
                        SCREEN_WIDTH / 2 - hintText->w / 2,
                        SCREEN_HEIGHT - 50,  // 距離底部 50 像素
                        hintText->w,
                        hintText->h
                    };
                    SDL_RenderCopy(renderer, hintTexture, NULL, &hintRect);
                    SDL_DestroyTexture(hintTexture);
                    SDL_FreeSurface(hintText);
                }
            }
            break;
        case GameState::ROUND_STARTING:
            // 回合開始狀態不需要特殊渲染
            break;
        case GameState::CHARACTER_INFO:
            renderCharacterInfo();
            break;
    }

    // 更新畫面
    SDL_RenderPresent(renderer);
}

void Game::spawnProjectile(float startX, float startY, int direction, int ownerIndex) {
    Projectile p;

    // --- 水平位置計算 ---
    // 從玩家身體的 X 位置開始計算偏移
    // 如果向右 (direction=1)，氣功在玩家寬度之後；如果向左 (direction=-1)，氣功在玩家 X 座標之前
    float spawnOffsetX = (direction > 0) ? (PLAYER_LOGIC_WIDTH * 0.5f) : (-PROJECTILE_HITBOX_W - PLAYER_LOGIC_WIDTH * 0.5f);
    p.x = startX + spawnOffsetX; // 使用傳入的 startX (這是玩家的 x 座標)

    // --- 垂直位置計算 (修正) ---
    // startY 已經是玩家的垂直中心 (player.y + PLAYER_LOGIC_HEIGHT / 2.0f)
    // 我們只需要將這個中心點減去氣功高度的一半，得到氣功的頂部 Y
    p.y = startY - (PROJECTILE_HITBOX_H / 2.0f);

    // --- 其他屬性設定 ---
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

    // 清理字體
    if (buttonFont) {
        TTF_CloseFont(buttonFont);
        buttonFont = nullptr;
    }
    TTF_Quit();

    // 退出 SDL 子系統
    IMG_Quit();
    SDL_Quit();
    printf("Game cleanup complete.\n");
}

void Game::resetPlayersForRound() {
    printf("Resetting players for round %d\n", currentRound);
    if (players.size() < 2) return;

    // 玩家 1 重置
    Player& p1 = players[0];
    p1.x = 100.0f; // 起始 X 位置
    p1.y = GROUND_LEVEL - PLAYER_LOGIC_HEIGHT; // 地面 Y 位置
    p1.vx = 0.0f;
    p1.vy = 0.0f;
    p1.health = PLAYER_DEFAULT_HEALTH; // 重置血量
    p1.direction = 1; // 面向右
    p1.state = Player::PlayerState::IDLE; // 初始狀態
    p1.currentAnimationType = AnimationType::IDLE; // 初始動畫
    p1.currentFrame = 0;
    p1.frameTimer = 0.0f;
    p1.invincibilityTimer = 0.0f; // 清除無敵
    p1.attackTimer = 0.0f;
    p1.attackCooldownTimer = 0.0f;
    p1.hurtTimer = 0.0f;
    p1.blockCooldownTimer = 0.0f;
    p1.attackRateCooldownTimer = 0.0f;
    p1.projectileCooldownTimer = 0.0f;
    p1.isOnGround = true; // 確保在地面上
    p1.shouldFireProjectile = false;

    // 玩家 2 重置
    Player& p2 = players[1];
    p2.x = SCREEN_WIDTH - 100.0f - PLAYER_LOGIC_WIDTH; // 起始 X 位置
    p2.y = GROUND_LEVEL - PLAYER_LOGIC_HEIGHT; // 地面 Y 位置
    p2.vx = 0.0f;
    p2.vy = 0.0f;
    p2.health = PLAYER_DEFAULT_HEALTH; // 重置血量
    p2.direction = -1; // 面向左
    p2.state = Player::PlayerState::IDLE; // 初始狀態
    p2.currentAnimationType = AnimationType::IDLE; // 初始動畫
    p2.currentFrame = 0;
    p2.frameTimer = 0.0f;
    p2.invincibilityTimer = 0.0f; // 清除無敵
    p2.attackTimer = 0.0f;
    p2.attackCooldownTimer = 0.0f;
    p2.hurtTimer = 0.0f;
    p2.blockCooldownTimer = 0.0f;
    p2.attackRateCooldownTimer = 0.0f;
    p2.projectileCooldownTimer = 0.0f;
    p2.isOnGround = true; // 確保在地面上
    p2.shouldFireProjectile = false;

    // 清除場上的氣功
    projectiles.clear();
}

void Game::startNewRound() {
    currentRound++; // 回合數增加
    printf("----- Starting Round %d -----\n", currentRound);
    roundTimer = ROUND_TIME_LIMIT; // 重置回合時間
    roundWinnerIndex = -1; // 清除上一回合勝利者
    resetPlayersForRound(); // 重置玩家狀態
    currentGameState = GameState::PLAYING; // 設定遊戲狀態為進行中
    // 可以在這裡播放 "Round X" 或 "Fight!" 的音效
    // AudioManager::playSound("round_start_sfx");
    // --- 混亂模式：第一回合事件倒數設為10秒 ---
    if (isChaosMode) {
        chaosEventTimer = 10.0f;
        chaosEventShowTimer = 0.0f;
        chaosEvent = ChaosEventType::NONE;
    }
}

void Game::endRound(int winnerPlayerIndex) {
    // 防止重複觸發回合結束
    if (currentGameState != GameState::PLAYING) {
        return;
    }

    printf("----- Round %d Over -----\n", currentRound);
    roundWinnerIndex = winnerPlayerIndex; // 記錄勝利者索引

    if (winnerPlayerIndex == 0) { // P1 獲勝
        playerWins[0]++;
        printf("Player 1 wins the round! Score: P1=%d, P2=%d\n", playerWins[0], playerWins[1]);
    } else if (winnerPlayerIndex == 1) { // P2 獲勝
        playerWins[1]++;
        printf("Player 2 wins the round! Score: P1=%d, P2=%d\n", playerWins[0], playerWins[1]);
    } else { // 平手
        printf("Round Draw! Score: P1=%d, P2=%d\n", playerWins[0], playerWins[1]);
        // 播放平手音效
        AudioManager::playSound("draw_sfx");
    }

    currentGameState = GameState::ROUND_OVER; // 切換到回合結束狀態
    roundOverTimer = 3.0f; // 設定為 3 秒的等待時間
}

void Game::checkForMatchWinner() {
    int winnerIndex = -1; // 先找出勝利者索引
    if (playerWins[0] >= ROUNDS_TO_WIN_MATCH) {
        winnerIndex = 0;
    } else if (playerWins[1] >= ROUNDS_TO_WIN_MATCH) {
        winnerIndex = 1;
    }

    // 如果有勝利者產生
    if (winnerIndex != -1) {
        printf("====== Player %d Wins the Match! ======\n", winnerIndex + 1);
        currentGameState = GameState::MATCH_OVER;
        roundWinnerIndex = winnerIndex; // 標記比賽勝利者

        // --- 新增：處理勝利者和失敗者狀態，以及音效 ---
        if (winnerIndex >= 0 && static_cast<size_t>(winnerIndex) < players.size()) { // 確保玩家存在
            players[winnerIndex].changeState(Player::PlayerState::VICTORY); // 設定勝利者狀態
            std::string cid = players[winnerIndex].characterId;
            // 強制轉小寫比對避免大小寫問題
            std::string cidLower = cid;
            std::transform(cidLower.begin(), cidLower.end(), cidLower.begin(), ::tolower);
            if (cidLower == "blockman")
                AudioManager::playRandomSound("blockman_victory",-1);
            else if (cidLower == "godon")
                AudioManager::playRandomSound("godon_victory",-1);

            // (可選) 設定失敗者狀態 (如果他不是 DEATH 的話)
            int loserIndex = 1 - winnerIndex; // 0 -> 1, 1 -> 0
            if (loserIndex >= 0 && static_cast<size_t>(loserIndex) < players.size() && players[loserIndex].isAlive()) {
                 printf("Match ended, loser (Player %d) was still alive.\n", loserIndex + 1);
            } else if (loserIndex >= 0 && static_cast<size_t>(loserIndex) < players.size()) {
                 printf("Match ended, loser (Player %d) was already defeated.\n", loserIndex + 1);
            }
        }

        AudioManager::stopMusic(); // 停止 BGM
        // 在整場比賽結束時保存記錄
        saveGameRecord();
    }
}

// --- 繪製回合相關資訊 (血條、勝利標記、計時器) ---
void Game::renderRoundInfo() {
    // --- 繪製血條 ---
    int healthBarWidth = 200;
    int healthBarHeight = 15;
    int healthBarY = 10;
    int winMarkY = healthBarY + healthBarHeight + 5; // 勝利標記 Y 座標
    int winMarkSize = 10;
    int winMarkSpacing = 5;

    // 玩家 1 血條和勝利標記
    if (players.size() >= 1) {
        Player& p1 = players[0];
        // 背景
        SDL_Rect p1HealthBg = {10, healthBarY, healthBarWidth, healthBarHeight};
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_RenderFillRect(renderer, &p1HealthBg);
        // 前景
        int p1FgWidth = (int)(healthBarWidth * std::max(0.0f, (float)p1.health) / PLAYER_DEFAULT_HEALTH);
        SDL_Rect p1HealthFg = {10, healthBarY, p1FgWidth, healthBarHeight};
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderFillRect(renderer, &p1HealthFg);
        // 勝利標記 (金色)
        SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
        for (int i = 0; i < playerWins[0]; ++i) {
            SDL_Rect p1Mark = {10 + i * (winMarkSize + winMarkSpacing), winMarkY, winMarkSize, winMarkSize};
            SDL_RenderFillRect(renderer, &p1Mark);
        }
    }

    // 玩家 2 血條和勝利標記
    if (players.size() >= 2) {
        Player& p2 = players[1];
        // 背景
        SDL_Rect p2HealthBg = {SCREEN_WIDTH - 10 - healthBarWidth, healthBarY, healthBarWidth, healthBarHeight};
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_RenderFillRect(renderer, &p2HealthBg);
        // 前景 (從右往左畫)
        int p2FgWidth = (int)(healthBarWidth * std::max(0.0f, (float)p2.health) / PLAYER_DEFAULT_HEALTH);
        SDL_Rect p2HealthFg = {SCREEN_WIDTH - 10 - p2FgWidth, healthBarY, p2FgWidth, healthBarHeight};
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderFillRect(renderer, &p2HealthFg);
        // 勝利標記 (金色)
        SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
        for (int i = 0; i < playerWins[1]; ++i) {
            // 從右邊開始排列
            SDL_Rect p2Mark = {SCREEN_WIDTH - 10 - (i + 1) * (winMarkSize + winMarkSpacing) + winMarkSpacing, winMarkY, winMarkSize, winMarkSize};
            SDL_RenderFillRect(renderer, &p2Mark);
        }
    }

    // --- 繪製回合計時器 ---
    int timerMaxWidth = 100;
    int timerHeight = 20;
    int timerPosX = SCREEN_WIDTH / 2 - timerMaxWidth / 2;
    int timerPosY = 10; // 與血條同高

    // 繪製計時器背景
    SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255); // 深灰
    SDL_Rect timerBg = {timerPosX, timerPosY, timerMaxWidth, timerHeight};
    SDL_RenderFillRect(renderer, &timerBg);

    // 繪製計時器前景 (模擬時間流逝)
    if (currentGameState == GameState::PLAYING || currentGameState == GameState::PAUSED) { // 在遊戲進行中和暫停時都顯示時間條
        float timeRatio = std::max(0.0f, roundTimer) / ROUND_TIME_LIMIT; // 時間比例 (0.0 ~ 1.0)
        SDL_Rect timerFg = {timerPosX, timerPosY, (int)(timerMaxWidth * timeRatio), timerHeight};
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // 黃色
        SDL_RenderFillRect(renderer, &timerFg);

        // 在暫停狀態下，顯示"暫停"文字
        if (currentGameState == GameState::PAUSED && buttonFont) {
            SDL_Color pauseColor = {255, 255, 255, 255}; // 白色
            SDL_Surface* pauseText = TTF_RenderUTF8_Blended(buttonFont, "暫停", pauseColor);
            if (pauseText) {
                SDL_Texture* pauseTex = SDL_CreateTextureFromSurface(renderer, pauseText);
                SDL_Rect pauseRect = {
                    timerPosX + (timerMaxWidth - pauseText->w) / 2,
                    timerPosY + (timerHeight - pauseText->h) / 2,
                    pauseText->w,
                    pauseText->h
                };
                SDL_RenderCopy(renderer, pauseTex, NULL, &pauseRect);
                SDL_DestroyTexture(pauseTex);
                SDL_FreeSurface(pauseText);
            }
        }
    }

    // 顯示冷卻條（Block、Punch、Projectile）
    if (players.size() >= 2) {
        // 冷卻條尺寸與間距
        int barWidth = 80;
        int barHeight = 12;
        int gapY = 8;
        int baseY = 50;

        // 載入小字體（只載入一次）
        static TTF_Font* smallFont = nullptr;
        if (!smallFont) {
            smallFont = TTF_OpenFont("assets/fonts/msjh.ttf", 16);
        }
        TTF_Font* labelFont = smallFont ? smallFont : buttonFont;

        // --- 玩家1（左上）---
        int leftX = 50;
        // Block 冷卻條
        {
            SDL_Rect blockBg = {leftX, baseY, barWidth, barHeight};
            SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
            SDL_RenderFillRect(renderer, &blockBg);
            float blockRatio = players[0].blockCooldownTimer / BLOCK_COOLDOWN;
            if (players[0].blockCooldownTimer <= 0) {
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                SDL_Rect fg = {leftX, baseY, barWidth, barHeight};
                SDL_RenderFillRect(renderer, &fg);
            } else {
                int fgWidth = (int)(barWidth * blockRatio);
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                SDL_Rect fg = {leftX, baseY, fgWidth, barHeight};
                SDL_RenderFillRect(renderer, &fg);
            }
            if (labelFont) {
                SDL_Color c = {255,255,255,255};
                SDL_Surface* txtSurf = TTF_RenderUTF8_Blended(labelFont, "Block", c);
                SDL_Texture* txtTex = SDL_CreateTextureFromSurface(renderer, txtSurf);
                SDL_Rect txtRect = {leftX + barWidth + 8, baseY + (barHeight-txtSurf->h)/2, txtSurf->w, txtSurf->h};
                SDL_RenderCopy(renderer, txtTex, NULL, &txtRect);
                SDL_FreeSurface(txtSurf);
                SDL_DestroyTexture(txtTex);
            }
        }
        // Punch 冷卻條
        {
            int y = baseY + barHeight + gapY;
            SDL_Rect atkBg = {leftX, y, barWidth, barHeight};
            SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
            SDL_RenderFillRect(renderer, &atkBg);
            float atkRatio = players[0].attackCooldownTimer / players[0].getAttackCooldown();
            if (players[0].attackCooldownTimer <= 0) {
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                SDL_Rect fg = {leftX, y, barWidth, barHeight};
                SDL_RenderFillRect(renderer, &fg);
            } else {
                int fgWidth = (int)(barWidth * atkRatio);
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                SDL_Rect fg = {leftX, y, fgWidth, barHeight};
                SDL_RenderFillRect(renderer, &fg);
            }
            if (labelFont) {
                SDL_Color c = {255,255,255,255};
                SDL_Surface* txtSurf = TTF_RenderUTF8_Blended(labelFont, "Punch", c);
                SDL_Texture* txtTex = SDL_CreateTextureFromSurface(renderer, txtSurf);
                SDL_Rect txtRect = {leftX + barWidth + 8, y + (barHeight-txtSurf->h)/2, txtSurf->w, txtSurf->h};
                SDL_RenderCopy(renderer, txtTex, NULL, &txtRect);
                SDL_FreeSurface(txtSurf);
                SDL_DestroyTexture(txtTex);
            }
        }
        // Projectile 冷卻條
        {
            int y = baseY + (barHeight + gapY) * 2;
            SDL_Rect qgBg = {leftX, y, barWidth, barHeight};
            SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
            SDL_RenderFillRect(renderer, &qgBg);
            float qgRatio = players[0].projectileCooldownTimer / PROJECTILE_COOLDOWN;
            if (players[0].projectileCooldownTimer <= 0) {
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                SDL_Rect fg = {leftX, y, barWidth, barHeight};
                SDL_RenderFillRect(renderer, &fg);
            } else {
                int fgWidth = (int)(barWidth * qgRatio);
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                SDL_Rect fg = {leftX, y, fgWidth, barHeight};
                SDL_RenderFillRect(renderer, &fg);
            }
            if (labelFont) {
                SDL_Color c = {255,255,255,255};
                SDL_Surface* txtSurf = TTF_RenderUTF8_Blended(labelFont, "Projectile", c);
                SDL_Texture* txtTex = SDL_CreateTextureFromSurface(renderer, txtSurf);
                SDL_Rect txtRect = {leftX + barWidth + 8, y + (barHeight-txtSurf->h)/2, txtSurf->w, txtSurf->h};
                SDL_RenderCopy(renderer, txtTex, NULL, &txtRect);
                SDL_FreeSurface(txtSurf);
                SDL_DestroyTexture(txtTex);
            }
        }

        // Special Attack 冷卻條
        {
            int y = baseY + (barHeight + gapY) * 3;
            SDL_Rect skillBg = {leftX, y, barWidth, barHeight};
            SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
            SDL_RenderFillRect(renderer, &skillBg);
            float skillRatio = players[0].specialAttackCooldownTimer / SPECIAL_ATTACK_COOLDOWN;
            if (players[0].specialAttackCooldownTimer <= 0) {
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                SDL_Rect fg = {leftX, y, barWidth, barHeight};
                SDL_RenderFillRect(renderer, &fg);
            } else {
                int fgWidth = (int)(barWidth * skillRatio);
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                SDL_Rect fg = {leftX, y, fgWidth, barHeight};
                SDL_RenderFillRect(renderer, &fg);
            }
            if (labelFont) {
                SDL_Color c = {255,255,255,255};
                SDL_Surface* txtSurf = TTF_RenderUTF8_Blended(labelFont, "Skill", c);
                SDL_Texture* txtTex = SDL_CreateTextureFromSurface(renderer, txtSurf);
                SDL_Rect txtRect = {leftX + barWidth + 8, y + (barHeight-txtSurf->h)/2, txtSurf->w, txtSurf->h};
                SDL_RenderCopy(renderer, txtTex, NULL, &txtRect);
                SDL_FreeSurface(txtSurf);
                SDL_DestroyTexture(txtTex);
            }
        }

        // --- 玩家2（右上）---
        int rightX = SCREEN_WIDTH - 50 - barWidth;
        // Block 冷卻條
        {
            SDL_Rect blockBg = {rightX, baseY, barWidth, barHeight};
            SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
            SDL_RenderFillRect(renderer, &blockBg);
            float blockRatio = players[1].blockCooldownTimer / BLOCK_COOLDOWN;
            if (players[1].blockCooldownTimer <= 0) {
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                SDL_Rect fg = {rightX, baseY, barWidth, barHeight};
                SDL_RenderFillRect(renderer, &fg);
            } else {
                int fgWidth = (int)(barWidth * blockRatio);
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                SDL_Rect fg = {rightX, baseY, fgWidth, barHeight};
                SDL_RenderFillRect(renderer, &fg);
            }
            if (labelFont) {
                SDL_Color c = {255,255,255,255};
                SDL_Surface* txtSurf = TTF_RenderUTF8_Blended(labelFont, "Block", c);
                SDL_Texture* txtTex = SDL_CreateTextureFromSurface(renderer, txtSurf);
                SDL_Rect txtRect = {rightX - txtSurf->w - 8, baseY + (barHeight-txtSurf->h)/2, txtSurf->w, txtSurf->h};
                SDL_RenderCopy(renderer, txtTex, NULL, &txtRect);
                SDL_FreeSurface(txtSurf);
                SDL_DestroyTexture(txtTex);
            }
        }
        // Punch 冷卻條
        {
            int y = baseY + barHeight + gapY;
            SDL_Rect atkBg = {rightX, y, barWidth, barHeight};
            SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
            SDL_RenderFillRect(renderer, &atkBg);
            float atkRatio = players[1].attackCooldownTimer / players[1].getAttackCooldown();
            if (players[1].attackCooldownTimer <= 0) {
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                SDL_Rect fg = {rightX, y, barWidth, barHeight};
                SDL_RenderFillRect(renderer, &fg);
            } else {
                int fgWidth = (int)(barWidth * atkRatio);
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                SDL_Rect fg = {rightX, y, fgWidth, barHeight};
                SDL_RenderFillRect(renderer, &fg);
            }
            if (labelFont) {
                SDL_Color c = {255,255,255,255};
                SDL_Surface* txtSurf = TTF_RenderUTF8_Blended(labelFont, "Punch", c);
                SDL_Texture* txtTex = SDL_CreateTextureFromSurface(renderer, txtSurf);
                SDL_Rect txtRect = {rightX - txtSurf->w - 8, y + (barHeight-txtSurf->h)/2, txtSurf->w, txtSurf->h};
                SDL_RenderCopy(renderer, txtTex, NULL, &txtRect);
                SDL_FreeSurface(txtSurf);
                SDL_DestroyTexture(txtTex);
            }
        }
        // Projectile 冷卻條
        {
            int y = baseY + (barHeight + gapY) * 2;
            SDL_Rect qgBg = {rightX, y, barWidth, barHeight};
            SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
            SDL_RenderFillRect(renderer, &qgBg);
            float qgRatio = players[1].projectileCooldownTimer / PROJECTILE_COOLDOWN;
            if (players[1].projectileCooldownTimer <= 0) {
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                SDL_Rect fg = {rightX, y, barWidth, barHeight};
                SDL_RenderFillRect(renderer, &fg);
            } else {
                int fgWidth = (int)(barWidth * qgRatio);
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                SDL_Rect fg = {rightX, y, fgWidth, barHeight};
                SDL_RenderFillRect(renderer, &fg);
            }
            if (labelFont) {
                SDL_Color c = {255,255,255,255};
                SDL_Surface* txtSurf = TTF_RenderUTF8_Blended(labelFont, "Projectile", c);
                SDL_Texture* txtTex = SDL_CreateTextureFromSurface(renderer, txtSurf);
                SDL_Rect txtRect = {rightX - txtSurf->w - 8, y + (barHeight-txtSurf->h)/2, txtSurf->w, txtSurf->h};
                SDL_RenderCopy(renderer, txtTex, NULL, &txtRect);
                SDL_FreeSurface(txtSurf);
                SDL_DestroyTexture(txtTex);
            }
        }

        // Special Attack 冷卻條
        {
            int y = baseY + (barHeight + gapY) * 3;
            SDL_Rect skillBg = {rightX, y, barWidth, barHeight};
            SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
            SDL_RenderFillRect(renderer, &skillBg);
            float skillRatio = players[1].specialAttackCooldownTimer / SPECIAL_ATTACK_COOLDOWN;
            if (players[1].specialAttackCooldownTimer <= 0) {
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                SDL_Rect fg = {rightX, y, barWidth, barHeight};
                SDL_RenderFillRect(renderer, &fg);
            } else {
                int fgWidth = (int)(barWidth * skillRatio);
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                SDL_Rect fg = {rightX, y, fgWidth, barHeight};
                SDL_RenderFillRect(renderer, &fg);
            }
            if (labelFont) {
                SDL_Color c = {255,255,255,255};
                SDL_Surface* txtSurf = TTF_RenderUTF8_Blended(labelFont, "Skill", c);
                SDL_Texture* txtTex = SDL_CreateTextureFromSurface(renderer, txtSurf);
                SDL_Rect txtRect = {rightX - txtSurf->w - 8, y + (barHeight-txtSurf->h)/2, txtSurf->w, txtSurf->h};
                SDL_RenderCopy(renderer, txtTex, NULL, &txtRect);
                SDL_FreeSurface(txtSurf);
                SDL_DestroyTexture(txtTex);
            }
        }
    }

    // --- 混亂模式冷卻條 ---
    if (isChaosMode && currentGameState == GameState::PLAYING) {
        int chaosBarWidth = 80;
        int chaosBarHeight = 10;
        int chaosBarX = SCREEN_WIDTH / 2 - chaosBarWidth / 2;
        int chaosBarY = timerPosY + timerHeight + 8; // 在回合計時條下方
        float ratio = std::min(1.0f, std::max(0.0f, chaosEventTimer / 15.0f));
        SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
        SDL_Rect bg = {chaosBarX, chaosBarY, chaosBarWidth, chaosBarHeight};
        SDL_RenderFillRect(renderer, &bg);
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_Rect fg = {chaosBarX, chaosBarY, (int)(chaosBarWidth * ratio), chaosBarHeight};
        SDL_RenderFillRect(renderer, &fg);
        // (移除標籤文字)
    }
}

void Game::update(float deltaTime) {
    // 更新選單冷卻計時器
    if (menuCooldownTimer > 0.0f) {
        menuCooldownTimer -= deltaTime;
    }

    // 根據遊戲狀態更新
    switch (currentGameState) {
        case GameState::CHARACTER_SELECTION:
            // 角色選擇狀態不需要特殊更新
            break;
        case GameState::GLOVE_SELECTION:
            // 拳套選擇狀態不需要特殊更新
            break;
        case GameState::PLAYING:
            // 檢查玩家是否死亡 (移到最前面，優先處理)
            for (size_t i = 0; i < players.size(); ++i) {
                if (players[i].health <= 0 && players[i].state == Player::PlayerState::DEATH) {
                    printf("Player %zu died, ending round...\n", i + 1);
                    endRound(1 - i); // 對手獲勝
                    return; // 立即返回，不執行其他更新
                }
            }

            // 更新回合計時器
            roundTimer -= deltaTime;
            if (roundTimer <= 0) {
                // 時間到，判斷血量決定勝負
                if (players.size() >= 2) {
                    if (players[0].health > players[1].health) {
                        endRound(0); // P1 勝
                    } else if (players[1].health > players[0].health) {
                        endRound(1); // P2 勝
                    } else {
                        endRound(-1); // 平手
                    }
                }
            }

            // 更新玩家
            for (Player& player : players) {
                player.update(deltaTime);
                
                // 檢查是否需要生成氣功
                if (player.shouldFireProjectile) {
                    float centerY = player.y + PLAYER_LOGIC_HEIGHT / 2.0f;
                    spawnProjectile(player.x, centerY, player.direction, &player - &players[0]);
                    player.shouldFireProjectile = false;
                }
            }

            // 更新氣功
            for (Projectile& proj : projectiles) {
                if (proj.isActive) {
                    proj.x += proj.vx * deltaTime;
                    
                    // 檢查是否超出畫面
                    if (proj.x < -PROJECTILE_HITBOX_W || proj.x > SCREEN_WIDTH) {
                        proj.isActive = false;
                    }
                }
            }

            // 檢查氣功碰撞
            for (Projectile& proj : projectiles) {
                if (!proj.isActive) continue;

                for (size_t i = 0; i < players.size(); ++i) {
                    if (i == static_cast<size_t>(proj.ownerPlayerIndex)) continue; // 跳過發射者
                    if (!players[i].isAlive()) continue; // 跳過已死亡的玩家

                    // 檢查碰撞
                    if (this->checkProjectilePlayerCollision(proj, players[i])) {
                        proj.isActive = false; // 氣功消失
                        break;
                    }
                }
            }

            // 檢查玩家之間的碰撞
            if (players.size() >= 2) {
                this->checkPlayerCollision(players[0], players[1]);
            }

            // --- 混亂模式事件觸發 ---
            if (isChaosMode) {
                chaosEventTimer -= deltaTime;
                if (chaosEventTimer <= 0.0f) {
                    int eventType = rand() % 2;
                    if (eventType == 0) {
                        chaosEvent = ChaosEventType::CONTROL_REVERSE;
                    } else {
                        chaosEvent = ChaosEventType::HP_SWAP;
                        // 立即執行血條交換
                        if (players.size() >= 2) {
                            float p1Percent = players[0].health / PLAYER_DEFAULT_HEALTH;
                            float p2Percent = players[1].health / PLAYER_DEFAULT_HEALTH;
                            int p1New = (int)(p2Percent * PLAYER_DEFAULT_HEALTH);
                            int p2New = (int)(p1Percent * PLAYER_DEFAULT_HEALTH);
                            players[0].health = p1New;
                            players[1].health = p2New;
                        }
                    }
                    chaosEventTimer = 15.0f;
                    chaosEventShowTimer = 3.0f;
                    chaosBgIndex = 1 - chaosBgIndex; // 交替背景
                }
                // 混亂事件名稱顯示倒數
                if (chaosEventShowTimer > 0.0f) {
                    chaosEventShowTimer -= deltaTime;
                    if (chaosEventShowTimer < 0.0f) chaosEventShowTimer = 0.0f;
                }
            }
            break;

        case GameState::ROUND_OVER:
            // 更新回合結束計時器
            roundOverTimer -= deltaTime;
            if (roundOverTimer <= 0) {
                checkForMatchWinner(); // 檢查是否有比賽勝利者
                if (currentGameState != GameState::MATCH_OVER) {
                    startNewRound(); // 開始新回合
                }
            }
            break;

        case GameState::MATCH_OVER:
            // 在比賽結束狀態下，只更新玩家的動畫
            for (Player& player : players) {
                player.update(deltaTime);
            }
            break;

        case GameState::PAUSED:
            // 暫停狀態下不更新遊戲邏輯
            break;

        case GameState::START_SCREEN:
        case GameState::ROUND_STARTING:
        case GameState::CHARACTER_INFO:
            // 這些狀態不需要更新遊戲邏輯
            break;
    }
}

bool Game::checkProjectilePlayerCollision(const Projectile& proj, Player& player) {
    // 躺下時不會被氣功打到
    if (player.state == Player::PlayerState::LYING) return false;
    if (!proj.isActive || !player.isAlive()) return false;

    // 檢查氣功和玩家的碰撞
    if (!proj.isActive || !player.isAlive()) return false;

    // 簡單的矩形碰撞檢測
    float projLeft = proj.x;
    float projRight = proj.x + PROJECTILE_HITBOX_W;
    float projTop = proj.y;
    float projBottom = proj.y + PROJECTILE_HITBOX_H;

    float playerLeft = player.x;
    float playerRight = player.x + PLAYER_LOGIC_WIDTH;
    float playerTop = player.y;
    float playerBottom = player.y + PLAYER_LOGIC_HEIGHT;

    // 檢查是否發生碰撞
    if (projRight > playerLeft && projLeft < playerRight &&
        projBottom > playerTop && projTop < playerBottom) {
        
        // 如果玩家正在格擋，則不受傷害
        if (player.state == Player::PlayerState::BLOCKING) {
            return true; // 氣功消失，但玩家不受傷害
        }

        // 造成傷害
        player.takeDamage(PROJECTILE_DAMAGE);
        return true;
    }

    return false;
}

void Game::checkPlayerCollision(Player& p1, Player& p2) {
    if (!p1.isAlive() || !p2.isAlive()) return;
    SDL_Rect p1Hitbox = p1.getHitboxWorld();
    SDL_Rect p2Hitbox = p2.getHitboxWorld();
    // 玩家1攻擊
    if (p1.state == Player::PlayerState::ATTACKING && p1Hitbox.w > 0) {
        SDL_Rect p2Box = p2.getBoundingBox();
        if (p1Hitbox.x < p2Box.x + p2Box.w &&
            p1Hitbox.x + p1Hitbox.w > p2Box.x &&
            p1Hitbox.y < p2Box.y + p2Box.h &&
            p1Hitbox.y + p1Hitbox.h > p2Box.y) {
            if (p2.state != Player::PlayerState::BLOCKING) {
                // Godon 衝刺技能
                if (p1.characterId == "Godon" && p1.isSpecialAttacking && !p1.hasHitDuringDash) {
                    p2.takeDamage(20);
                    p1.hasHitDuringDash = true;
                    p1.vx = 0; // 立即停止衝刺
                    p1.isSpecialAttacking = false;
                    p1.attackTimer = 0; // 立即結束攻擊狀態
                } else if (!p1.isSpecialAttacking) {
                    p2.takeDamage(p1.getAttackDamage());
                }
            }
        }
    }
    // 玩家2攻擊
    if (p2.state == Player::PlayerState::ATTACKING && p2Hitbox.w > 0) {
        SDL_Rect p1Box = p1.getBoundingBox();
        if (p2Hitbox.x < p1Box.x + p1Box.w &&
            p2Hitbox.x + p2Hitbox.w > p1Box.x &&
            p2Hitbox.y < p1Box.y + p1Box.h &&
            p2Hitbox.y + p2Hitbox.h > p1Box.y) {
            if (p1.state != Player::PlayerState::BLOCKING) {
                // Godon 衝刺技能
                if (p2.characterId == "Godon" && p2.isSpecialAttacking && !p2.hasHitDuringDash) {
                    p1.takeDamage(20);
                    p2.hasHitDuringDash = true;
                    p2.vx = 0; // 立即停止衝刺
                    p2.isSpecialAttacking = false;
                    p2.attackTimer = 0; // 立即結束攻擊狀態
                } else if (!p2.isSpecialAttacking) {
                    p1.takeDamage(p2.getAttackDamage());
                }
            }
        }
    }
    // 防止玩家重疊
    float p1Right = p1.x + PLAYER_LOGIC_WIDTH;
    float p2Left = p2.x;
    float p1Left = p1.x;
    float p2Right = p2.x + PLAYER_LOGIC_WIDTH;
    if (p1Right > p2Left && p1Left < p2Right) {
        float overlap = p1Right - p2Left;
        if (overlap > 0) {
            float pushDistance = overlap / 2.0f;
            p1.x -= pushDistance;
            p2.x += pushDistance;
        }
    }
}

void Game::handleGloveSelection() {
    // 已廢棄，保留空函數避免連結錯誤
}

void Game::renderGloveSelection() {
    // 背景圖
    SDL_Texture* bgTex = TextureManager::getTexture("start_screen");
    if (bgTex) {
        SDL_RenderCopy(renderer, bgTex, NULL, NULL);
    } else {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
    }

    // 拳套資訊
    const char* gloveNames[3] = {"10oz 蹦闆拿的", "14oz 等國動拿", "18oz 統神拿的"};
    SDL_Color gloveColors[3] = {
        {200, 200, 255, 255}, // 10oz
        {200, 255, 200, 255}, // 14oz
        {255, 220, 180, 255}  // 18oz
    };
    int gloveBoxW = 220, gloveBoxH = 70;
    int gapY = 30;
    int leftX = 80, rightX = SCREEN_WIDTH - 80 - gloveBoxW;
    int startY = 120;

    // 畫左側（玩家一）三個拳套框
    for (int i = 0; i < 3; ++i) {
        SDL_Rect box = {leftX, startY + i * (gloveBoxH + gapY), gloveBoxW, gloveBoxH};
        SDL_SetRenderDrawColor(renderer, gloveColors[i].r, gloveColors[i].g, gloveColors[i].b, 255);
        SDL_RenderFillRect(renderer, &box);
        SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
        SDL_RenderDrawRect(renderer, &box);
        // 拳套名稱
        if (buttonFont) {
            SDL_Surface* txtSurf = TTF_RenderUTF8_Blended(buttonFont, gloveNames[i], textColor);
            SDL_Texture* txtTex = SDL_CreateTextureFromSurface(renderer, txtSurf);
            SDL_Rect txtRect = {box.x + (box.w - txtSurf->w)/2, box.y + (box.h - txtSurf->h)/2, txtSurf->w, txtSurf->h};
            SDL_RenderCopy(renderer, txtTex, NULL, &txtRect);
            SDL_FreeSurface(txtSurf);
            SDL_DestroyTexture(txtTex);
        }
    }
    // 畫右側（玩家二）三個拳套框
    for (int i = 0; i < 3; ++i) {
        SDL_Rect box = {rightX, startY + i * (gloveBoxH + gapY), gloveBoxW, gloveBoxH};
        SDL_SetRenderDrawColor(renderer, gloveColors[i].r, gloveColors[i].g, gloveColors[i].b, 255);
        SDL_RenderFillRect(renderer, &box);
        SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
        SDL_RenderDrawRect(renderer, &box);
        // 拳套名稱
        if (buttonFont) {
            SDL_Surface* txtSurf = TTF_RenderUTF8_Blended(buttonFont, gloveNames[i], textColor);
            SDL_Texture* txtTex = SDL_CreateTextureFromSurface(renderer, txtSurf);
            SDL_Rect txtRect = {box.x + (box.w - txtSurf->w)/2, box.y + (box.h - txtSurf->h)/2, txtSurf->w, txtSurf->h};
            SDL_RenderCopy(renderer, txtTex, NULL, &txtRect);
            SDL_FreeSurface(txtSurf);
            SDL_DestroyTexture(txtTex);  
        }
    }

    // 玩家一選擇框
    int idx1 = selectedGloveIndex[0];
    SDL_Rect selBox1 = {leftX-8, startY + idx1 * (gloveBoxH + gapY)-8, gloveBoxW+16, gloveBoxH+16};
    if (gloveSelectionConfirmed[0]) {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // 綠
    } else {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // 紅
    }
    SDL_RenderDrawRect(renderer, &selBox1);
    SDL_RenderDrawRect(renderer, &selBox1);
    // 玩家一已確認
    if (gloveSelectionConfirmed[0]) {
        const char* confirmTxt = "P1已確認";
        SDL_Color c = {255,0,0,255}; // 紅色
        SDL_Surface* txtSurf = TTF_RenderUTF8_Blended(buttonFont, confirmTxt, c);
        SDL_Texture* txtTex = SDL_CreateTextureFromSurface(renderer, txtSurf);
        SDL_Rect txtRect = {leftX + gloveBoxW + 20, startY + idx1 * (gloveBoxH + gapY) + 10, txtSurf->w, txtSurf->h};
        SDL_RenderCopy(renderer, txtTex, NULL, &txtRect);
        SDL_FreeSurface(txtSurf);
        SDL_DestroyTexture(txtTex);
    }

    // 玩家二選擇框
    int idx2 = selectedGloveIndex[1];
    SDL_Rect selBox2 = {rightX-8, startY + idx2 * (gloveBoxH + gapY)-8, gloveBoxW+16, gloveBoxH+16};
    if (gloveSelectionConfirmed[1]) {
        SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255); // 藍綠
    } else {
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // 藍
    }
    SDL_RenderDrawRect(renderer, &selBox2);
    SDL_RenderDrawRect(renderer, &selBox2);
    // 玩家二已確認
    if (gloveSelectionConfirmed[1]) {
        const char* confirmTxt = "P2已確認";
        SDL_Color c = {255,0,0,255}; // 紅色
        SDL_Surface* txtSurf = TTF_RenderUTF8_Blended(buttonFont, confirmTxt, c);
        SDL_Texture* txtTex = SDL_CreateTextureFromSurface(renderer, txtSurf);
        SDL_Rect txtRect = {rightX - txtSurf->w - 20, startY + idx2 * (gloveBoxH + gapY) + 10, txtSurf->w, txtSurf->h};
        SDL_RenderCopy(renderer, txtTex, NULL, &txtRect);
        SDL_FreeSurface(txtSurf);
        SDL_DestroyTexture(txtTex);
    }

    // 標題
    if (buttonFont) {
        SDL_Color c = {255,255,255,255};
        SDL_Surface* titleSurf = TTF_RenderUTF8_Blended(buttonFont, "選擇你的拳套", c);
        SDL_Texture* titleTex = SDL_CreateTextureFromSurface(renderer, titleSurf);
        SDL_Rect titleRect = {SCREEN_WIDTH/2 - titleSurf->w/2, 40, titleSurf->w, titleSurf->h};
        SDL_RenderCopy(renderer, titleTex, NULL, &titleRect);
        SDL_FreeSurface(titleSurf);
        SDL_DestroyTexture(titleTex);
    }

    // 操作說明分成兩邊，白色字
    const char* helpText1 = "P1: W/S選擇, Enter確認";
    const char* helpText2 = "P2: ↑/↓選擇, 小鍵盤Enter確認";
    if (buttonFont) {
        SDL_Color c = {255,255,255,255}; // 白色
        SDL_Surface* helpSurf1 = TTF_RenderUTF8_Blended(buttonFont, helpText1, c);
        SDL_Texture* helpTex1 = SDL_CreateTextureFromSurface(renderer, helpSurf1);
        SDL_Rect helpRect1 = {leftX, SCREEN_HEIGHT - helpSurf1->h - 100, helpSurf1->w, helpSurf1->h}; // 從 -20 改為 -100
        SDL_RenderCopy(renderer, helpTex1, NULL, &helpRect1);
        SDL_FreeSurface(helpSurf1);
        SDL_DestroyTexture(helpTex1);

        SDL_Surface* helpSurf2 = TTF_RenderUTF8_Blended(buttonFont, helpText2, c);
        SDL_Texture* helpTex2 = SDL_CreateTextureFromSurface(renderer, helpSurf2);
        SDL_Rect helpRect2 = {rightX + gloveBoxW - helpSurf2->w, SCREEN_HEIGHT - helpSurf2->h - 100, helpSurf2->w, helpSurf2->h}; // 從 -20 改為 -100
        SDL_RenderCopy(renderer, helpTex2, NULL, &helpRect2);
        SDL_FreeSurface(helpSurf2);
        SDL_DestroyTexture(helpTex2);
    }

    SDL_RenderPresent(renderer);
}

void Game::startGameAfterGloveSelection() {
    // 重置所有遊戲數據
    currentRound = 1;
    playerWins[0] = 0;
    playerWins[1] = 0;
    roundTimer = ROUND_DURATION;
    roundOverTimer = 0.0f;
    roundWinnerIndex = -1;
    isPaused = false;

    // 設置玩家的拳套
    players[0].setGlove(static_cast<Player::GloveType>(selectedGloveIndex[0]));
    players[1].setGlove(static_cast<Player::GloveType>(selectedGloveIndex[1]));

    // 重置選擇狀態
    gloveSelectionConfirmed[0] = false;
    gloveSelectionConfirmed[1] = false;

    // 開始遊戲
    currentGameState = GameState::PLAYING;
    startNewRound();
}

void Game::handleCharacterSelection() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_KEYDOWN) {
            // 玩家1控制 (W/S)
            if (!characterSelectionConfirmed[0]) {
                if (event.key.keysym.sym == SDLK_w) {
                    selectedCharacterIndex[0] = (selectedCharacterIndex[0] - 1 + 2) % 2;
                    SDL_Delay(200);
                    printf("Player 1 selected character %d\n", selectedCharacterIndex[0]);
                } else if (event.key.keysym.sym == SDLK_s) {
                    selectedCharacterIndex[0] = (selectedCharacterIndex[0] + 1) % 2;
                    SDL_Delay(200);
                    printf("Player 1 selected character %d\n", selectedCharacterIndex[0]);
                } else if (event.key.keysym.sym == SDLK_RETURN && !event.key.repeat) {
                    characterSelectionConfirmed[0] = true;
                    printf("Player 1 confirmed character selection\n");
                }
            }

            // 玩家2控制 (上下方向鍵)
            if (!characterSelectionConfirmed[1]) {
                if (event.key.keysym.sym == SDLK_UP) {
                    selectedCharacterIndex[1] = (selectedCharacterIndex[1] - 1 + 2) % 2;
                    SDL_Delay(200);
                    printf("Player 2 selected character %d\n", selectedCharacterIndex[1]);
                } else if (event.key.keysym.sym == SDLK_DOWN) {
                    selectedCharacterIndex[1] = (selectedCharacterIndex[1] + 1) % 2;
                    SDL_Delay(200);
                    printf("Player 2 selected character %d\n", selectedCharacterIndex[1]);
                } else if (event.key.keysym.sym == SDLK_KP_ENTER && !event.key.repeat) {
                    characterSelectionConfirmed[1] = true;
                    printf("Player 2 confirmed character selection\n");
                }
            }

            // 如果兩個玩家都確認了選擇，進入拳套選擇階段
            if (characterSelectionConfirmed[0] && characterSelectionConfirmed[1]) {
                // 先重置所有選擇狀態
                selectedGloveIndex[0] = 0;
                selectedGloveIndex[1] = 0;
                gloveSelectionConfirmed[0] = false;
                gloveSelectionConfirmed[1] = false;
                
                // 清除所有按鍵事件
                SDL_FlushEvents(SDL_KEYDOWN, SDL_KEYUP);
                
                // 添加延遲
                SDL_Delay(200);
                
                // 進入拳套選擇階段
                startGameAfterCharacterSelection();
                return; // 立即返回，避免處理其他事件
            }
        }
    }
}

void Game::renderCharacterSelection() {
    // 背景圖
    SDL_Texture* bgTex = TextureManager::getTexture("start_screen");
    if (bgTex) {
        SDL_RenderCopy(renderer, bgTex, NULL, NULL);
    } else {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
    }

    // 角色資訊
    const char* characterNames[2] = {"統神", "國動"};
    SDL_Color characterColors[2] = {
        {255, 200, 200, 255}, // 統神
        {200, 200, 255, 255}  // 國動
    };
    int characterBoxW = 220, characterBoxH = 70;
    int gapY = 30;
    int leftX = 80, rightX = SCREEN_WIDTH - 80 - characterBoxW;
    int startY = 120;

    // 畫左側（玩家一）兩個角色框
    for (int i = 0; i < 2; ++i) {
        SDL_Rect box = {leftX, startY + i * (characterBoxH + gapY), characterBoxW, characterBoxH};
        SDL_SetRenderDrawColor(renderer, characterColors[i].r, characterColors[i].g, characterColors[i].b, 255);
        SDL_RenderFillRect(renderer, &box);
        SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
        SDL_RenderDrawRect(renderer, &box);
        // 角色名稱
        if (buttonFont) {
            SDL_Surface* txtSurf = TTF_RenderUTF8_Blended(buttonFont, characterNames[i], textColor);
            SDL_Texture* txtTex = SDL_CreateTextureFromSurface(renderer, txtSurf);
            SDL_Rect txtRect = {box.x + (box.w - txtSurf->w)/2, box.y + (box.h - txtSurf->h)/2, txtSurf->w, txtSurf->h};
            SDL_RenderCopy(renderer, txtTex, NULL, &txtRect);
            SDL_FreeSurface(txtSurf);
            SDL_DestroyTexture(txtTex);
        }
    }

    // 畫右側（玩家二）兩個角色框
    for (int i = 0; i < 2; ++i) {
        SDL_Rect box = {rightX, startY + i * (characterBoxH + gapY), characterBoxW, characterBoxH};
        SDL_SetRenderDrawColor(renderer, characterColors[i].r, characterColors[i].g, characterColors[i].b, 255);
        SDL_RenderFillRect(renderer, &box);
        SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
        SDL_RenderDrawRect(renderer, &box);
        // 角色名稱
        if (buttonFont) {
            SDL_Surface* txtSurf = TTF_RenderUTF8_Blended(buttonFont, characterNames[i], textColor);
            SDL_Texture* txtTex = SDL_CreateTextureFromSurface(renderer, txtSurf);
            SDL_Rect txtRect = {box.x + (box.w - txtSurf->w)/2, box.y + (box.h - txtSurf->h)/2, txtSurf->w, txtSurf->h};
            SDL_RenderCopy(renderer, txtTex, NULL, &txtRect);
            SDL_FreeSurface(txtSurf);
            SDL_DestroyTexture(txtTex);
        }
    }

    // 玩家一選擇框
    int idx1 = selectedCharacterIndex[0];
    SDL_Rect selBox1 = {leftX-8, startY + idx1 * (characterBoxH + gapY)-8, characterBoxW+16, characterBoxH+16};
    if (characterSelectionConfirmed[0]) {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // 綠
    } else {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // 紅
    }
    SDL_RenderDrawRect(renderer, &selBox1);
    SDL_RenderDrawRect(renderer, &selBox1);
    // 玩家一已確認
    if (characterSelectionConfirmed[0]) {
        const char* confirmTxt = "P1已確認";
        SDL_Color c = {255,0,0,255}; // 紅色
        SDL_Surface* txtSurf = TTF_RenderUTF8_Blended(buttonFont, confirmTxt, c);
        SDL_Texture* txtTex = SDL_CreateTextureFromSurface(renderer, txtSurf);
        SDL_Rect txtRect = {leftX + characterBoxW + 20, startY + idx1 * (characterBoxH + gapY) + 10, txtSurf->w, txtSurf->h};
        SDL_RenderCopy(renderer, txtTex, NULL, &txtRect);
        SDL_FreeSurface(txtSurf);
        SDL_DestroyTexture(txtTex);
    }

    // 玩家二選擇框
    int idx2 = selectedCharacterIndex[1];
    SDL_Rect selBox2 = {rightX-8, startY + idx2 * (characterBoxH + gapY)-8, characterBoxW+16, characterBoxH+16};
    if (characterSelectionConfirmed[1]) {
        SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255); // 藍綠
    } else {
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // 藍
    }
    SDL_RenderDrawRect(renderer, &selBox2);
    SDL_RenderDrawRect(renderer, &selBox2);
    // 玩家二已確認
    if (characterSelectionConfirmed[1]) {
        const char* confirmTxt = "P2已確認";
        SDL_Color c = {255,0,0,255}; // 紅色
        SDL_Surface* txtSurf = TTF_RenderUTF8_Blended(buttonFont, confirmTxt, c);
        SDL_Texture* txtTex = SDL_CreateTextureFromSurface(renderer, txtSurf);
        SDL_Rect txtRect = {rightX - txtSurf->w - 20, startY + idx2 * (characterBoxH + gapY) + 10, txtSurf->w, txtSurf->h};
        SDL_RenderCopy(renderer, txtTex, NULL, &txtRect);
        SDL_FreeSurface(txtSurf);
        SDL_DestroyTexture(txtTex);
    }

    // 標題
    if (buttonFont) {
        SDL_Color c = {255,255,255,255};
        SDL_Surface* titleSurf = TTF_RenderUTF8_Blended(buttonFont, "選擇你的角色", c);
        SDL_Texture* titleTex = SDL_CreateTextureFromSurface(renderer, titleSurf);
        SDL_Rect titleRect = {SCREEN_WIDTH/2 - titleSurf->w/2, 40, titleSurf->w, titleSurf->h};
        SDL_RenderCopy(renderer, titleTex, NULL, &titleRect);
        SDL_FreeSurface(titleSurf);
        SDL_DestroyTexture(titleTex);
    }

    // 操作說明分成兩邊，白色字
    const char* helpText1 = "P1: W/S選擇, Enter確認";
    const char* helpText2 = "P2: ↑/↓選擇, 小鍵盤Enter確認";
    if (buttonFont) {
        SDL_Color c = {255,255,255,255}; // 白色
        SDL_Surface* helpSurf1 = TTF_RenderUTF8_Blended(buttonFont, helpText1, c);
        SDL_Texture* helpTex1 = SDL_CreateTextureFromSurface(renderer, helpSurf1);
        SDL_Rect helpRect1 = {leftX, SCREEN_HEIGHT - helpSurf1->h - 100, helpSurf1->w, helpSurf1->h}; // 從 -20 改為 -100
        SDL_RenderCopy(renderer, helpTex1, NULL, &helpRect1);
        SDL_FreeSurface(helpSurf1);
        SDL_DestroyTexture(helpTex1);

        SDL_Surface* helpSurf2 = TTF_RenderUTF8_Blended(buttonFont, helpText2, c);
        SDL_Texture* helpTex2 = SDL_CreateTextureFromSurface(renderer, helpSurf2);
        SDL_Rect helpRect2 = {rightX + characterBoxW - helpSurf2->w, SCREEN_HEIGHT - helpSurf2->h - 100, helpSurf2->w, helpSurf2->h}; // 從 -20 改為 -100
        SDL_RenderCopy(renderer, helpTex2, NULL, &helpRect2);
        SDL_FreeSurface(helpSurf2);
        SDL_DestroyTexture(helpTex2);
    }
}

void Game::startGameAfterCharacterSelection() {
    // 清除所有按鍵狀態
    SDL_FlushEvents(SDL_KEYDOWN, SDL_KEYUP);
    
    // 添加延遲
    SDL_Delay(200);

    // 根據選擇的角色創建玩家
    players.clear();
    
    // 玩家1
    if (selectedCharacterIndex[0] == 0) {
        players.emplace_back(100.0f, GROUND_LEVEL - PLAYER_LOGIC_HEIGHT, 1, "BlockMan", "blockman_sprites");
    } else {
        players.emplace_back(100.0f, GROUND_LEVEL - PLAYER_LOGIC_HEIGHT, 1, "Godon", "godon_sprites");
    }

    // 玩家2
    if (selectedCharacterIndex[1] == 0) {
        players.emplace_back(SCREEN_WIDTH - 100.0f - PLAYER_LOGIC_WIDTH, GROUND_LEVEL - PLAYER_LOGIC_HEIGHT, -1, "BlockMan", "blockman_sprites");
    } else {
        players.emplace_back(SCREEN_WIDTH - 100.0f - PLAYER_LOGIC_WIDTH, GROUND_LEVEL - PLAYER_LOGIC_HEIGHT, -1, "Godon", "godon_sprites");
    }

    // 重置拳套選擇狀態
    selectedGloveIndex[0] = 0;
    selectedGloveIndex[1] = 0;
    gloveSelectionConfirmed[0] = false;
    gloveSelectionConfirmed[1] = false;

    // 進入拳套選擇階段
    currentGameState = GameState::GLOVE_SELECTION;
    
    // 再次清除所有按鍵狀態
    SDL_FlushEvents(SDL_KEYDOWN, SDL_KEYUP);
    
    printf("Entering glove selection phase\n");
}

void Game::renderStartScreen() {
    // 繪製背景
    SDL_RenderCopy(renderer, TextureManager::getTexture("start_screen"), nullptr, nullptr);

    // 如果正在顯示記錄，則只渲染記錄，不繪製主選單按鈕
    if (showRecords) {
        renderGameRecords();
        return;
    }

    // 計算按鈕位置
    int buttonWidth = 200;
    int buttonHeight = 50;
    int buttonGap = 20; // 更小的間距
    int totalButtons = 4;
    int totalHeight = buttonHeight * totalButtons + buttonGap * (totalButtons - 1);
    int centerX = SCREEN_WIDTH / 2 - buttonWidth / 2;
    int startY = SCREEN_HEIGHT / 2 - totalHeight / 2;

    // 依新順序排列：開始遊戲、混亂模式、角色介紹、查看遊戲紀錄
    continueButton = {centerX, startY, buttonWidth, buttonHeight};
    chaosModeButton = {centerX, startY + (buttonHeight + buttonGap) * 1, buttonWidth, buttonHeight};
    characterInfoButton = {centerX, startY + (buttonHeight + buttonGap) * 2, buttonWidth, buttonHeight};
    recordButton = {centerX, startY + (buttonHeight + buttonGap) * 3, buttonWidth, buttonHeight};

    // 繪製按鈕底色與邊框
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &continueButton);
    SDL_RenderFillRect(renderer, &chaosModeButton);
    SDL_RenderFillRect(renderer, &characterInfoButton);
    SDL_RenderFillRect(renderer, &recordButton);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &continueButton);
    SDL_RenderDrawRect(renderer, &chaosModeButton);
    SDL_RenderDrawRect(renderer, &characterInfoButton);
    SDL_RenderDrawRect(renderer, &recordButton);

    // 繪製按鈕文字
    SDL_Surface* startText = TTF_RenderUTF8_Blended(buttonFont, "開始遊戲", textColor);
    SDL_Surface* chaosText = TTF_RenderUTF8_Blended(buttonFont, "混亂模式", textColor);
    SDL_Surface* infoText = TTF_RenderUTF8_Blended(buttonFont, "角色介紹", textColor);
    SDL_Surface* recordText = TTF_RenderUTF8_Blended(buttonFont, "查看遊戲記錄", textColor);
    
    if (startText && chaosText && infoText && recordText) {
        SDL_Texture* startTexture = SDL_CreateTextureFromSurface(renderer, startText);
        SDL_Texture* chaosTexture = SDL_CreateTextureFromSurface(renderer, chaosText);
        SDL_Texture* infoTexture = SDL_CreateTextureFromSurface(renderer, infoText);
        SDL_Texture* recordTexture = SDL_CreateTextureFromSurface(renderer, recordText);
        
        if (startTexture && chaosTexture && infoTexture && recordTexture) {
            SDL_Rect startTextRect = {
                continueButton.x + (continueButton.w - startText->w) / 2,
                continueButton.y + (continueButton.h - startText->h) / 2,
                startText->w,
                startText->h
            };
            SDL_Rect chaosTextRect = {
                chaosModeButton.x + (chaosModeButton.w - chaosText->w) / 2,
                chaosModeButton.y + (chaosModeButton.h - chaosText->h) / 2,
                chaosText->w,
                chaosText->h
            };
            SDL_Rect infoTextRect = {
                characterInfoButton.x + (characterInfoButton.w - infoText->w) / 2,
                characterInfoButton.y + (characterInfoButton.h - infoText->h) / 2,
                infoText->w,
                infoText->h
            };
            SDL_Rect recordTextRect = {
                recordButton.x + (recordButton.w - recordText->w) / 2,
                recordButton.y + (recordButton.h - recordText->h) / 2,
                recordText->w,
                recordText->h
            };
            
            SDL_RenderCopy(renderer, startTexture, nullptr, &startTextRect);
            SDL_RenderCopy(renderer, chaosTexture, nullptr, &chaosTextRect);
            SDL_RenderCopy(renderer, infoTexture, nullptr, &infoTextRect);
            SDL_RenderCopy(renderer, recordTexture, nullptr, &recordTextRect);
            
            SDL_DestroyTexture(startTexture);
            SDL_DestroyTexture(chaosTexture);
            SDL_DestroyTexture(infoTexture);
            SDL_DestroyTexture(recordTexture);
        }
        
        SDL_FreeSurface(startText);
        SDL_FreeSurface(chaosText);
        SDL_FreeSurface(infoText);
        SDL_FreeSurface(recordText);
    }
}

void Game::renderCharacterInfo() {
    // 繪製背景
    SDL_RenderCopy(renderer, TextureManager::getTexture("start_screen"), nullptr, nullptr);

    // 半透明黑色遮罩
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_Rect overlay = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderFillRect(renderer, &overlay);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    // 角色資訊
    const char* characterNames[2] = {"統神", "國動"};
    const char* characterDesc[2] = {
        "統神-最拉風的辣個男人\naka自費電競選手\n\n特殊技能:使用「只剩一張帥臉」，以他英俊的帥臉使對手被帥到倒在地上無法自拔\n缺點:不擅長端火鍋",
        "國動-瘋狗的外號十歲就有\naka最熟悉的陌生人\n\n特殊技能:使用「瘋狗衝刺」，使對手受到猶如坦克車輾過的衝擊\n缺點:不擅長坐椅子"
    };
    SDL_Rect spriteSrc[2] = {
        {0, 0, 68, 116},    // 統神
        {10, 14, 112, 115}  // 國動
    };
    SDL_Texture* spriteTex[2] = {
        TextureManager::getTexture("blockman_sprites"),
        TextureManager::getTexture("godon_sprites")
    };

    // 區塊寬度與位置
    int blockWidth = SCREEN_WIDTH / 2;
    int blockHeight = 350;
    int blockY = (SCREEN_HEIGHT - blockHeight) / 2;
    int nameY = blockY + 10;
    int spriteY = nameY + 50;
    int descY = spriteY + 130;
    int descMaxWidth = blockWidth - 40;

    for (int i = 0; i < 2; ++i) {
        int blockX = i == 0 ? 0 : blockWidth;
        int centerX = blockX + blockWidth / 2;

        // 角色名稱
        SDL_Surface* nameText = TTF_RenderUTF8_Blended(buttonFont, characterNames[i], {255,255,255,255});
        if (nameText) {
            SDL_Texture* nameTexture = SDL_CreateTextureFromSurface(renderer, nameText);
            SDL_Rect nameRect = {
                centerX - nameText->w / 2,
                nameY,
                nameText->w,
                nameText->h
            };
            SDL_RenderCopy(renderer, nameTexture, nullptr, &nameRect);
            SDL_DestroyTexture(nameTexture);
            SDL_FreeSurface(nameText);
        }

        // 角色圖片
        if (spriteTex[i]) {
            int spriteW = spriteSrc[i].w;
            int spriteH = spriteSrc[i].h;
            SDL_Rect spriteRect = {
                centerX - spriteW / 2,
                spriteY,
                spriteW,
                spriteH
            };
            SDL_RenderCopy(renderer, spriteTex[i], &spriteSrc[i], &spriteRect);
        }

        // 角色介紹（自動換行）
        SDL_Surface* descText = TTF_RenderUTF8_Blended_Wrapped(
            buttonFont, characterDesc[i], {255,255,255,255}, descMaxWidth);
        if (descText) {
            SDL_Texture* descTexture = SDL_CreateTextureFromSurface(renderer, descText);
            SDL_Rect descRect = {
                centerX - descText->w / 2,
                descY,
                descText->w,
                descText->h
            };
            SDL_RenderCopy(renderer, descTexture, nullptr, &descRect);
            SDL_DestroyTexture(descTexture);
            SDL_FreeSurface(descText);
        }
    }

    // 返回提示
    SDL_Surface* hintText = TTF_RenderUTF8_Blended(buttonFont, "按 ESC 返回主選單", {255,255,255,255});
    if (hintText) {
        SDL_Texture* hintTexture = SDL_CreateTextureFromSurface(renderer, hintText);
        SDL_Rect hintRect = {
            SCREEN_WIDTH / 2 - hintText->w / 2,
            SCREEN_HEIGHT - 50,
            hintText->w,
            hintText->h
        };
        SDL_RenderCopy(renderer, hintTexture, nullptr, &hintRect);
        SDL_DestroyTexture(hintTexture);
        SDL_FreeSurface(hintText);
    }
}

void Game::saveGameRecord() {
    GameRecord record;
    
    // 獲取當前時間並格式化
    time_t now = time(0);
    struct tm* timeinfo = localtime(&now);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y/%m/%d %H:%M:%S", timeinfo);
    record.timestamp = std::string(buffer);
    
    // 記錄角色
    record.p1Character = players[0].characterId;
    record.p2Character = players[1].characterId;
    
    // 記錄勝利者
    record.winnerIndex = roundWinnerIndex;
    
    // 記錄遊戲時間
    record.gameTime = ROUND_DURATION - roundTimer;
    
    // 添加到記錄列表
    gameRecords.push_front(record);
    
    // 保持最多5條記錄
    if (gameRecords.size() > static_cast<size_t>(MAX_RECORDS)) {
        gameRecords.pop_back();
    }
    
    // 保存到文件
    std::ofstream file("src/record.txt");
    if (file.is_open()) {
        for (const auto& rec : gameRecords) {
            file << rec.timestamp << "|"
                 << rec.p1Character << "|"
                 << rec.p2Character << "|"
                 << rec.winnerIndex << "|"
                 << rec.gameTime << "\n";
        }
        file.close();
    }
}

void Game::loadGameRecords() {
    gameRecords.clear();
    std::ifstream file("src/record.txt");
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            GameRecord record;
            size_t pos = 0;
            std::string token;
            std::string delimiter = "|";
            
            // 解析時間戳
            pos = line.find(delimiter);
            record.timestamp = line.substr(0, pos);
            line.erase(0, pos + delimiter.length());
            
            // 解析P1角色
            pos = line.find(delimiter);
            record.p1Character = line.substr(0, pos);
            line.erase(0, pos + delimiter.length());
            
            // 解析P2角色
            pos = line.find(delimiter);
            record.p2Character = line.substr(0, pos);
            line.erase(0, pos + delimiter.length());
            
            // 解析勝利者
            pos = line.find(delimiter);
            record.winnerIndex = std::stoi(line.substr(0, pos));
            line.erase(0, pos + delimiter.length());
            
            // 解析遊戲時間
            record.gameTime = std::stof(line);
            
            gameRecords.push_back(record);
        }
        file.close();
    }
}

void Game::renderGameRecords() {
    if (!showRecords) return;
    
    // 創建全螢幕半透明背景
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_Rect bgRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderFillRect(renderer, &bgRect);
    
    // 設置文字顏色
    SDL_Color textColor = {255, 255, 255, 255};
    
    // 渲染標題
    SDL_Surface* titleSurface = TTF_RenderUTF8_Blended(buttonFont, "最近五局遊戲記錄", textColor);
    SDL_Texture* titleTexture = SDL_CreateTextureFromSurface(renderer, titleSurface);
    SDL_Rect titleRect = {SCREEN_WIDTH/2 - titleSurface->w/2, 50, titleSurface->w, titleSurface->h};
    SDL_RenderCopy(renderer, titleTexture, NULL, &titleRect);
    SDL_FreeSurface(titleSurface);
    SDL_DestroyTexture(titleTexture);

    // 欄位 X 座標
    int col1_x = 50;                // 時間
    int col2_x = 400;               // 對戰角色
    int col3_x = 750;               // 勝利玩家

    // 標題
    SDL_Surface* timeTitle = TTF_RenderUTF8_Blended(buttonFont, "時間", textColor);
    SDL_Surface* vsTitle = TTF_RenderUTF8_Blended(buttonFont, "對戰角色", textColor);
    SDL_Surface* winnerTitle = TTF_RenderUTF8_Blended(buttonFont, "勝利玩家", textColor);

    SDL_Texture* timeTitleTex = SDL_CreateTextureFromSurface(renderer, timeTitle);
    SDL_Texture* vsTitleTex = SDL_CreateTextureFromSurface(renderer, vsTitle);
    SDL_Texture* winnerTitleTex = SDL_CreateTextureFromSurface(renderer, winnerTitle);

    int y = 100;
    SDL_Rect timeRect = {col1_x, y, timeTitle->w, timeTitle->h};
    SDL_Rect vsRect = {col2_x, y, vsTitle->w, vsTitle->h};
    SDL_Rect winnerRect = {col3_x, y, winnerTitle->w, winnerTitle->h};

    SDL_RenderCopy(renderer, timeTitleTex, NULL, &timeRect);
    SDL_RenderCopy(renderer, vsTitleTex, NULL, &vsRect);
    SDL_RenderCopy(renderer, winnerTitleTex, NULL, &winnerRect);

    SDL_FreeSurface(timeTitle); SDL_DestroyTexture(timeTitleTex);
    SDL_FreeSurface(vsTitle); SDL_DestroyTexture(vsTitleTex);
    SDL_FreeSurface(winnerTitle); SDL_DestroyTexture(winnerTitleTex);

    // 檢查是否有記錄
    if (gameRecords.empty()) {
        // 顯示無記錄訊息
        SDL_Surface* noRecordSurface = TTF_RenderUTF8_Blended(buttonFont, "無遊戲記錄", textColor);
        SDL_Texture* noRecordTexture = SDL_CreateTextureFromSurface(renderer, noRecordSurface);
        SDL_Rect noRecordRect = {
            SCREEN_WIDTH/2 - noRecordSurface->w/2,
            SCREEN_HEIGHT/2 - noRecordSurface->h/2,
            noRecordSurface->w,
            noRecordSurface->h
        };
        SDL_RenderCopy(renderer, noRecordTexture, NULL, &noRecordRect);
        SDL_FreeSurface(noRecordSurface);
        SDL_DestroyTexture(noRecordTexture);
    } else {
        // 渲染每條記錄
        y += 50;
        int lineHeight = 40;
        for (const auto& record : gameRecords) {
            // 轉換角色名稱為中文
            std::string p1Name = (record.p1Character == "BlockMan") ? "統神" : "國動";
            std::string p2Name = (record.p2Character == "BlockMan") ? "統神" : "國動";
            std::string vsText = p1Name + " vs " + p2Name;
            
            // 轉換勝利者名稱為中文
            std::string winnerText;
            if (record.winnerIndex == -1) {
                winnerText = "平手";
            } else if (record.winnerIndex == 0) {
                winnerText = p1Name;
            } else {
                winnerText = p2Name;
            }

            SDL_Surface* timeSurf = TTF_RenderUTF8_Blended(buttonFont, record.timestamp.c_str(), textColor);
            SDL_Surface* vsSurf = TTF_RenderUTF8_Blended(buttonFont, vsText.c_str(), textColor);
            SDL_Surface* winnerSurf = TTF_RenderUTF8_Blended(buttonFont, winnerText.c_str(), textColor);

            SDL_Texture* timeTex = SDL_CreateTextureFromSurface(renderer, timeSurf);
            SDL_Texture* vsTex = SDL_CreateTextureFromSurface(renderer, vsSurf);
            SDL_Texture* winnerTex = SDL_CreateTextureFromSurface(renderer, winnerSurf);

            SDL_Rect timeR = {col1_x, y, timeSurf->w, timeSurf->h};
            SDL_Rect vsR = {col2_x, y, vsSurf->w, vsSurf->h};
            SDL_Rect winnerR = {col3_x, y, winnerSurf->w, winnerSurf->h};

            SDL_RenderCopy(renderer, timeTex, NULL, &timeR);
            SDL_RenderCopy(renderer, vsTex, NULL, &vsR);
            SDL_RenderCopy(renderer, winnerTex, NULL, &winnerR);

            SDL_FreeSurface(timeSurf); SDL_DestroyTexture(timeTex);
            SDL_FreeSurface(vsSurf); SDL_DestroyTexture(vsTex);
            SDL_FreeSurface(winnerSurf); SDL_DestroyTexture(winnerTex);

            y += lineHeight;
        }
    }
    
    // 渲染返回提示
    SDL_Surface* hintSurface = TTF_RenderUTF8_Blended(buttonFont, "按 ESC 返回主選單", textColor);
    SDL_Texture* hintTexture = SDL_CreateTextureFromSurface(renderer, hintSurface);
    SDL_Rect hintRect = {SCREEN_WIDTH/2 - hintSurface->w/2, SCREEN_HEIGHT - 50, hintSurface->w, hintSurface->h};
    SDL_RenderCopy(renderer, hintTexture, NULL, &hintRect);
    SDL_FreeSurface(hintSurface);
    SDL_DestroyTexture(hintTexture);
}



