#include "AudioManager.h"
#include <stdio.h> // for printf
#include <SDL2/SDL_mixer.h> // <--- 為了 Mix_Music* 和 Mix_Chunk*
#include <map>              // <--- 為了 std::map
#include <string>           // <--- 為了 std::string
#include <vector>           // <--- 為了 std::vector (給 soundIds 用的)
#include <random>           // <--- 為了 std::random_device and std::mt19937

// 初始化靜態成員
std::map<std::string, Mix_Music*> AudioManager::musicMap;
std::map<std::string, Mix_Chunk*> AudioManager::soundMap;
std::vector<std::string> AudioManager::soundIds;
bool AudioManager::isInitialized = false;
std::random_device AudioManager::rd;
std::mt19937 AudioManager::gen(AudioManager::rd());

bool AudioManager::init(int frequency, Uint16 format, int channels, int chunksize) {
    if (isInitialized) {
        printf("AudioManager already initialized.\n");
        return true;
    }

    // 注意：SDL_Init(SDL_INIT_AUDIO) 應該在 Game::initialize 中完成
    // 初始化 SDL_mixer
    if (Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG) != (MIX_INIT_MP3 | MIX_INIT_OGG)) {
         printf( "SDL_mixer could not initialize required decoders! Mix_Error: %s\n", Mix_GetError() );
         // 即使解碼器沒完全初始化，還是嘗試打開音訊裝置
    }


    if (Mix_OpenAudio(frequency, format, channels, chunksize) < 0) {
        printf("SDL_mixer could not open audio! Mix_Error: %s\n", Mix_GetError());
        Mix_Quit(); // 如果連裝置都打不開就退出
        return false;
    }

    // 查詢實際開啟的規格 (可選)
    int actual_frequency, actual_channels;
    Uint16 actual_format;
    Mix_QuerySpec(&actual_frequency, &actual_format, &actual_channels);
    printf("SDL_mixer initialized: Freq=%d, Format=%d, Channels=%d\n", actual_frequency, actual_format, actual_channels);


    isInitialized = true;
    return true;
}

bool AudioManager::loadMusic(const std::string& id, const std::string& filePath) {
    if (!isInitialized) { printf("Error: AudioManager not initialized.\n"); return false; }
    if (musicMap.count(id)) {
        printf("Warning: Music with ID '%s' already loaded.\n", id.c_str());
        return true;
    }
    Mix_Music* music = Mix_LoadMUS(filePath.c_str());
    if (music == nullptr) {
        printf("Failed to load music '%s'! Mix_Error: %s\n", filePath.c_str(), Mix_GetError());
        return false;
    }
    musicMap[id] = music;
    printf("Loaded music '%s' with ID '%s'\n", filePath.c_str(), id.c_str());
    return true;
}

bool AudioManager::loadSound(const std::string& id, const std::string& filePath) {
    if (!isInitialized) { printf("Error: AudioManager not initialized.\n"); return false; }
     if (soundMap.count(id)) {
        printf("Warning: Sound with ID '%s' already loaded.\n", id.c_str());
        return true;
    }
    Mix_Chunk* chunk = Mix_LoadWAV(filePath.c_str()); // Mix_LoadWAV 通常支援多種格式
    if (chunk == nullptr) {
        printf("Failed to load sound '%s'! Mix_Error: %s\n", filePath.c_str(), Mix_GetError());
        return false;
    }
    soundMap[id] = chunk;
    soundIds.push_back(id); // 記錄 ID
    printf("Loaded sound '%s' with ID '%s'\n", filePath.c_str(), id.c_str());
    return true;
}

void AudioManager::playMusic(const std::string& id, int loops) {
    if (!isInitialized) return;
    if (musicMap.count(id)) {
        if (Mix_PlayMusic(musicMap[id], loops) == -1) {
            printf("Failed to play music '%s'! Mix_Error: %s\n", id.c_str(), Mix_GetError());
        }
    } else {
        printf("Error: Music with ID '%s' not found.\n", id.c_str());
    }
}

void AudioManager::stopMusic() {
    if (!isInitialized) return;
    Mix_HaltMusic(); // 停止 BGM
}

void AudioManager::pauseMusic() {
     if (!isInitialized) return;
     Mix_PauseMusic();
}

void AudioManager::resumeMusic() {
    if (!isInitialized) return;
    Mix_ResumeMusic();
}


int AudioManager::playSound(const std::string& id, int loops) {
    if (!isInitialized) return -1;
    if (soundMap.count(id)) {
        // 播放音效在第一個可用的 channel (-1) 上，重複 loops 次 (0 表示播放一次)
        int channel = Mix_PlayChannel(-1, soundMap[id], loops);
        if (channel == -1) {
             printf("Failed to play sound '%s'! (Possibly no free channels) Mix_Error: %s\n", id.c_str(), Mix_GetError());
        }
        return channel;
    } else {
        printf("Error: Sound with ID '%s' not found.\n", id.c_str());
        return -1;
    }
}

void AudioManager::stopChannel(int channel) {
    if (!isInitialized) return;
    Mix_HaltChannel(channel);
}

void AudioManager::stopAllSounds() {
    if (!isInitialized) return;
    Mix_HaltChannel(-1); // 停止所有 channel 的音效
}


void AudioManager::setSoundVolume(const std::string& id, int volume) {
     if (!isInitialized) return;
     if (soundMap.count(id)) {
         Mix_VolumeChunk(soundMap[id], volume); // 設定特定音效的音量
     } else {
          printf("Error: Sound with ID '%s' not found for volume setting.\n", id.c_str());
     }
}

void AudioManager::setMusicVolume(int volume) {
    if (!isInitialized) return;
    Mix_VolumeMusic(volume); // 設定 BGM 的音量
}


void AudioManager::cleanup() {
    if (!isInitialized) return;
    printf("Cleaning up AudioManager...\n");
    stopMusic();
    stopAllSounds();

    for (auto const& [id, music] : musicMap) {
        if (music) Mix_FreeMusic(music);
        // printf("  - Freed music ID: %s\n", id.c_str());
    }
    musicMap.clear();

    for (auto const& [id, sound] : soundMap) {
        if (sound) Mix_FreeChunk(sound);
         // printf("  - Freed sound ID: %s\n", id.c_str());
    }
    soundMap.clear();
    soundIds.clear();

    // 關閉所有開啟的音訊裝置
    while(Mix_QuerySpec(nullptr, nullptr, nullptr)) { // 當還有裝置開啟時
        Mix_CloseAudio();
    }

    Mix_Quit();       // 退出 SDL_mixer 子系統
    isInitialized = false;
    printf("AudioManager cleanup complete.\n");
}

int AudioManager::playRandomSound(const std::string& type, int loops) {
    if (!isInitialized) return -1;

    // 收集所有以指定類型開頭的音效ID
    std::vector<std::string> matchingSounds;
    for (const auto& [id, _] : soundMap) {
        if (id.find(type) == 0) { // 檢查ID是否以指定類型開頭
            matchingSounds.push_back(id);
        }
    }

    if (matchingSounds.empty()) {
        printf("Error: No sounds found for type '%s'\n", type.c_str());
        return -1;
    }

    // 隨機選擇一個音效
    std::uniform_int_distribution<> dis(0, matchingSounds.size() - 1);
    std::string selectedSound = matchingSounds[dis(gen)];

    // 播放選中的音效
    return playSound(selectedSound, loops);
}