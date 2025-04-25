#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static GameState game_state; // 靜態遊戲狀態實例
static const Glove glove_types[] = {
    {10, 1.0f}, // 拳套1: 攻擊力10，攻擊速度1次/秒
    {15, 0.8f}, // 拳套2: 攻擊力15，攻擊速度0.8次/秒
    {20, 0.6f}  // 拳套3: 攻擊力20，攻擊速度0.6次/秒
};

// 初始化遊戲
void init_game(GameState* state, int num_characters) {
    state->num_characters = num_characters;
    state->player1.character_id = 0; // 未選擇
    state->player1.health = 100;
    state->player1.glove.attack_power = 0;
    state->player1.glove.attack_speed = 0.0f;
    state->player2.character_id = 0; // 未選擇
    state->player2.health = 100;
    state->player2.glove.attack_power = 0;
    state->player2.glove.attack_speed = 0.0f;
    state->score = 0;
    state->game_time = 0;
    state->is_running = 1;
    state->winner = 0;
    state->phase = PHASE_CHARACTER_SELECT;
    printf("Game initialized: Character selection phase\n");

    // 初始化隨機數生成器
    srand((unsigned int)time(NULL));
}

// 選擇角色
void select_character(GameState* state, int player, int character_id) {
    if (state->phase != PHASE_CHARACTER_SELECT || character_id < 1 || character_id > state->num_characters) {
        printf("Invalid character selection for player %d\n", player);
        return;
    }

    if (player == 1) {
        state->player1.character_id = character_id;
        printf("Player 1 selected character %d\n", character_id);
    } else if (player == 2) {
        state->player2.character_id = character_id;
        printf("Player 2 selected character %d\n", character_id);
    }

    // 檢查是否兩人都選好角色
    if (state->player1.character_id != 0 && state->player2.character_id != 0) {
        state->phase = PHASE_GLOVE_SELECT;
        printf("Moving to glove selection phase\n");
    }
}

// 選擇拳套
void select_glove(GameState* state, int player, int glove_type) {
    if (state->phase != PHASE_GLOVE_SELECT || glove_type < 0 || glove_type >= 3) {
        printf("Invalid glove selection for player %d\n", player);
        return;
    }

    if (player == 1) {
        state->player1.glove = glove_types[glove_type];
        printf("Player 1 selected glove: Power=%d, Speed=%.1f\n", 
               state->player1.glove.attack_power, state->player1.glove.attack_speed);
    } else if (player == 2) {
        state->player2.glove = glove_types[glove_type];
        printf("Player 2 selected glove: Power=%d, Speed=%.1f\n", 
               state->player2.glove.attack_power, state->player2.glove.attack_speed);
    }

    // 檢查是否兩人都選好拳套
    if (state->player1.glove.attack_power != 0 && state->player2.glove.attack_power != 0) {
        state->phase = PHASE_BATTLE;
        printf("Starting battle phase\n");
    }
}

// 開始對戰
void start_battle(GameState* state) {
    if (state->phase == PHASE_BATTLE) {
        printf("Battle started!\n");
    }
}

// 更新遊戲邏輯
void update_game(GameState* state, Uint32 delta_time) {
    if (!state->is_running || state->phase != PHASE_BATTLE) return;

    // 更新遊戲時間
    state->game_time += delta_time;

    // 檢查勝負條件
    if (state->player1.health <= 0) {
        state->winner = 2; // 玩家2勝利
        state->is_running = 0;
        state->phase = PHASE_GAME_OVER;
        printf("Player 2 wins!\n");
    } else if (state->player2.health <= 0) {
        state->winner = 1; // 玩家1勝利
        state->is_running = 0;
        state->phase = PHASE_GAME_OVER;
        printf("Player 1 wins!\n");
    }

    // 模擬得分增加
    state->score += delta_time / 1000; // 每秒加1分
}

// 結束遊戲
void end_game(GameState* state) {
    state->is_running = 0;
    state->phase = PHASE_GAME_OVER;
    printf("Game ended. Final score: %d\n", state->score);
}

// 獲取遊戲狀態
GameState* get_game_state(void) {
    return &game_state;
}

// 初始化音效
void init_sound(SoundManager* sound, int num_characters) {
    // 初始化 SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer could not initialize! Error: %s\n", Mix_GetError());
        return;
    }

    sound->num_characters = num_characters;
    sound->attack_sounds = (Mix_Chunk**)malloc(num_characters * sizeof(Mix_Chunk*));
    sound->num_attack_sounds = (int*)malloc(num_characters * sizeof(int));

    // 為每個角色載入 2~3 個攻擊音效
    for (int i = 0; i < num_characters; i++) {
        sound->num_attack_sounds[i] = (rand() % 2) + 2; // 2 或 3
        sound->attack_sounds[i] = (Mix_Chunk*)malloc(sound->num_attack_sounds[i] * sizeof(Mix_Chunk));

        for (int j = 0; j < sound->num_attack_sounds[i]; j++) {
            char filename[32];
            snprintf(filename, sizeof(filename), "attack_%d_%d.wav", i + 1, j + 1);
            sound->attack_sounds[i][j] = Mix_LoadWAV(filename);
            if (!sound->attack_sounds[i][j]) {
                printf("Failed to load %s! Error: %s\n", filename, Mix_GetError());
            }
        }
    }

    // 載入背景音樂
    sound->background_music = Mix_LoadMUS("background.mp3");
    if (!sound->background_music) {
        printf("Failed to load background music! Error: %s\n", Mix_GetError());
    }
}

// 播放隨機攻擊音效
void play_attack_sound(SoundManager* sound, int character_id) {
    if (character_id < 1 || character_id > sound->num_characters) return;

    int index = character_id - 1; // 轉換為 0-based 索引
    if (sound->num_attack_sounds[index] > 0) {
        int sound_index = rand() % sound->num_attack_sounds[index];
        if (sound->attack_sounds[index][sound_index]) {
            Mix_PlayChannel(-1, sound->attack_sounds[index][sound_index], 0);
        }
    }
}

// 播放背景音樂
void play_background_music(SoundManager* sound) {
    if (sound->background_music && !Mix_PlayingMusic()) {
        Mix_PlayMusic(sound->background_music, -1); // 無限循環
    }
}

// 停止背景音樂
void stop_background_music(SoundManager* sound) {
    if (Mix_PlayingMusic()) {
        Mix_HaltMusic();
    }
}

// 清理音效資源
void cleanup_sound(SoundManager* sound) {
    for (int i = 0; i < sound->num_characters; i++) {
        for (int j = 0; j < sound->num_attack_sounds[i]; j++) {
            if (sound->attack_sounds[i][j]) {
                Mix_FreeChunk(sound->attack_sounds[i][j]);
                sound->attack_sounds[i][j] = NULL;
            }
        }
        free(sound->attack_sounds[i]);
    }
    free(sound->attack_sounds);
    free(sound->num_attack_sounds);

    if (sound->background_music) {
        Mix_FreeMusic(sound->background_music);
        sound->background_music = NULL;
    }

    Mix_CloseAudio();
}