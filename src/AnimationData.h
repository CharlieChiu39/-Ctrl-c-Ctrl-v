#ifndef ANIMATIONDATA_H
#define ANIMATIONDATA_H

#include <SDL2/SDL.h>
#include <vector>
#include <string>
#include <map>

// --- 角色動畫類型 ---
enum class AnimationType {
    IDLE, WALK, JUMP, FALL, ATTACK, HURT, BLOCK, DEATH, VICTORY
    // 可以根據需要增加更多類型
};

// --- 單一動畫的資料 ---
struct AnimationInfo {
    std::vector<SDL_Rect> frames;        // 該動畫的所有幀 (來源矩形)
    int frameCount = 0;                 // 幀數 (frames.size())
    float frameDuration = 0.1f;         // 每幀持續時間 (可覆寫預設值)
    bool loop = true;                   // 是否循環播放
    // 可以加入其他屬性，例如特定動畫的音效 ID 等
};

// --- 角色動畫數據管理器 ---
class AnimationDataManager {
public:
    // 為特定角色載入/定義動畫數據
    // characterId: 角色識別字 (例如 "BlockMan", "Ninja")
    // animationType: 動畫類型 (IDLE, WALK 等)
    // frameRects: 包含該動畫所有幀的 SDL_Rect 向量
    // duration: (可選) 此動畫的特定幀持續時間
    // shouldLoop: (可選) 此動畫是否循環
    static void defineAnimation(const std::string& characterId, AnimationType type,
                                const std::vector<SDL_Rect>& frameRects,
                                float duration = 0.1f, bool shouldLoop = true);

    // 取得特定角色、特定動畫類型的資料
    static const AnimationInfo* getAnimationInfo(const std::string& characterId, AnimationType type);

    // (建議) 從設定檔載入所有角色動畫 (未來擴充)
    // static bool loadAnimationsFromFile(const std::string& filePath);

    // 初始化內建角色 ("BlockMan") 的動畫數據
    // 這是將原本 initializeAnimationRects 的邏輯封裝起來
    static void initializeBlockManAnimations();

private:
    // 使用巢狀 Map 來儲存: characterId -> AnimationType -> AnimationInfo
    static std::map<std::string, std::map<AnimationType, AnimationInfo>> characterAnimations;
};


#endif // ANIMATIONDATA_H