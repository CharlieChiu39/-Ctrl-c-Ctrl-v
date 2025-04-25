#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

// 遊戲階段枚舉
typedef enum {
    PHASE_CHARACTER_SELECT, // 角色選擇
    PHASE_GLOVE_SELECT,     // 拳套選擇
    PHASE_BATTLE,           // 對戰
    PHASE_GAME_OVER         // 遊戲結束
} GamePhase;

// 拳套類型結構
typedef struct {
    int attack_power;     // 攻擊力
    float attack_speed;   // 攻擊速度（每秒攻擊次數）
} Glove;

// 角色數據結構
typedef struct {
    int character_id;     // 角色ID
    int health;           // 血量
    Glove glove;          // 所選拳套
} Character;

// 遊戲狀態結構
typedef struct {
    Character player1;     // 玩家1
    Character player2;     // 玩家2
    int score;             // 遊戲得分
    Uint32 game_time;      // 遊戲時間（毫秒）
    int is_running;        // 遊戲是否運行
    int winner;            // 勝利者（0:無，1:玩家1，2:玩家2）
    GamePhase phase;       // 當前遊戲階段
    int num_characters;    // 可用角色數量
} GameState;

// 音效管理結構
typedef struct {
    Mix_Chunk** attack_sounds; // 每個角色的攻擊音效陣列（二維）
    int* num_attack_sounds;    // 每個角色的攻擊音效數量
    int num_characters;        // 角色數量
    Mix_Music* background_music; // 背景音樂
} SoundManager;

// 遊戲函數宣告
void init_game(GameState* state, int num_characters);
void update_game(GameState* state, Uint32 delta_time);
void end_game(GameState* state);
GameState* get_game_state(void);
void select_character(GameState* state, int player, int character_id);
void select_glove(GameState* state, int player, int glove_type);
void start_battle(GameState* state);

// 音效函數宣告
void init_sound(SoundManager* sound, int num_characters);
void play_attack_sound(SoundManager* sound, int character_id);
void play_background_music(SoundManager* sound);
void stop_background_music(SoundManager* sound);
void cleanup_sound(SoundManager* sound);

#endif // GAME_H