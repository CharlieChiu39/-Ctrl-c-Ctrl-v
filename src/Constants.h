#ifndef CONSTANTS_H
#define CONSTANTS_H

// --- 螢幕與視窗 ---
const int SCREEN_WIDTH = 1024;
const int SCREEN_HEIGHT = 576;

// --- 遊戲世界 ---
const int GROUND_LEVEL = SCREEN_HEIGHT - 80;

// --- 物理常數 ---
const float MOVE_SPEED = 300.0f;
const float JUMP_STRENGTH = 800.0f;   // 進一步降低跳躍力量
const float GRAVITY = 2000.0f;        // 相應調整重力

// --- 遊戲規則/數值 ---
const int ATTACK_DAMAGE = 15;
const float PLAYER_DEFAULT_HEALTH = 100.0f;
const float ROUND_TIME_LIMIT = 99.0f; // 回合時間限制 (秒)
const float ROUND_DURATION = ROUND_TIME_LIMIT; // 回合持續時間
const int   ROUNDS_TO_WIN_MATCH = 2;     // 贏得比賽所需的回合勝利數
const float ROUND_OVER_DELAY = 3.0f;       // 回合結束後顯示結果的延遲時間 (秒)

// --- 時間相關 ---
const float ATTACK_DURATION = 0.3f;
const float ATTACK_COOLDOWN = 0.1f;
const float HURT_DURATION = 0.4f;
const float HURT_INVINCIBILITY = 0.6f;
const float ANIMATION_FRAME_DURATION = 0.1f; // 預設動畫幀持續時間
const float ATTACK_HITBOX_ACTIVE_START = 0.1f; // Hitbox 在攻擊動畫中的生效開始時間 (從攻擊開始算)
const float ATTACK_HITBOX_ACTIVE_END = 0.3f;   // Hitbox 在攻擊動畫中的生效結束時間 (從攻擊開始算)
const float BLOCK_COOLDOWN = 3.0f; // 格擋冷卻時間 (3秒)
const float ATTACK_RATE_COOLDOWN = 1.0f; // 攻擊速率冷卻時間 (1秒)

// --- 氣功 (Projectile) 常數 ---
const float PROJECTILE_SPEED = 600.0f;           // 氣功飛行速度 (像素/秒)
const int   PROJECTILE_DAMAGE = 25;              // 氣功傷害值
const float PROJECTILE_COOLDOWN = 5.0f;          // 氣功發射冷卻時間 (秒)
const float SPECIAL_ATTACK_COOLDOWN = 10.0f;     // 特殊技能冷卻時間 (秒)

// 精靈圖相關 (非常重要，請務必修改!)
const int   PROJECTILE_SRC_X = 630;                // <--- 氣功在精靈圖上的 X 起始座標
const int   PROJECTILE_SRC_Y = 1437;                // <--- 氣功在精靈圖上的 Y 起始座標
const int   PROJECTILE_SRC_W = 64;               // <--- 氣功在精靈圖上的寬度
const int   PROJECTILE_SRC_H = 45;               // <--- 氣功在精靈圖上的高度
// 如果氣功有多幀動畫，這裡需要更複雜的定義，我們先用單幀

// 遊戲世界中顯示的大小
const int   PROJECTILE_HITBOX_W = 64;            // <--- 氣功在遊戲中的碰撞寬度 (可以跟 SRC_W 一樣)
const int   PROJECTILE_HITBOX_H = 45;            // <--- 氣功在遊戲中的碰撞高度 (可以跟 SRC_H 一樣)

// --- 玩家邏輯/碰撞尺寸 (每個角色可能不同，可移至角色配置檔) ---
// BlockMan 的尺寸
const int BLOCKMAN_LOGIC_WIDTH = 120;
const int BLOCKMAN_LOGIC_HEIGHT = 205;

// Godon 的尺寸
const int GODON_LOGIC_WIDTH = 160;  // 比 BlockMan 寬 40 像素
const int GODON_LOGIC_HEIGHT = 245; // 比 BlockMan 高 40 像素

// 為了向後兼容，保留原有的常數名稱
const int PLAYER_LOGIC_WIDTH = BLOCKMAN_LOGIC_WIDTH;
const int PLAYER_LOGIC_HEIGHT = BLOCKMAN_LOGIC_HEIGHT;

#endif // CONSTANTS_H