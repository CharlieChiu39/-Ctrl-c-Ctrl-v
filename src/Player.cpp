#include "Player.h"
#include "TextureManager.h" // 需要使用 TextureManager
#include "AnimationData.h" // 需要使用 AnimationDataManager
#include "Constants.h"       // 需要核心常數
#include "AudioManager.h"
#include <cmath>                     // for fabsf
#include <stdio.h>                   // for printf

// 拳套相關的常數
const float LIGHT_GLOVE_COOLDOWN = 0.8f;    // 10oz 拳套冷卻時間
const float MEDIUM_GLOVE_COOLDOWN = 1.0f;   // 14oz 拳套冷卻時間
const float HEAVY_GLOVE_COOLDOWN = 1.2f;    // 18oz 拳套冷卻時間

const int LIGHT_GLOVE_DAMAGE = 8;          // 10oz 拳套傷害
const int MEDIUM_GLOVE_DAMAGE = 10;         // 14oz 拳套傷害
const int HEAVY_GLOVE_DAMAGE = 12;          // 18oz 拳套傷害

Player::Player(float startX, float startY, int startDir,
               const std::string& charId, const std::string& texId) :
    x(startX), y(startY), vx(0.0f), vy(0.0f),
    health(PLAYER_DEFAULT_HEALTH),
    direction(startDir),
    state(PlayerState::IDLE),
    characterId(charId), textureId(texId),
    currentGlove(GloveType::LIGHT_10OZ),
    attackTimer(0.0f), attackCooldownTimer(0.0f), hurtTimer(0.0f), invincibilityTimer(0.0f), blockCooldownTimer(0.0f),
    attackRateCooldownTimer(0.0f),
    projectileCooldownTimer(0.0f),
    specialAttackCooldownTimer(0.0f),
    isOnGround(true),
    shouldFireProjectile(false),
    currentFrame(0), frameTimer(0.0f),
    currentAnimationType(AnimationType::IDLE),
    isSpecialAttacking(false),
    hasHitDuringDash(false)
{
    // 根據角色設定尺寸
    if (charId == "Godon") {
        logicWidth = GODON_LOGIC_WIDTH;
        logicHeight = GODON_LOGIC_HEIGHT;
    } else { // BlockMan 或其他角色
        logicWidth = BLOCKMAN_LOGIC_WIDTH;
        logicHeight = BLOCKMAN_LOGIC_HEIGHT;
    }
    printf("Player created: CharacterID='%s', TextureID='%s', Size=%dx%d\n", 
           characterId.c_str(), textureId.c_str(), logicWidth, logicHeight);
}

// 封裝狀態改變邏輯
void Player::changeState(PlayerState newState) {
    if (state == newState) return; // 狀態沒變，不做事
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
        case PlayerState::VICTORY:   currentAnimationType = AnimationType::VICTORY; break;
        case PlayerState::LYING:     currentAnimationType = AnimationType::LYING; break;
        default:                    currentAnimationType = AnimationType::IDLE; break;
    }
}

void Player::handleAction(const std::string& action) {
    if (state == PlayerState::LYING && hurtTimer > 0) return;
    if (action == "STOP_BLOCK" && state == PlayerState::BLOCKING) {
        changeState(PlayerState::IDLE);
        if (blockCooldownTimer <= 0) {
            blockCooldownTimer = BLOCK_COOLDOWN;
        }
        return;
    }
    if (health <= 0 || state == PlayerState::HURT) return;

    // --- 處理特殊攻擊 ---
    if ((action == "SPECIAL_ATTACK" || action == "SPECIAL_ATTACK_NUMPAD") && 
        canUseSpecialAttack() && 
        state != PlayerState::ATTACKING && 
        state != PlayerState::HURT && 
        state != PlayerState::BLOCKING) {
        attackTimer = ATTACK_DURATION;
        specialAttackCooldownTimer = SPECIAL_ATTACK_COOLDOWN;
        isSpecialAttacking = true;
        hasHitDuringDash = false;
        if (characterId == "Godon") {
            // Godon 衝刺
            vx = 700.0f * direction; // 衝刺速度
            attackTimer = 1.0f; // 延長攻擊判定時間，確保有足夠時間撞到敵人
        } else {
            vx = 0;
        }
        changeState(PlayerState::ATTACKING);
        return;
    }

    // --- 處理 LYING ---
    if (action == "LYING" && isOnGround &&
        state != PlayerState::ATTACKING && state != PlayerState::JUMPING &&
        state != PlayerState::FALLING && state != PlayerState::BLOCKING && 
        state != PlayerState::DEATH && state != PlayerState::HURT) {
        vx = 0;
        vy = 0;
        changeState(PlayerState::LYING);
        return;
    }

    // --- 處理 BLOCK ---
    if (action == "BLOCK" && isOnGround && blockCooldownTimer <= 0 &&
        state != PlayerState::ATTACKING && state != PlayerState::JUMPING && state != PlayerState::FALLING) {
        vx = 0;
        changeState(PlayerState::BLOCKING);
    }
    // --- 處理氣功發射 ---
    else if (action == "FIRE_PROJECTILE") {
        if (canFireProjectile()) {
            changeState(PlayerState::ATTACKING);
            vx = 0;
            attackTimer = ATTACK_DURATION;
            resetProjectileCooldown();
            shouldFireProjectile = true;
            std::string prefix = (characterId == "BlockMan") ? "blockman_fire" : "godon_fire";
            AudioManager::playRandomSound(prefix);
        }
    }
    // --- 處理其他動作 ---
    else if (state != PlayerState::BLOCKING) {
        if (action == "LEFT" && state != PlayerState::ATTACKING) {
            vx = -MOVE_SPEED;
            direction = -1;
            if (isOnGround) changeState(PlayerState::WALKING);
        }
        else if (action == "RIGHT" && state != PlayerState::ATTACKING) {
            vx = MOVE_SPEED;
            direction = 1;
            if (isOnGround) changeState(PlayerState::WALKING);
        }
        else if (action == "JUMP" && isOnGround && state != PlayerState::ATTACKING) {
            vy = -JUMP_STRENGTH;
            isOnGround = false;
            changeState(PlayerState::JUMPING);
            AudioManager::playRandomSound("jump");
            printf("Jump initiated - vy: %.2f, y: %.2f\n", vy, y); // 調試輸出
        }
        else if (action == "ATTACK" &&
            state != PlayerState::ATTACKING &&
            state != PlayerState::HURT &&
            state != PlayerState::BLOCKING &&
            attackCooldownTimer <= 0 &&
            attackRateCooldownTimer <= 0 &&
            !shouldFireProjectile) {
            attackTimer = ATTACK_DURATION;
            attackCooldownTimer = ATTACK_DURATION + getAttackCooldown();
            attackRateCooldownTimer = ATTACK_RATE_COOLDOWN;
            vx = 0;
            isSpecialAttacking = false;
            changeState(PlayerState::ATTACKING);
        }
        else if (action == "STOP_X" && state == PlayerState::WALKING) {
            vx = 0;
            if (isOnGround) changeState(PlayerState::IDLE);
        }
    }
}

void Player::update(float deltaTime) {
    // 處理 VICTORY 狀態
    if (state == PlayerState::VICTORY) {
        updateAnimation(deltaTime);
        vx = 0;
        vy = 0;
        if (!isOnGround) {
            y = GROUND_LEVEL - PLAYER_LOGIC_HEIGHT;
            isOnGround = true;
        }
        return;
    }

    // 處理死亡狀態
    if (state == PlayerState::DEATH) {
        updateAnimation(deltaTime);
        return;
    }

    // --- 更新計時器 ---
    if (attackTimer > 0) attackTimer -= deltaTime;
    if (attackCooldownTimer > 0) attackCooldownTimer -= deltaTime;
    if (hurtTimer > 0) hurtTimer -= deltaTime;
    if (invincibilityTimer > 0) invincibilityTimer -= deltaTime;
    if (blockCooldownTimer > 0) blockCooldownTimer -= deltaTime;
    if (attackRateCooldownTimer > 0) attackRateCooldownTimer -= deltaTime; 
    if (projectileCooldownTimer > 0) projectileCooldownTimer -= deltaTime;
    if (specialAttackCooldownTimer > 0) specialAttackCooldownTimer -= deltaTime;

    // --- 狀態自動轉換 ---
    if (state == PlayerState::ATTACKING && attackTimer <= 0) {
        changeState(PlayerState::IDLE);
        isSpecialAttacking = false;
    }
    if (state == PlayerState::HURT && hurtTimer <= 0) {
        changeState(PlayerState::IDLE);
    }

    // --- 物理更新 ---
    // 格擋狀態特殊處理
    if (state == PlayerState::BLOCKING) {
        vx = 0;
        vy = 0;
        y = GROUND_LEVEL - PLAYER_LOGIC_HEIGHT;
        isOnGround = true;
    } else {
        // 先更新垂直位置和速度
        if (!isOnGround) {
            vy += GRAVITY * deltaTime;
            y += vy * deltaTime;
            printf("Physics update - y: %.2f, vy: %.2f\n", y, vy); // 調試輸出
        }

        // 更新水平位置
        x += vx * deltaTime;

        // --- 地面檢測與處理 ---
        if (y + PLAYER_LOGIC_HEIGHT >= GROUND_LEVEL) {
            y = GROUND_LEVEL - PLAYER_LOGIC_HEIGHT;
            vy = 0;
            if (!isOnGround) {
                isOnGround = true;
                printf("Landed on ground\n"); // 調試輸出
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
        } else {
            isOnGround = false;
            if (state == PlayerState::IDLE || state == PlayerState::WALKING) {
                changeState(PlayerState::FALLING);
            }
        }
    }

    // --- 邊界檢測 ---
    if (x < 0) x = 0;
    if (x + PLAYER_LOGIC_WIDTH > SCREEN_WIDTH) x = SCREEN_WIDTH - PLAYER_LOGIC_WIDTH;

    // --- 更新動畫 ---
    updateAnimation(deltaTime);

    // 更新躺下狀態
    if (state == PlayerState::LYING) {
        if (hurtTimer <= 0 && !isOnGround) {
            changeState(PlayerState::IDLE);
        }
    }

    // Godon 衝刺技能：攻擊結束時歸零 vx
    if (characterId == "Godon" && isSpecialAttacking && state == PlayerState::ATTACKING) {
        // 衝刺期間
        // 若 attackTimer <= 0 或已經撞到人，結束衝刺
        if (attackTimer <= 0 || hasHitDuringDash) {
            vx = 0;
            isSpecialAttacking = false;
            if (hasHitDuringDash) {
                attackTimer = 0; // 如果撞到人了，立即結束攻擊狀態
            }
        }
    }
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
    SDL_Texture* texture = TextureManager::getTexture(textureId);
    if (!texture) {
        printf("Error: Texture '%s' not found for player '%s'\n", textureId.c_str(), characterId.c_str());
        SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
        SDL_Rect errorRect = getBoundingBox();
        SDL_RenderFillRect(renderer, &errorRect);
        return;
    }
    const AnimationInfo* animInfo = AnimationDataManager::getAnimationInfo(characterId, currentAnimationType);
    if (!animInfo || currentFrame >= animInfo->frameCount) return;
    SDL_Rect srcRect = animInfo->frames[currentFrame];

    SDL_Rect destRect;
    if (state == PlayerState::LYING) {
        int lyingW = logicWidth;
        int lyingH = static_cast<int>(logicHeight * 0.35f);
        int lyingX = (int)x;
        int lyingY = GROUND_LEVEL - lyingH;
        destRect = { lyingX, lyingY, lyingW, lyingH };
    } else {
        // 計算基準尺寸（使用 IDLE 動畫的第一幀作為基準）
        const AnimationInfo* idleInfo = AnimationDataManager::getAnimationInfo(characterId, AnimationType::IDLE);
        if (!idleInfo || idleInfo->frames.empty()) return;
        
        SDL_Rect baseFrame = idleInfo->frames[0];
        float baseScale = (float)PLAYER_LOGIC_HEIGHT / baseFrame.h;
        
        // 計算當前幀的縮放比例
        float currentScale = baseScale;
        
        // 特殊處理攻擊動畫
        if (state == PlayerState::ATTACKING) {
            float attackScale = (float)PLAYER_LOGIC_HEIGHT / srcRect.h;
            currentScale = attackScale;
        }
        
        // 計算渲染尺寸
        int scaledWidth = (int)(srcRect.w * currentScale);
        int scaledHeight = (int)(srcRect.h * currentScale);
        
        // 計算渲染位置
        int renderX = (int)x;
        int renderY = (int)y;
        
        if (state == PlayerState::BLOCKING || state == PlayerState::DEATH) {
            scaledWidth = (int)(srcRect.w * baseScale);
            scaledHeight = (int)(srcRect.h * baseScale);
            renderY = GROUND_LEVEL - scaledHeight;
        }
        
        destRect = { renderX, renderY, scaledWidth, scaledHeight };
    }
    
    // 處理角色的渲染
    SDL_RendererFlip flip = (direction == 1) ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
    bool drawPlayer = true;
    if (invincibilityTimer > 0 && fmod(invincibilityTimer, 0.2f) < 0.1f) {
        drawPlayer = false;
    }
    if (drawPlayer) {
        SDL_RenderCopyEx(renderer, texture, &srcRect, &destRect, 0.0, NULL, flip);
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

        return; // 阻擋傷害，直接返回
    }

    health -= damage;
    printf("Player %s took %d damage, health: %d\n", characterId.c_str(), damage, health);

    if (health <= 0) {
        health = 0;
        printf("Player %s defeated!\n", characterId.c_str());
        changeState(PlayerState::DEATH);
        std::string prefix = (characterId == "BlockMan") ? "blockman_death" : "godon_death";
        AudioManager::playRandomSound(prefix);
        // 確保玩家停止所有動作
        vx = 0;
        vy = 0;
        isOnGround = true;
        attackTimer = 0;
        hurtTimer = 0;
        invincibilityTimer = 0;
        blockCooldownTimer = 0;
        attackRateCooldownTimer = 0;
        projectileCooldownTimer = 0;
    } else {
        changeState(PlayerState::HURT);
        hurtTimer = HURT_DURATION;
        invincibilityTimer = HURT_INVINCIBILITY;
        vx = 0;
        vy = -100.0f;
        isOnGround = false;
        attackTimer = 0;
        std::string prefix = (characterId == "BlockMan") ? "blockman_hurt" : "godon_hurt";
        AudioManager::playRandomSound(prefix);
    }
}

SDL_Rect Player::getBoundingBox() const {
    if (state == PlayerState::LYING) {
        int lyingW = logicWidth;
        int lyingH = static_cast<int>(logicHeight * 0.35f); // 根據角色高度調整躺下時的高度
        int lyingX = (int)x;
        int lyingY = GROUND_LEVEL - lyingH;
        return { lyingX, lyingY, lyingW, lyingH };
    }
    return { (int)x, (int)y, logicWidth, logicHeight };
}

// 計算相對於自身的攻擊框 (Hitbox)
SDL_Rect Player::calculateRelativeHitbox() const {
    // 根據角色尺寸調整攻擊判定框
    int hitboxWidth = static_cast<int>(logicWidth * 0.5f);  // 攻擊判定寬度為角色寬度的一半
    int hitboxHeight = static_cast<int>(logicHeight * 0.25f);  // 攻擊判定高度為角色高度的四分之一
    int offsetX = (direction == 1) ? logicWidth - 20 : -hitboxWidth + 20;
    int offsetY = logicHeight / 2 - hitboxHeight / 2;
    return {offsetX, offsetY, hitboxWidth, hitboxHeight};
}


SDL_Rect Player::getHitboxWorld() const {
    if (state == PlayerState::ATTACKING && attackTimer > 0) {
        // 在攻擊動畫的前半段產生判定
        if (attackTimer > ATTACK_DURATION * 0.5f) {
            SDL_Rect relativeHitbox = calculateRelativeHitbox();
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

void Player::setGlove(GloveType gloveType) {
    currentGlove = gloveType;
    printf("Player %s switched to %s gloves\n", characterId.c_str(), getGloveName().c_str());
}

float Player::getAttackCooldown() const {
    switch (currentGlove) {
        case GloveType::LIGHT_10OZ:
            return LIGHT_GLOVE_COOLDOWN;
        case GloveType::MEDIUM_14OZ:
            return MEDIUM_GLOVE_COOLDOWN;
        case GloveType::HEAVY_18OZ:
            return HEAVY_GLOVE_COOLDOWN;
        default:
            return LIGHT_GLOVE_COOLDOWN;
    }
}

int Player::getAttackDamage() const {
    switch (currentGlove) {
        case GloveType::LIGHT_10OZ:
            return LIGHT_GLOVE_DAMAGE;
        case GloveType::MEDIUM_14OZ:
            return MEDIUM_GLOVE_DAMAGE;
        case GloveType::HEAVY_18OZ:
            return HEAVY_GLOVE_DAMAGE;
        default:
            return LIGHT_GLOVE_DAMAGE;
    }
}

std::string Player::getGloveName() const {
    switch (currentGlove) {
        case GloveType::LIGHT_10OZ:
            return "10oz";
        case GloveType::MEDIUM_14OZ:
            return "14oz";
        case GloveType::HEAVY_18OZ:
            return "18oz";
        default:
            return "Unknown";
    }
}

bool Player::canUseSpecialAttack() const {
    return specialAttackCooldownTimer <= 0;
}

void Player::resetSpecialAttackCooldown() {
    specialAttackCooldownTimer = SPECIAL_ATTACK_COOLDOWN;
    hasHitDuringDash = false;
}