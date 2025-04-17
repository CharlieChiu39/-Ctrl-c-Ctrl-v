#include "player.h" // 包含玩家標頭檔
#include <stdio.h>  // 用於 printf 除錯輸出 (可選)
#include <math.h>   // 可能需要數學函數 (例如絕對值 fabsf)

// 假設的攻擊 Hitbox (相對座標)
const SDL_Rect ATTACK_HITBOX_RIGHT = { 40, 30, 40, 30 }; // 面右攻擊框
const SDL_Rect ATTACK_HITBOX_LEFT  = { -30, 30, 40, 30 }; // 面左攻擊框 (x不同)
// 應該在 player.c 開頭附近，和其他 const float 或 const SDL_Rect 一起
const float ATTACK_HITBOX_ACTIVE_START = 0.1f; // <-- 確認這行在且沒被註解
const float ATTACK_HITBOX_ACTIVE_END = 0.3f;   // <-- 確認這行也在且沒被註解

// == 假設 Player 結構體已在 player.h 中擴充 ==


// == 內部輔助函數 ==

// 檢查玩家是否與螢幕邊界碰撞，並進行修正 (內部使用)
static void checkBoundaryCollisionInternal(Player *player, int screenWidth) {
    // 左邊界
    if (player->x < 0) {
        player->x = 0;
        if (player->currentState != PLAYER_STATE_HURT) player->vx = 0; // 撞牆停止 (除非是被打飛)
    }
    // 右邊界
    // 注意: player->bounding_box.w 應該是相對寬度, 但這裡先用常數 PLAYER_WIDTH
    if (player->x + PLAYER_WIDTH > screenWidth) {
        player->x = screenWidth - PLAYER_WIDTH;
        if (player->currentState != PLAYER_STATE_HURT) player->vx = 0;
    }
    // (可選) 上邊界 - 防止跳出畫面
    // if (player->y < 0) {
    //     player->y = 0;
    //     if (player->vy < 0) player->vy = 0;
    // }
}

// 更新動畫幀 (內部使用)
static void updateAnimation(Player *player, float deltaTime) {
    player->frameTimer += deltaTime;
    if (player->frameTimer >= ANIMATION_FRAME_DURATION) {
        player->frameTimer -= ANIMATION_FRAME_DURATION;
        player->currentFrame++;

        // 根據不同狀態決定最大幀數和是否循環
        int maxFrames = 1;
        bool loop = true;

        switch (player->currentState) {
            case PLAYER_STATE_IDLE:      maxFrames = 4; loop = true; break; // 假設站立呼吸有4幀
            case PLAYER_STATE_WALKING:   maxFrames = 6; loop = true; break; // 假設走路有6幀
            case PLAYER_STATE_JUMPING:   maxFrames = 2; loop = false; break;// 假設跳躍起跳2幀
            case PLAYER_STATE_FALLING:   maxFrames = 2; loop = false; break;// 假設下落2幀
            case PLAYER_STATE_ATTACKING: maxFrames = 4; loop = false; break;// 假設攻擊4幀, 不循環
            case PLAYER_STATE_HURT:      maxFrames = 2; loop = false; break;// 假設受傷2幀
            case PLAYER_STATE_DEFEATED:  maxFrames = 1; loop = false; break;// 假設死亡1幀
            // case PLAYER_STATE_CROUCHING: maxFrames = 2; loop = false; break;
            // case PLAYER_STATE_BLOCKING:  maxFrames = 1; loop = false; break;
            // case PLAYER_STATE_SKILL:     maxFrames = 5; loop = false; break;
            default: maxFrames = 1; break;
        }

        if (player->currentFrame >= maxFrames) {
            if (loop) {
                player->currentFrame = 0; // 循環動畫回到第0幀
            } else {
                player->currentFrame = maxFrames - 1; // 停留在最後一幀
            }
        }
    }
    // !! 實際繪圖時 (在 render.c 中):
    // !! renderer 會根據 player->currentState, player->currentFrame, player->direction
    // !! 去查找對應的動畫序列和幀的圖片 (SDL_Texture) 來繪製。
    // !! 例如: drawSprite(textureAtlas, animationMap[player->currentState][player->currentFrame], player->x, player->y, player->direction);
}


// == 函數實作 ==

// 創建並初始化玩家
Player createPlayer(float startX, float startY, int initialDirection) {
    Player newPlayer = {0}; // 清零

    // 位置與移動
    newPlayer.x = startX;
    newPlayer.y = startY;
    newPlayer.vx = 0.0f;
    newPlayer.vy = 0.0f;
    newPlayer.direction = initialDirection;
    newPlayer.isOnGround = false; // 初始假設在空中, update 會檢測

    // 狀態與屬性
    newPlayer.health = PLAYER_INITIAL_HEALTH;
    newPlayer.currentState = PLAYER_STATE_FALLING; // 初始狀態設為下落
    newPlayer.isHittable = true; // 初始可被擊中

    // 碰撞框 (使用預設寬高, 相對偏移為0)
    newPlayer.bounding_box.x = 0;
    newPlayer.bounding_box.y = 0;
    newPlayer.bounding_box.w = PLAYER_WIDTH;
    newPlayer.bounding_box.h = PLAYER_HEIGHT;

    // 初始化計時器和動畫相關 (假設這些成員存在)
    newPlayer.attackTimer = 0.0f;
    newPlayer.hurtTimer = 0.0f;
    newPlayer.skillTimer = 0.0f;
    newPlayer.invincibilityTimer = 0.0f;
    newPlayer.currentFrame = 0;
    newPlayer.frameTimer = 0.0f;
    newPlayer.isAttacking = false;
    newPlayer.isSkillActive = false; // 新增: 技能狀態標記
    newPlayer.current_hitbox = (SDL_Rect){0, 0, 0, 0}; // 初始無攻擊框

    // !! 關於 "載入角色圖片" !!
    // !! 這裡不執行實際的圖片載入 (如 IMG_LoadTexture)。
    // !! 載入應在 render.c 或資源管理器中完成。
    // !! createPlayer 可能需要接收一個 'characterType' 或 指向已載入資源的指標/ID，
    // !! 以便 Player 物件知道自己對應哪些圖形資源。
    // !! 例如: newPlayer.characterGfxId = loadCharacterGraphics("Ryu");

    printf("Player created. Health: %d\n", newPlayer.health);
    return newPlayer;
}

// 處理輸入指令
void handlePlayerCommand(Player *player, PlayerCommand command) {
    // 無法行動的狀態 (硬直, 戰敗)
    if (player->currentState == PLAYER_STATE_HURT ||
        player->currentState == PLAYER_STATE_DEFEATED) {
        return;
    }

    // 某些動作執行中可能不允許被打斷 (例如某些攻擊或技能)
    // if (player->isAttacking || player->isSkillActive) {
    //     // 這裡可以加入更複雜的規則, 例如某些攻擊可以被跳躍取消
    //     return;
    // }

    // 根據指令處理
    switch (command) {
        case CMD_LEFT:
            if (!player->isAttacking && !player->isSkillActive /*&& !player->isBlocking */) { // 假設攻擊/技能/格擋時不能移動
                player->vx = -MOVE_SPEED;
                player->direction = -1;
                if (player->isOnGround && player->currentState != PLAYER_STATE_JUMPING && player->currentState != PLAYER_STATE_FALLING) {
                    player->currentState = PLAYER_STATE_WALKING;
                }
            }
            break;
        case CMD_RIGHT:
             if (!player->isAttacking && !player->isSkillActive /*&& !player->isBlocking */) {
                player->vx = MOVE_SPEED;
                player->direction = 1;
                if (player->isOnGround && player->currentState != PLAYER_STATE_JUMPING && player->currentState != PLAYER_STATE_FALLING) {
                    player->currentState = PLAYER_STATE_WALKING;
                }
            }
            break;
        case CMD_JUMP:
            // 必須在地上且不在執行某些動作時才能跳
            if (player->isOnGround && !player->isAttacking && !player->isSkillActive /*&& !player->isBlocking */) {
                player->vy = -JUMP_STRENGTH;
                player->isOnGround = false;
                player->currentState = PLAYER_STATE_JUMPING;
                player->currentFrame = 0; // 重置跳躍動畫
                player->frameTimer = 0.0f;
            }
            break;
        case CMD_ATTACK: // 代表基礎拳擊/踢擊
            // 必須在地面或空中特定狀態, 且不在執行其他動作時
             if (!player->isAttacking && !player->isSkillActive /*&& !player->isBlocking */) {
                player->currentState = PLAYER_STATE_ATTACKING;
                player->isAttacking = true;
                player->attackTimer = ATTACK_DURATION; // 設定攻擊持續時間
                player->vx = 0; // 攻擊時停止水平移動 (可選)
                player->currentFrame = 0; // 重置攻擊動畫幀
                player->frameTimer = 0.0f;

                // 根據方向設定 Hitbox
                 if (player->direction == 1) {
                    player->current_hitbox = ATTACK_HITBOX_RIGHT;
                } else {
                    player->current_hitbox = ATTACK_HITBOX_LEFT;
                }
                 printf("Player Attack! Hitbox active between %.2fs and %.2fs\n", ATTACK_HITBOX_ACTIVE_START, ATTACK_HITBOX_ACTIVE_END);
            }
            break;
        // --- 以下是新增的動作 ---
        case CMD_CROUCH_START: // 開始蹲下 (假設有個 CMD_CROUCH_START)
             if (player->isOnGround && !player->isAttacking && !player->isSkillActive /*&& !player->isBlocking */) {
                 // player->currentState = PLAYER_STATE_CROUCHING;
                 player->vx = 0;
                 // 調整碰撞框高度?
                 // player->bounding_box.h = PLAYER_HEIGHT * CROUCH_HEIGHT_RATIO;
                 // player->bounding_box.y = PLAYER_HEIGHT * (1.0f - CROUCH_HEIGHT_RATIO); // Offset y
                 printf("Player Crouch Start\n");
             }
            break;
         case CMD_CROUCH_END: // 結束蹲下
             // if (player->currentState == PLAYER_STATE_CROUCHING) {
             //     player->currentState = PLAYER_STATE_IDLE;
                 // 恢復碰撞框高度
                 // player->bounding_box.h = PLAYER_HEIGHT;
                 // player->bounding_box.y = 0;
                 // printf("Player Crouch End\n");
            // }
            break;
         case CMD_BLOCK_START: // 開始格擋
             if (player->isOnGround && !player->isAttacking && !player->isSkillActive) {
                 // player->currentState = PLAYER_STATE_BLOCKING;
                 // player->isBlocking = true;
                 player->vx = 0; // 格擋時不能移動
                 printf("Player Block Start\n");
             }
            break;
         case CMD_BLOCK_END: // 結束格擋
            // if (player->currentState == PLAYER_STATE_BLOCKING) {
            //      player->currentState = PLAYER_STATE_IDLE;
            //      player->isBlocking = false;
            //      printf("Player Block End\n");
            // }
            break;
        case CMD_SKILL: // 釋放技能
            if (player->isOnGround && !player->isAttacking && !player->isSkillActive /*&& !player->isBlocking */) {
                // player->currentState = PLAYER_STATE_SKILL; // 可能有多種技能狀態
                // player->isSkillActive = true;
                // player->skillTimer = SKILL_DURATION;
                player->vx = 0; // 技能時可能不能移動
                player->currentFrame = 0;
                player->frameTimer = 0.0f;
                // 這裡可以觸發技能特效、產生飛行道具等邏輯
                printf("Player Use Skill!\n");
            }
            break;
        // --- End of new actions ---

        case CMD_NONE: // 沒有左右移動指令 (且不在其他狀態)
             if (player->isOnGround &&
                 player->currentState != PLAYER_STATE_IDLE &&
                 player->currentState != PLAYER_STATE_ATTACKING &&
                 player->currentState != PLAYER_STATE_HURT /* && others... */ )
             {
                 // 如果在走路, 停止變成 IDLE
                 if(player->currentState == PLAYER_STATE_WALKING) {
                     player->vx = 0;
                     player->currentState = PLAYER_STATE_IDLE;
                 }
                 // 其他情況可能也需要回到 IDLE, 例如攻擊/技能結束後沒有輸入
            }
            // 如果在空中沒有水平輸入, 仍然保持當前狀態 (JUMPING/FALLING)
            break;
    }
}

// 更新玩家狀態
void updatePlayer(Player *player, float deltaTime, int groundLevel) {
    // 戰敗狀態，不更新邏輯
    if (player->currentState == PLAYER_STATE_DEFEATED) {
        // updateAnimation(player, deltaTime); // 可能還需要更新死亡動畫
        return;
    }

    // 更新計時器
    if (player->attackTimer > 0) {
        player->attackTimer -= deltaTime;
        if (player->attackTimer <= 0) {
            player->isAttacking = false;
            player->current_hitbox = (SDL_Rect){0, 0, 0, 0}; // 清除攻擊框
            // 攻擊結束後, 若無其他輸入且在地面, 回到 IDLE
            if (player->isOnGround && player->currentState == PLAYER_STATE_ATTACKING) {
                player->currentState = PLAYER_STATE_IDLE;
            }
        }
    }
     if (player->hurtTimer > 0) {
        player->hurtTimer -= deltaTime;
        if (player->hurtTimer <= 0) {
             // 硬直結束後, 若無其他輸入且在地面, 回到 IDLE
             // (注意: 落地檢測那裡也會處理從空中硬直狀態恢復)
            if (player->currentState == PLAYER_STATE_HURT && player->isOnGround) {
                 player->currentState = PLAYER_STATE_IDLE;
            }
        }
    }
     if (player->invincibilityTimer > 0) {
        player->invincibilityTimer -= deltaTime;
        if (player->invincibilityTimer <= 0) {
             player->isHittable = true; // 無敵結束
        }
    }
    // if (player->skillTimer > 0) { // 更新技能計時器
    //     player->skillTimer -= deltaTime;
    //     if (player->skillTimer <= 0) {
    //         player->isSkillActive = false;
    //         if (player->isOnGround && player->currentState == PLAYER_STATE_SKILL) {
    //              player->currentState = PLAYER_STATE_IDLE;
    //         }
    //     }
    // }

    // --- 物理和狀態更新 (只有非硬直狀態下才應用速度和重力) ---
    if (player->currentState != PLAYER_STATE_HURT) {
        // 應用重力
        if (!player->isOnGround) {
            player->vy += GRAVITY * deltaTime;
        }
        // 更新位置
        player->x += player->vx * deltaTime;
        player->y += player->vy * deltaTime;

        // 狀態轉換 (基於物理)
        if (!player->isOnGround) {
            if (player->vy > 0 && player->currentState == PLAYER_STATE_JUMPING) {
                player->currentState = PLAYER_STATE_FALLING; // 從跳躍最高點開始下落
                 player->currentFrame = 0; // 重置下落動畫
                 player->frameTimer = 0.0f;
            } else if (player->currentState != PLAYER_STATE_JUMPING && player->currentState != PLAYER_STATE_FALLING && player->currentState != PLAYER_STATE_ATTACKING /* && others */) {
                 // 如果意外離地 (例如從平台掉落), 進入下落狀態
                 player->currentState = PLAYER_STATE_FALLING;
                 player->currentFrame = 0;
                 player->frameTimer = 0.0f;
            }
        }
    } else { // 硬直狀態下的移動 (通常是被擊退)
         player->x += player->vx * deltaTime; // 假設受傷時 vx, vy 已被設定
         player->y += player->vy * deltaTime;
         if (!player->isOnGround) {
            player->vy += GRAVITY * deltaTime; // 硬直時也受重力
        }
         // 可能需要加入摩擦力讓擊退速度減慢
         // player->vx *= 0.95f;
    }


    // --- 地面檢測 ---
    if (player->y + PLAYER_HEIGHT >= groundLevel) { // 假設 Player 高度為 PLAYER_HEIGHT
        if (player->vy >= 0) { // 只有在下落或靜止時才觸發地面碰撞
            player->y = groundLevel - PLAYER_HEIGHT;
            player->vy = 0;
            if (!player->isOnGround) { // 剛剛落地
                player->isOnGround = true;
                // 根據落地前的狀態決定落地後的狀態
                if (player->currentState == PLAYER_STATE_JUMPING || player->currentState == PLAYER_STATE_FALLING) {
                    // 落地後根據水平速度決定是站立還是走路
                    player->currentState = (fabsf(player->vx) < 0.1f) ? PLAYER_STATE_IDLE : PLAYER_STATE_WALKING;
                } else if (player->currentState == PLAYER_STATE_HURT) {
                     // 從空中硬直狀態落地, 如果硬直時間已到, 恢復 IDLE
                     if(player->hurtTimer <= 0) {
                         player->currentState = PLAYER_STATE_IDLE;
                         player->vx = 0; // 清除擊退速度
                     }
                     // 如果硬直時間未到, 保持 HURT 狀態直到時間到或再次被攻擊
                }
                 player->currentFrame = 0; // 重置落地後的動畫幀
                 player->frameTimer = 0.0f;
            }
        }
    } else {
        player->isOnGround = false;
    }

    // --- 邊界碰撞 ---
    checkBoundaryCollisionInternal(player, 1280); // 假設螢幕寬度 1280, 應從外部傳入

    // --- 更新動畫 ---
    updateAnimation(player, deltaTime);

    // --- 處理碰撞判定 (說明) ---
    // !! 實際的玩家間碰撞判定不在此處進行 !!
    // !! 應在 game.c 或 main.c 的主迴圈中:
    // !! 1. 對每個玩家呼叫 updatePlayer()
    // !! 2. 獲取玩家 P1 的攻擊框: hitbox1 = getPlayerHitboxWorld(&player1);
    // !! 3. 獲取玩家 P2 的受擊框: hurtbox2 = getPlayerBoundingBox(&player2); // 或更精確的 hurtbox
    // !! 4. 檢查碰撞: if (SDL_HasIntersection(&hitbox1, &hurtbox2)) {
    // !! 5.    如果碰撞，且 P1 在攻擊，且 P2 可被擊中: playerTakesDamage(&player2, damage);
    // !! 6. 對 P2 攻擊 P1 做同樣的檢查。
}

// 計算並獲取玩家的世界座標邊界框
SDL_Rect getPlayerBoundingBox(const Player *player) {
    SDL_Rect worldBox = player->bounding_box; // 使用結構體中定義的相對框
    worldBox.x += (int)player->x;
    worldBox.y += (int)player->y;
    // 注意: 如果實作了蹲下改變 bounding_box, 這裡會自動反映
    return worldBox;
}

// ** (內部輔助，或移到 game.c) **
// 計算並獲取玩家的世界座標攻擊框 (Hitbox)
// 這個函數是碰撞判定的關鍵部分, 可能需要更複雜的邏輯
SDL_Rect getPlayerHitboxWorld(const Player *player) {
    SDL_Rect worldBox = {0, 0, 0, 0}; // 默認無效

    // 只有在攻擊狀態, 且在 Hitbox 生效時間內
    if (player->isAttacking &&
        player->attackTimer > (ATTACK_DURATION - ATTACK_HITBOX_ACTIVE_END) &&
        player->attackTimer <= (ATTACK_DURATION - ATTACK_HITBOX_ACTIVE_START))
    {
        // 根據當前攻擊類型獲取相對 Hitbox
        if (player->currentState == PLAYER_STATE_ATTACKING) { // 假設只有一種攻擊
            worldBox = player->current_hitbox; // 使用攻擊時設定的相對框
            worldBox.x += (int)player->x;
            worldBox.y += (int)player->y;
        }
        // else if (player->currentState == PLAYER_STATE_SKILL && ...) {
        //    // 技能可能有不同的 hitbox
        // }
    }
    return worldBox;
}


// 對玩家施加傷害
void playerTakesDamage(Player *player, int damage) {
    // 如果處於不可被擊中狀態 (例如剛受傷後的無敵幀), 或已戰敗, 則不處理
    if (!player->isHittable || player->currentState == PLAYER_STATE_DEFEATED) {
        return;
    }

    // !! 格擋邏輯 !!
    // if (player->isBlocking && playerFacingOpponent) { // 需要判斷是否面朝對手
    //     printf("Player Blocked!\n");
    //     // 播放格擋特效/音效, 可能有 chip damage (少量穿透傷害)
    //     // player->health -= chip_damage;
    //     // 可能需要觸發一個短暫的格擋硬直狀態 Block Stun
    //     return; // 阻止後續的受傷處理
    // }


    printf("Player (Health %d) takes %d damage!\n", player->health, damage);
    player->health -= damage;

    // 清除當前可能正在進行的動作 (攻擊、技能等)
    player->isAttacking = false;
    player->attackTimer = 0;
    player->isSkillActive = false;
    player->skillTimer = 0;
    player->current_hitbox = (SDL_Rect){0, 0, 0, 0};
    player->vx = 0; // 清除自身移動速度

    if (player->health <= 0) {
        player->health = 0;
        player->currentState = PLAYER_STATE_DEFEATED;
        player->vy = 0; // 死亡時可能停止所有速度
        player->currentFrame = 0; // 設置死亡動畫幀
        player->frameTimer = 0.0f;
        printf("Player Defeated!\n");
    } else {
        // 進入受傷硬直狀態
        player->currentState = PLAYER_STATE_HURT;
        player->hurtTimer = HURT_DURATION;
        player->currentFrame = 0; // 設置受傷動畫幀
        player->frameTimer = 0.0f;

        // 施加擊退效果 (可選, 這裡給一個簡單的例子)
        // player->vx = -player->direction * 100.0f; // 向後彈開
        // player->vy = -200.0f; // 向上彈起
        // player->isOnGround = false; // 被擊飛離地

        // 觸發短暫無敵
        player->isHittable = false;
        player->invincibilityTimer = HURT_INVINCIBILITY_DURATION;
        printf("Player Hurt! Invincible for %.2fs\n", HURT_INVINCIBILITY_DURATION);
    }
}