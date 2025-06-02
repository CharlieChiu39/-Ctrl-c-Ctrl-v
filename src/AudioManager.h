#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <SDL2/SDL_mixer.h>
#include <string>
#include <map>
#include <vector> // 為了儲存 sound IDs
#include <random> // 為了隨機數生成

class AudioManager {
public:
    // 初始化 SDL_mixer
    static bool init(int frequency = 44100, Uint16 format = MIX_DEFAULT_FORMAT, int channels = 2, int chunksize = 2048);

    // 載入背景音樂
    static bool loadMusic(const std::string& id, const std::string& filePath);

    // 載入音效
    static bool loadSound(const std::string& id, const std::string& filePath);

    // 播放背景音樂 (loops = -1 表示無限循環)
    static void playMusic(const std::string& id, int loops = -1);

    // 停止背景音樂
    static void stopMusic();

    // 暫停背景音樂
    static void pauseMusic();

    // 恢復背景音樂
    static void resumeMusic();

    // 播放音效 (回傳播放的 channel，-1 表示失敗)
    static int playSound(const std::string& id, int loops = 0); // 預設不循環

    // 隨機播放指定類型的音效 (例如 "hurt", "fire", "death")
    static int playRandomSound(const std::string& type, int loops = 0);

    // 停止特定 channel 的音效
    static void stopChannel(int channel);

    // 停止所有音效
    static void stopAllSounds();

    // 設定音效音量 (0-128)
    static void setSoundVolume(const std::string& id, int volume);

    // 設定 BGM 音量 (0-128)
    static void setMusicVolume(int volume);

    // 清理資源
    static void cleanup();

private:
    // 禁止實例化
    AudioManager() {}

    static std::map<std::string, Mix_Music*> musicMap;
    static std::map<std::string, Mix_Chunk*> soundMap;
    static std::vector<std::string> soundIds; // 方便設定音量
    static bool isInitialized;
    static std::random_device rd; // 用於生成隨機數
    static std::mt19937 gen; // Mersenne Twister 隨機數生成器
};

#endif // AUDIOMANAGER_H