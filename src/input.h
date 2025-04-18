#ifndef INPUT_H
#define INPUT_H

#include "player.h"
#include "game.h"

/**
 * @brief 輸入系統狀態枚舉
 * 
 * 定義遊戲不同階段的輸入處理狀態，每個狀態有不同的輸入處理邏輯
 */
typedef enum {
    INPUT_STATE_MAIN_MENU,        ///< 主選單狀態：處理開始遊戲或退出
    INPUT_STATE_CHARACTER_SELECT, ///< 角色選擇狀態：處理玩家角色選擇
    INPUT_STATE_GLOVE_SELECT,     ///< 拳套選擇狀態：處理玩家拳套選擇
    INPUT_STATE_GAMEPLAY,         ///< 遊戲進行狀態：處理遊戲中玩家控制
    INPUT_STATE_GAME_OVER         ///< 遊戲結束狀態：處理遊戲結束後選項
} InputState;

/**
 * @brief 初始化輸入系統
 * 
 * 必須在遊戲啟動時呼叫，初始化SDL鍵盤狀態追蹤
 */
void init_input();

/**
 * @brief 處理所有輸入事件
 * 
 * 每幀呼叫一次，根據當前狀態處理不同的輸入邏輯
 * @param player1 玩家1物件指針
 * @param player2 玩家2物件指針
 * @param game_state 遊戲狀態指針
 */
void process_input(Player* player1, Player* player2, GameState* game_state);

/**
 * @brief 清理輸入系統資源
 * 
 * 在遊戲結束時呼叫，釋放輸入系統相關資源
 */
void cleanup_input();

#endif // INPUT_H