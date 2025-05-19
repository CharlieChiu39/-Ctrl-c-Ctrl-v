#include "AnimationData.h"
#include <stdio.h> // for printf

// 初始化靜態成員變數
std::map<std::string, std::map<AnimationType, AnimationInfo>> AnimationDataManager::characterAnimations;

void AnimationDataManager::defineAnimation(const std::string& characterId, AnimationType type,
                                         const std::vector<SDL_Rect>& frameRects,
                                         float duration, bool shouldLoop) {
    if (frameRects.empty()) {
        printf("Warning: Trying to define animation for '%s' type %d with empty frames.\n",
               characterId.c_str(), static_cast<int>(type));
        return;
    }

    AnimationInfo info;
    info.frames = frameRects;
    info.frameCount = frameRects.size();
    info.frameDuration = duration;
    info.loop = shouldLoop;

    // 存入 Map
    characterAnimations[characterId][type] = info;

    // printf("Defined animation for %s, type %d with %d frames.\n",
    //        characterId.c_str(), static_cast<int>(type), info.frameCount);
}

const AnimationInfo* AnimationDataManager::getAnimationInfo(const std::string& characterId, AnimationType type) {
    // 找角色
    auto charIt = characterAnimations.find(characterId);
    if (charIt != characterAnimations.end()) {
        // 找動畫類型
        auto animIt = charIt->second.find(type);
        if (animIt != charIt->second.end()) {
            return &animIt->second; // 回傳 AnimationInfo 的指標
        }
    }
    printf("Warning: Animation info not found for character '%s', type %d.\n",
           characterId.c_str(), static_cast<int>(type));
    return nullptr; // 找不到返回 nullptr
}

// 初始化 BlockMan 的動畫數據
void AnimationDataManager::initializeBlockManAnimations() {
    const std::string charId = "BlockMan"; // 定義此角色的 ID

    // --- 從 main (2).cpp 來的幀定義 ---
    // **注意:** 這些尺寸需要根據你的實際 Sprite Sheet 調整
    // Sprite Sheet 上的幀尺寸 (可能因動畫不同)
    const int IDLE_W = 68, IDLE_H = 116;
    const int WALK_W = 73, WALK_H = 116;
    const int ATTACK_W = 76, ATTACK_H = 105; // Attack 幀尺寸不同
    const int HURT_W = 75, HURT_H = 116;
    const int JUMP_W = 68, JUMP_H = 115; // Jump 幀尺寸不同
    const int FALL_W = 65, FALL_H = 115; // Fall 幀尺寸不同
    const int BLOCK_W = 66, BLOCK_H = 115;
    const int DEATH_W = 83, DEATH_H = 103;     


    // Idle 動畫 (4 幀, 在 Y=0 開始)
    std::vector<SDL_Rect> idleFrames;
    for (int i = 0; i < 4; ++i) idleFrames.push_back({i * IDLE_W, 0, IDLE_W, IDLE_H});
    defineAnimation(charId, AnimationType::IDLE, idleFrames, 0.1f, true);

    // Walk 動畫 (5 幀, 在 Y=IDLE_H 開始)
    std::vector<SDL_Rect> walkFrames;
    for (int i = 0; i < 5; ++i) walkFrames.push_back({i * WALK_W, IDLE_H, WALK_W, WALK_H});
    defineAnimation(charId, AnimationType::WALK, walkFrames, 0.1f, true);

    // Attack 動畫 (2 幀, 在 Y=355 開始, 硬編碼)
    std::vector<SDL_Rect> attackFrames = {{190, 355, ATTACK_W, ATTACK_H}, {270, 355, ATTACK_W, ATTACK_H}};
    defineAnimation(charId, AnimationType::ATTACK, attackFrames, 0.15f, false); // 攻擊動畫通常不循環，時間可調

    // Hurt 動畫 (3 幀, 在 Y=2145 開始, 硬編碼)
    std::vector<SDL_Rect> hurtFrames;
    for (int i = 0; i < 3; ++i) hurtFrames.push_back({160 + i * HURT_W, 2145, HURT_W, HURT_H});
    defineAnimation(charId, AnimationType::HURT, hurtFrames, 0.13f, false); // 受傷動畫通常不循環

    // Jump 動畫 (1 幀, 硬編碼)
    std::vector<SDL_Rect> jumpFrames = {{62, 240, JUMP_W, JUMP_H}};
    defineAnimation(charId, AnimationType::JUMP, jumpFrames, 0.1f, false);

    // Fall 動畫 (1 幀, 硬編碼)
    std::vector<SDL_Rect> fallFrames = {{372, 243, FALL_W, FALL_H}};
    defineAnimation(charId, AnimationType::FALL, fallFrames, 0.1f, false);

    // Block 動畫
    std::vector<SDL_Rect> blockFrames = {{455, 1045, BLOCK_W, BLOCK_H}}; // <--- 請換成實際格擋幀的座標和尺寸
    defineAnimation(charId, AnimationType::BLOCK, blockFrames, 0.1f, false); // 格擋通常不循環播放

    // DEATH 動畫
    std::vector<SDL_Rect> deathFrames = {{150, 2260, DEATH_W, DEATH_H}}; // <--- 請換成實際格擋幀的座標和尺寸
    defineAnimation(charId, AnimationType::DEATH, deathFrames, 0.1f, false); // 死亡通常不循環播放

     // --- 新增 VICTORY 動畫 ---
    // vvvvv 請務必換成你實際的勝利動畫幀數據 vvvvv
    const int VICTORY_W = 80; // 假設寬度
    const int VICTORY_H = 120; // 假設高度
    const int VICTORY_START_Y = 2500; // 假設 Y 座標
    std::vector<SDL_Rect> victoryFrames;
    // 假設有 5 幀勝利動畫
    for (int i = 0; i < 5; ++i) {
        victoryFrames.push_back({i * VICTORY_W, VICTORY_START_Y, VICTORY_W, VICTORY_H});
    }
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    // 定義勝利動畫，假設每幀 0.12 秒，不循環
    defineAnimation(charId, AnimationType::VICTORY, victoryFrames, 0.12f, false);


    printf("Initialized animations for character: %s\n", charId.c_str());
}