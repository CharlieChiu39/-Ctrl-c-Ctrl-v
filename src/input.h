#ifndef INPUT_H  // 如果 INPUT_H 尚未定義
#define INPUT_H  // 定義 INPUT_H，防止重複包含

#include "player.h"  // 包含玩家相關的宣告和定義
#include "game.h"  // 包含遊戲狀態相關的宣告和定義
#include <SDL2/SDL.h>  // 包含 SDL 庫的相關定義

// 定義輸入系統的狀態枚舉
typedef enum {
    INPUT_STATE_MAIN_MENU,        ///< 主選單狀態：處理開始遊戲或退出
    INPUT_STATE_CHARACTER_SELECT, ///< 角色選擇狀態：處理玩家角色選擇
    INPUT_STATE_GLOVE_SELECT,     ///< 拳套選擇狀態：處理玩家拳套選擇
    INPUT_STATE_GAMEPLAY,         ///< 遊戲進行狀態：處理遊戲中玩家控制
    INPUT_STATE_GAME_OVER         ///< 遊戲結束狀態：處理遊戲結束後選項
} InputState;

// 定義鍵盤映射結構
typedef struct {
    SDL_Scancode left;     ///< 左移動鍵
    SDL_Scancode right;    ///< 右移動鍵
    SDL_Scancode up;       ///< 上移動鍵
    SDL_Scancode down;     ///< 下移動鍵
    SDL_Scancode jump;     ///< 跳躍鍵
    SDL_Scancode attack;   ///< 攻擊鍵
    SDL_Scancode crouch;   ///< 蹲下鍵
    SDL_Scancode block;    ///< 防禦鍵
    SDL_Scancode skill;    ///< 技能鍵
} KeyMapping;

// 外部變數宣告，用於在 input.c 中定義和初始化
extern KeyMapping player1_keys;  // 玩家 1 的鍵盤映射
extern KeyMapping player2_keys;  // 玩家 2 的鍵盤映射

// 函數原型宣告
void init_input();                  ///< 初始化輸入系統
void process_input(Player* player1, Player* player2, GameState* game_state); ///< 處理所有輸入事件
void cleanup_input();               ///< 清理輸入系統資源

#endif // INPUT_H  // 結束 INPUT_H 的定義
