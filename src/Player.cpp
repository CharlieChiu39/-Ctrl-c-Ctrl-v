#include "Player.h"
#include "TextureManager.h" // 需要使用 TextureManager
#include "AnimationData.h" // 需要使用 AnimationDataManager
#include "Constants.h"       // 需要核心常數
#include "AudioManager.h"
#include <cmath>                     // for fabsf
#include <stdio.h>                   // for printf


Player::Player(float startX, float startY, int startDir,
               const std::string& charId, const std::string& texId) :
    x(startX), y(startY), vx(0.0f), vy(0.0f),
    health(PLAYER_DEFAULT_HEALTH), // 使用常數
    direction(startDir),
    state(PlayerState::FALLING), // 初始狀態
    characterId(charId), textureId(texId), // 儲存 ID
    attackTimer(0.0f), attackCooldownTimer(0.0f), hurtTimer(0.0f), invincibilityTimer(0.0f), blockCooldownTimer(0.0f),
    attackRateCooldownTimer(0.0f),
    projectileCooldownTimer(0.0f),
    isOnGround(false),
    shouldFireProjectile(false),
    currentFrame(0), frameTimer(0.0f),
    currentAnimationType(AnimationType::FALL) // 初始動畫類型
{
    printf("Player created: CharacterID='%s', TextureID='%s'\n", characterId.c_str(), textureId.c_str());
}

// 封裝狀態改變邏輯
void Player::changeState(PlayerState newState) {
    if (state == newState) return; // 狀態沒變，不做事

    // printf("Player %s changing state from %d to %d\n", characterId.c_str(), static_cast<int>(state), static_cast<int>(newState));
    PlayerState oldState = state;
    printf("[State] Player %s changing state from %d to %d\n",
    characterId.c_str(), static_cast<int>(oldState), static_cast<int>(newState));
    state = newState;
    currentFrame = 0; // 重置動畫幀
    frameTimer = 0.0f; // 重置幀計時器

    // 根據新狀態設定對應的動畫類型
    switch (newState) {
        case PlayerState::IDLE:      currentAnimationType = AnimationType::IDLE; break;
        case PlayerState::WALKING:   currentAnimationType = AnimationType::WALK; break;
        case PlayerState::JUMPING:   currentAnimationType = AnimationType::JUMP; break;
        case PlayerState::FALLING:   currentAnimationType = AnimationType::FALL; break;
        case PlayerState::ATTACKING: currentAnimationType = AnimationType::ATTACK; break;
        case PlayerState::HURT:      currentAnimationType = AnimationType::HURT; break;
        case PlayerState::BLOCKING:  currentAnimationType = AnimationType::BLOCK; break;
        case PlayerState::DEATH:     currentAnimationType = AnimationType::DEATH; break;
        case PlayerState::VICTORY:   currentAnimationType = AnimationType::VICTORY; break; // <--- 新增處理
        default:                    currentAnimationType = AnimationType::IDLE; break; // 預設
    }



}


void Player::handleAction(const std::string& action) {

    // 允許在受傷或死亡時執行 STOP_BLOCK (如果玩家在這些狀態下放開按鍵)
    if (action == "STOP_BLOCK" && state == PlayerState::BLOCKING) {
        changeState(PlayerState::IDLE);
        // 在這裡確保冷卻被觸發 (如果 changeState 不觸發了)
        if (blockCooldownTimer <= 0) { // 避免因其他原因快速切換導致重複觸發
             blockCooldownTimer = BLOCK_COOLDOWN;
              printf("Player %s Block Cooldown Started (%.1fs) - From STOP_BLOCK\n", characterId.c_str(), BLOCK_COOLDOWN);
        }
        return;
    }

    if (state == PlayerState::HURT || health <= 0) return; // 受傷或死亡時不接受控制



    // --- 處理 BLOCK ---
    if (action == "BLOCK" && isOnGround && blockCooldownTimer <= 0 &&
        state != PlayerState::ATTACKING && state != PlayerState::JUMPING && state != PlayerState::FALLING)
    {
        // 可以在 IDLE 或 WALKING 時格擋
        vx = 0; // 格擋時停止移動
        changeState(PlayerState::BLOCKING);
    }

    // --- 處理氣功發射 ---
    else if (action == "FIRE_PROJECTILE") {
        if (canFireProjectile()) {
            printf("Player %s handling FIRE_PROJECTILE action.\n", characterId.c_str());
            changeState(PlayerState::ATTACKING); // <-- 切換到攻擊狀態來播放動畫
            // 注意：攻擊狀態可能會讓 vx = 0，如果希望發射時能移動，需要調整 ATTACKING 狀態的邏輯
            vx = 0; // 遵循原本攻擊狀態的邏輯，發射時停下
            attackTimer = ATTACK_DURATION;
            resetProjectileCooldown();          // <-- 重置冷卻
            shouldFireProjectile = true;        // <-- 設定請求標記
            AudioManager::playSound("fire"); 
        }
    }

    // --- 處理其他動作 (只有在非 Blocking 狀態下才執行) ---
    else if (state != PlayerState::BLOCKING) {
         if (action == "LEFT" && state != PlayerState::ATTACKING) {
            vx = -MOVE_SPEED; direction = -1;
            if (isOnGround) changeState(PlayerState::WALKING);
        } else if (action == "RIGHT" && state != PlayerState::ATTACKING) {
            vx = MOVE_SPEED; direction = 1;
            if (isOnGround) changeState(PlayerState::WALKING);
        } else if (action == "JUMP" && isOnGround && state != PlayerState::ATTACKING) {
            vy = -JUMP_STRENGTH; isOnGround = false;
            changeState(PlayerState::JUMPING);
            // AudioManager::playSoundEffect("sfx_jump"); // 假設有音效
        } else if (action == "ATTACK" &&
            state != PlayerState::ATTACKING &&  // 不能在攻擊中再攻擊
            state != PlayerState::HURT &&      // 不能在受傷中攻擊
            state != PlayerState::BLOCKING &&  // 不能在格擋中攻擊 (可選)
            attackCooldownTimer <= 0 &&      // 確保攻擊動畫恢復完成 (原本的 cooldown)
            attackRateCooldownTimer <= 0 &&
            !shouldFireProjectile) {
            attackTimer = ATTACK_DURATION;
            attackCooldownTimer = ATTACK_DURATION + ATTACK_COOLDOWN;
            attackRateCooldownTimer = ATTACK_RATE_COOLDOWN;
            vx = 0; // 攻擊時停止水平移動
            changeState(PlayerState::ATTACKING);
            // AudioManager::playSoundEffect("sfx_attack"); // 假設有音效
        } else if (action == "STOP_X" && state == PlayerState::WALKING) {
            vx = 0;
            if (isOnGround) changeState(PlayerState::IDLE);
        }
    }
}

void Player::update(float deltaTime) {

    // 處理 VICTORY 狀態
    if (state == PlayerState::VICTORY) {
        updateAnimation(deltaTime); // 勝利時只更新動畫
        vx = 0; // 停止移動
        vy = 0; // 確保不會因重力下落 (如果剛好在空中獲勝)
        // 可以考慮確保他在地面上
        if (!isOnGround) {
             y = GROUND_LEVEL - PLAYER_LOGIC_HEIGHT;
             isOnGround = true;
        }
        return; // 直接返回，不執行後續的物理、狀態轉換等
    }


    //新增死亡狀態的處理
    if (state == PlayerState::DEATH) {
        updateAnimation(deltaTime); // 死亡時只更新動畫
        // 可以在此添加其他死亡時的行為，例如緩慢下沉

        return; // 直接返回，不執行後續邏輯
    }

     // --- 更新計時器 ---
     if (attackTimer > 0) attackTimer -= deltaTime;
     if (attackCooldownTimer > 0) attackCooldownTimer -= deltaTime;
     if (hurtTimer > 0) hurtTimer -= deltaTime;
     if (invincibilityTimer > 0) invincibilityTimer -= deltaTime;
     if (blockCooldownTimer > 0) blockCooldownTimer -= deltaTime;
     if (attackRateCooldownTimer > 0) attackRateCooldownTimer -= deltaTime; 
     if (projectileCooldownTimer > 0) projectileCooldownTimer -= deltaTime;

     // --- 狀態自動轉換 (基於計時器) ---
     // (只有在非格擋狀態下，攻擊/受傷計時器到了才變回 IDLE)
    if (state == PlayerState::ATTACKING && attackTimer <= 0) {
        changeState(PlayerState::IDLE);
    }
    if (state == PlayerState::HURT && hurtTimer <= 0) {
        changeState(PlayerState::IDLE);
    }
    if (state == PlayerState::DEATH) {
         updateAnimation(deltaTime); // 死亡時只更新動畫
         // 可以在此添加其他死亡時的行為，例如緩慢下沉
         // vy = 0; // 停止垂直移動，如果希望停在原地
         // vx = 0; // 停止水平移動
         return; // 直接返回，不執行後續邏輯
     }

     // --- 物理更新 ---
     if (state == PlayerState::BLOCKING) { // 格擋時特殊處理
        vx = 0; // 不能移動
        if (!isOnGround) { // 如果在空中格擋？(目前設計是在地面才能啟動)
            vy += GRAVITY * deltaTime; // 仍然受重力
            y += vy * deltaTime;
        } else {
            vy = 0; // 在地面格擋，垂直速度為 0
        }
    } else if (state != PlayerState::HURT) { // 非格擋非受傷時正常物理
        if (!isOnGround) vy += GRAVITY * deltaTime;
        x += vx * deltaTime;
        y += vy * deltaTime;
    } else { // 受傷時的物理
        vx = 0;
        if (!isOnGround) { vy += GRAVITY * deltaTime; y += vy * deltaTime; }
    }

     // --- 地面檢測與處理 ---
     // (基本不變，但落地時的狀態轉換需要考慮 Blocking)
    if (y + PLAYER_LOGIC_HEIGHT >= GROUND_LEVEL && vy >= 0) {
        y = GROUND_LEVEL - PLAYER_LOGIC_HEIGHT; vy = 0;
        if (!isOnGround) {
            isOnGround = true;
             // 只有在非格擋狀態下落地才轉換成 IDLE/WALKING
             if (state != PlayerState::BLOCKING) {
                 if (state == PlayerState::JUMPING || state == PlayerState::FALLING ||
                     (state == PlayerState::HURT && hurtTimer <= 0)) {
                     changeState((fabsf(vx) < 1.0f) ? PlayerState::IDLE : PlayerState::WALKING);
                 } else if (state == PlayerState::ATTACKING) {
                     attackTimer = 0;
                     changeState((fabsf(vx) < 1.0f) ? PlayerState::IDLE : PlayerState::WALKING);
                 }
             }
        }
    } else if (y + PLAYER_LOGIC_HEIGHT < GROUND_LEVEL) { // 在空中
        isOnGround = false;
        // 只有在 IDLE/WALKING 狀態下離開地面才變成 FALLING
        if (state == PlayerState::IDLE || state == PlayerState::WALKING) {
            changeState(PlayerState::FALLING);
        }
    }

     // --- 邊界檢測 ---
     if (x < 0) x = 0;
     if (x + PLAYER_LOGIC_WIDTH > SCREEN_WIDTH) x = SCREEN_WIDTH - PLAYER_LOGIC_WIDTH;

     // --- 更新動畫 ---
     updateAnimation(deltaTime);
}

void Player::updateAnimation(float deltaTime) {
    // 取得目前動畫類型的資料
    const AnimationInfo* animInfo = AnimationDataManager::getAnimationInfo(characterId, currentAnimationType);
    if (!animInfo || animInfo->frameCount <= 0) return; // 沒有動畫資料或沒有幀

    frameTimer += deltaTime;
    if (frameTimer >= animInfo->frameDuration) { // 使用該動畫指定的幀持續時間
        frameTimer -= animInfo->frameDuration;
        currentFrame++;

        // 處理幀數循環或停留
        if (currentFrame >= animInfo->frameCount) {
            if (animInfo->loop) {
                currentFrame = 0; // 循環
            } else {
                currentFrame = animInfo->frameCount - 1; // 停在最後一幀
                // 對於非循環動畫，可以在這裡觸發事件或狀態改變，但目前 update 裡處理了
            }
        }
    }
}

void Player::render(SDL_Renderer* renderer) {

    // 從 TextureManager 取得紋理
    SDL_Texture* texture = TextureManager::getTexture(textureId);
    if (!texture) {
        printf("Error: Texture '%s' not found for player '%s'\n", textureId.c_str(), characterId.c_str());
        // 可以畫一個預設方塊代替
        SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255); // 紫紅色表示錯誤
        SDL_Rect errorRect = getBoundingBox();
        SDL_RenderFillRect(renderer, &errorRect);
        return;
    }

    // 取得目前動畫幀的來源矩形
    const AnimationInfo* animInfo = AnimationDataManager::getAnimationInfo(characterId, currentAnimationType);
    if (!animInfo || currentFrame >= animInfo->frameCount) return; // 沒有動畫或幀索引錯誤
    SDL_Rect srcRect = animInfo->frames[currentFrame];

    // 計算目標繪製矩形 (使用邏輯尺寸)
    SDL_Rect destRect = { (int)x, (int)y, PLAYER_LOGIC_WIDTH, PLAYER_LOGIC_HEIGHT };

    // 決定翻轉
    SDL_RendererFlip flip = (direction == 1) ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;

    // --- 處理無敵閃爍 ---
    bool drawPlayer = true;
    if (invincibilityTimer > 0 && fmod(invincibilityTimer, 0.2f) < 0.1f) {
        drawPlayer = false;
    }

    // --- 繪製角色 ---
    if (drawPlayer) {
        SDL_RenderCopyEx(renderer, texture, &srcRect, &destRect, 0.0, NULL, flip);
    }

    // --- (可選) 繪製攻擊特效 ---
    if (state == PlayerState::ATTACKING) {
        SDL_Rect worldHitbox = getHitboxWorld();
        SDL_Rect attackEffectSrcRect = {996, 1811, 60, 52};
        SDL_Rect* pSrcRect = NULL;
        pSrcRect = &attackEffectSrcRect;
        if (worldHitbox.w > 0 ) {
           SDL_Rect effectDestRect = worldHitbox;
           SDL_RenderCopyEx(renderer, texture, pSrcRect, &effectDestRect, 0.0, NULL, flip);
        }
        }
    }



void Player::takeDamage(int damage) {
    if (invincibilityTimer > 0  || state == PlayerState::DEATH) return;

    printf("[Damage Check] Player %s Current State: %d (Is it BLOCKING? %d)\n",
    characterId.c_str(), static_cast<int>(state), static_cast<int>(PlayerState::BLOCKING));

    // --- 格擋成功判斷 ---
    if (state == PlayerState::BLOCKING) {
        printf("Player %s BLOCKED the attack!\n", characterId.c_str());
        // 可以在這裡加點格擋特效或音效
        // AudioManager::playSoundEffect("sfx_block");
        // 觸發一次短暫的格擋動畫/效果？ (可選)

        // 觸發冷卻時間
        if (blockCooldownTimer <= 0) { // 避免重複觸發冷卻
            blockCooldownTimer = BLOCK_COOLDOWN; // 使用 Constants.h 的值
            printf("Player %s Block Cooldown Started (%.1fs) - From Successful Block\n", characterId.c_str(), BLOCK_COOLDOWN);
       }

       //  重要：格擋成功後，立刻改變狀態 
       //changeState(PlayerState::IDLE); // <--- 將這一行加回來！

       return; // 阻擋傷害，直接返回
    }

    health -= damage;
    printf("Player %s took %d damage, health: %d\n", characterId.c_str(), damage, health);

    if (health <= 0) {
        health = 0;
        printf("Player %s defeated!\n", characterId.c_str());
        changeState(PlayerState::DEATH);
        AudioManager::playSound("death");
        // TODO: 可能需要一個 DEFEATED 狀態
    } else {
        changeState(PlayerState::HURT); // 切換到受傷狀態
        hurtTimer = HURT_DURATION;
        invincibilityTimer = HURT_INVINCIBILITY;
        vx = 0;
        vy = -100.0f; // 受傷彈跳
        isOnGround = false;
        attackTimer = 0; // 中斷攻擊
        AudioManager::playSound("hurt");
    }
}

SDL_Rect Player::getBoundingBox() const {
    return {(int)x, (int)y, PLAYER_LOGIC_WIDTH, PLAYER_LOGIC_HEIGHT};
}

// 計算相對於自身的攻擊框 (Hitbox)
SDL_Rect Player::calculateRelativeHitbox() const {
    // TODO: 這個應該根據角色和攻擊類型來決定，可以從 AnimationData 或角色配置讀取
    // 暫時使用 main (2).cpp 的硬編碼值
    int hitboxWidth = 40;
    int hitboxHeight = 20;
    int offsetX = (direction == 1) ? PLAYER_LOGIC_WIDTH : -hitboxWidth; // X 偏移量
    int offsetY = PLAYER_LOGIC_HEIGHT / 4; // Y 偏移量
    return {offsetX, offsetY, hitboxWidth, hitboxHeight};
}


SDL_Rect Player::getHitboxWorld() const {
    if (state == PlayerState::ATTACKING && attackTimer > 0) {
        float attackEndTime = ATTACK_DURATION; // 假設從 Constants.h 讀取
        // 檢查是否在 hitbox 生效時間內
        if (attackTimer > (attackEndTime - ATTACK_HITBOX_ACTIVE_END) &&
            attackTimer <= (attackEndTime - ATTACK_HITBOX_ACTIVE_START))
        {
            SDL_Rect relativeHitbox = calculateRelativeHitbox();
            // 計算世界座標
            return {(int)x + relativeHitbox.x, (int)y + relativeHitbox.y, relativeHitbox.w, relativeHitbox.h};
        }
    }
    return {0, 0, 0, 0}; // 無效 hitbox
}

bool Player::canFireProjectile() const {
    // 可以在這裡加入更多條件，例如必須在地面，不能在攻擊/受傷/格擋/跳躍/下落中
    if (state == PlayerState::ATTACKING || state == PlayerState::HURT || state == PlayerState::BLOCKING) {
        // 只有在 攻擊/受傷/格擋 時不能發
        return false;
    }
    return projectileCooldownTimer <= 0 && isAlive(); // 基本條件：冷卻結束且活著
}

void Player::resetProjectileCooldown() {
    projectileCooldownTimer = PROJECTILE_COOLDOWN; // 使用 Constants.h 的值
}

bool Player::isAlive() const {
    // 簡單判斷是否活著 (方便在 Game::checkCollisions 中使用)
    return health > 0 && state != PlayerState::DEATH;
}

bool Player::isControllable() const {
    // 根據目前的狀態判斷是否可被控制
    switch (state) {
        case PlayerState::IDLE:
        case PlayerState::WALKING:
        case PlayerState::JUMPING: // 通常跳躍中也可以左右移動
        case PlayerState::FALLING: // 下落中也可以左右移動
            return true; // 這些狀態下可以控制
        case PlayerState::ATTACKING:
        case PlayerState::BLOCKING:
        case PlayerState::HURT:
        case PlayerState::DEATH:
        case PlayerState::VICTORY: // <--- 新增：勝利時不可控制
        default:
            return false; // 這些狀態下不可控制
    }
}