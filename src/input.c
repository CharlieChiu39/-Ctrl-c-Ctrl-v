#include "input.h"  // 包含輸入處理的標頭檔
#include <SDL2/SDL.h>  // 包含 SDL 庫的相關定義
#include <stdbool.h>  // 包含布林型別的定義
#include <stdio.h>  // 包含標準輸入輸出函數

// 定義和初始化玩家 1 的鍵盤映射結構
KeyMapping player1_keys = {
    .left = SDL_SCANCODE_A,        // 玩家 1 左移動鍵
    .right = SDL_SCANCODE_D,       // 玩家 1 右移動鍵
    .up = SDL_SCANCODE_W,           // 玩家 1 上移動鍵
    .down = SDL_SCANCODE_S,         // 玩家 1 下移動鍵
    .jump = SDL_SCANCODE_SPACE,     // 玩家 1 跳躍鍵
    .attack = SDL_SCANCODE_J,       // 玩家 1 攻擊鍵
    .crouch = SDL_SCANCODE_C,       // 玩家 1 蹲下鍵
    .block = SDL_SCANCODE_K,        // 玩家 1 防禦鍵
    .skill = SDL_SCANCODE_L        // 玩家 1 技能鍵
};

// 定義和初始化玩家 2 的鍵盤映射結構
KeyMapping player2_keys = {
    .left = SDL_SCANCODE_LEFT,     // 玩家 2 左移動鍵
    .right = SDL_SCANCODE_RIGHT,    // 玩家 2 右移動鍵
    .up = SDL_SCANCODE_UP,          // 玩家 2 上移動鍵
    .down = SDL_SCANCODE_DOWN,      // 玩家 2 下移動鍵
    .jump = SDL_SCANCODE_RCTRL,    // 玩家 2 跳躍鍵
    .attack = SDL_SCANCODE_1,       // 玩家 2 攻擊鍵
    .crouch = SDL_SCANCODE_RSHIFT,  // 玩家 2 蹲下鍵
    .block = SDL_SCANCODE_2,        // 玩家 2 防禦鍵
    .skill = SDL_SCANCODE_3         // 玩家 2 技能鍵
};

// 當前輸入處理狀態
static InputState current_input_state = INPUT_STATE_MAIN_MENU;  // 初始狀態為主選單
// SDL 鍵盤狀態陣列指針
static const Uint8* current_key_states = NULL;  // 初始為 NULL
// 本幀按鍵按下標記陣列
static bool key_pressed_this_frame[SDL_NUM_SCANCODES] = {false};  // 初始所有按鍵標記為 false
// 選擇狀態追蹤變數
static bool character_selected_p1 = false;  // 玩家 1 是否已確認角色選擇
static bool character_selected_p2 = false;  // 玩家 2 是否已確認角色選擇
static bool glove_selected_p1 = false;    // 玩家 1 是否已確認拳套選擇
static bool glove_selected_p2 = false;    // 玩家 2 是否已確認拳套選擇

/**
 * @brief 初始化輸入系統
 *
 * 獲取 SDL 鍵盤狀態陣列指針，必須在 SDL 初始化後呼叫
 */
void init_input() {
    current_key_states = SDL_GetKeyboardState(NULL);  // 獲取當前鍵盤狀態陣列指針
}

/**
 * @brief 清理輸入系統資源
 *
 * 目前無需特殊清理，保留函數以備未來擴展
 */
void cleanup_input() {
    // 目前沒有需要清理的資源
}

/**
 * @brief 處理所有輸入事件
 *
 * 主輸入處理函數，每幀呼叫一次，根據當前狀態處理不同輸入邏輯
 * @param player1 玩家 1 物件指針
 * @param player2 玩家 2 物件指針
 * @param game_state 遊戲狀態指針
 */
void process_input(Player* player1, Player* player2, GameState* game_state) {
    SDL_Event event;  // SDL 事件結構體

    // 重置本幀按鍵按下標記
    for (int i = 0; i < SDL_NUM_SCANCODES; i++) {
        key_pressed_this_frame[i] = false;  // 將所有按鍵標記設為 false
    }

    // 處理所有 SDL 事件
    while (SDL_PollEvent(&event)) {  // 檢查事件佇列中的所有事件
        switch (event.type) {  // 根據事件類型進行處理
            case SDL_QUIT:  // 窗口關閉事件
                game_state->is_running = false;  // 設定遊戲狀態為非運行中
                break;
            case SDL_KEYDOWN:  // 鍵盤按下事件
                key_pressed_this_frame[event.key.keysym.scancode] = true;  // 設定對應按鍵標記為 true
                break;
        }
    }

    // 根據當前遊戲狀態處理輸入
    switch (current_input_state) {
        case INPUT_STATE_MAIN_MENU:  // 主選單狀態
            if (key_pressed_this_frame[SDL_SCANCODE_RETURN] ||  // 按下 Enter 鍵
                key_pressed_this_frame[SDL_SCANCODE_SPACE]) {  // 或按下空格鍵
                printf("主選單：開始遊戲，進入角色選擇\n");
                current_input_state = INPUT_STATE_CHARACTER_SELECT;  // 切換到角色選擇狀態
            }
            break;

        case INPUT_STATE_CHARACTER_SELECT:  // 角色選擇狀態
            // 玩家 1 角色選擇控制
            if (key_pressed_this_frame[player1_keys.up]) {  // 按下玩家 1 的上移動鍵
                printf("玩家 1 選擇上一個角色\n");
                // 這裡應呼叫角色選擇的實際邏輯
            }
            if (key_pressed_this_frame[player1_keys.down]) {  // 按下玩家 1 的下移動鍵
                printf("玩家 1 選擇下一個角色\n");
                // 這裡應呼叫角色選擇的實際邏輯
            }
            // 玩家 2 角色選擇控制
            if (key_pressed_this_frame[player2_keys.up]) {  // 按下玩家 2 的上移動鍵
                printf("玩家 2 選擇上一個角色\n");
                // 這裡應呼叫角色選擇的實際邏輯
            }
            if (key_pressed_this_frame[player2_keys.down]) {  // 按下玩家 2 的下移動鍵
                printf("玩家 2 選擇下一個角色\n");
                // 這裡應呼叫角色選擇的實際邏輯
            }
            // 確認角色選擇
            if (key_pressed_this_frame[player1_keys.attack]) {  // 按下玩家 1 的攻擊鍵
                printf("玩家 1 確認角色選擇\n");
                character_selected_p1 = true;  // 設定玩家 1 已確認角色選擇
            }
            if (key_pressed_this_frame[player2_keys.attack]) {  // 按下玩家 2 的攻擊鍵
                printf("玩家 2 確認角色選擇\n");
                character_selected_p2 = true;  // 設定玩家 2 已確認角色選擇
            }
            if (character_selected_p1 && character_selected_p2) {  // 如果雙方玩家都已確認角色選擇
                printf("雙方玩家已確認角色，進入拳套選擇\n");
                current_input_state = INPUT_STATE_GLOVE_SELECT;  // 切換到拳套選擇狀態
                character_selected_p1 = false;  // 重置玩家 1 的角色選擇狀態
                character_selected_p2 = false;  // 重置玩家 2 的角色選擇狀態
            }
            if (key_pressed_this_frame[SDL_SCANCODE_ESCAPE]) {  // 按下 ESC 鍵
                printf("返回主選單\n");
                current_input_state = INPUT_STATE_MAIN_MENU;  // 切換到主選單狀態
            }
            break;

        case INPUT_STATE_GLOVE_SELECT:  // 拳套選擇狀態
            // 玩家 1 拳套選擇控制
            if (key_pressed_this_frame[player1_keys.up]) {  // 按下玩家 1 的上移動鍵
                printf("玩家 1 選擇上一個拳套\n");
                // 這裡應呼叫拳套選擇的實際邏輯
            }
            if (key_pressed_this_frame[player1_keys.down]) {  // 按下玩家 1 的下移動鍵
                printf("玩家 1 選擇下一個拳套\n");
                // 這裡應呼叫拳套選擇的實際邏輯
            }
            // 玩家 2 拳套選擇控制
            if (key_pressed_this_frame[player2_keys.up]) {  // 按下玩家 2 的上移動鍵
                printf("玩家 2 選擇上一個拳套\n");
                // 這裡應呼叫拳套選擇的實際邏輯
            }
            if (key_pressed_this_frame[player2_keys.down]) {  // 按下玩家 2 的下移動鍵
                printf("玩家 2 選擇下一個拳套\n");
                // 這裡應呼叫拳套選擇的實際邏輯
            }
            // 確認拳套選擇
            if (key_pressed_this_frame[player1_keys.attack]) {  // 按下玩家 1 的攻擊鍵
                printf("玩家 1 確認拳套選擇\n");
                glove_selected_p1 = true;  // 設定玩家 1 已確認拳套選擇
            }
            if (key_pressed_this_frame[player2_keys.attack]) {  // 按下玩家 2 的攻擊鍵
                printf("玩家 2 確認拳套選擇\n");
                glove_selected_p2 = true;  // 設定玩家 2 已確認拳套選擇
            }
            if (glove_selected_p1 && glove_selected_p2) {  // 如果雙方玩家都已確認拳套選擇
                printf("雙方玩家已確認拳套，開始遊戲！\n");
                current_input_state = INPUT_STATE_GAMEPLAY;  // 切換到遊戲進行狀態
                glove_selected_p1 = false;  // 重置玩家 1 的拳套選擇狀態
                glove_selected_p2 = false;  // 重置玩家 2 的拳套選擇狀態
                init_player(player1, 200, 400, 1);  // 初始化玩家 1
                init_player(player2, 1000, 400, -1);  // 初始化玩家 2
            }
            if (key_pressed_this_frame[SDL_SCANCODE_ESCAPE]) {  // 按下 ESC 鍵
                printf("返回角色選擇\n");
                current_input_state = INPUT_STATE_CHARACTER_SELECT;  // 切換到角色選擇狀態
                glove_selected_p1 = false;  // 重置玩家 1 的拳套選擇狀態
                glove_selected_p2 = false;  // 重置玩家 2 的拳套選擇狀態
            }
            break;

        case INPUT_STATE_GAMEPLAY:  // 遊戲進行狀態
            // 玩家 1 遊戲控制
            if (current_key_states[player1_keys.left]) {  // 按住玩家 1 的左移動鍵
                handlePlayerCommand(player1, CMD_LEFT);  // 處理玩家 1 左移動指令
            }
            if (current_key_states[player1_keys.right]) {  // 按住玩家 1 的右移動鍵
                handlePlayerCommand(player1, CMD_RIGHT);  // 處理玩家 1 右移動指令
            }
            if (key_pressed_this_frame[player1_keys.jump]) {  // 按下玩家 1 的跳躍鍵
                handlePlayerCommand(player1, CMD_JUMP);  // 處理玩家 1 跳躍指令
            }
            if (key_pressed_this_frame[player1_keys.crouch]) {  // 按下玩家 1 的蹲下鍵
                handlePlayerCommand(player1, CMD_CROUCH_START);  // 處理玩家 1 蹲下開始指令
            } else if (!current_key_states[player1_keys.crouch]) {  // 釋放玩家 1 的蹲下鍵
                handlePlayerCommand(player1, CMD_CROUCH_END);  // 處理玩家 1 蹲下結束指令
            }
            if (key_pressed_this_frame[player1_keys.attack]) {  // 按下玩家 1 的攻擊鍵
                handlePlayerCommand(player1, CMD_ATTACK);  // 處理玩家 1 攻擊指令
            }
            if (key_pressed_this_frame[player1_keys.block]) {  // 按下玩家 1 的防禦鍵
                handlePlayerCommand(player1, CMD_BLOCK_START);  // 處理玩家 1 防禦開始指令
            } else if (!current_key_states[player1_keys.block]) {  // 釋放玩家 1 的防禦鍵
                handlePlayerCommand(player1, CMD_BLOCK_END);  // 處理玩家 1 防禦結束指令
            }
            // 玩家 2 遊戲控制
            if (current_key_states[player2_keys.left]) {  // 按住玩家 2 的左移動鍵
                handlePlayerCommand(player2, CMD_LEFT);  // 處理玩家 2 左移動指令
            }
            if (current_key_states[player2_keys.right]) {  // 按住玩家 2 的右移動鍵
                handlePlayerCommand(player2, CMD_RIGHT);  // 處理玩家 2 右移動指令
            }
            if (key_pressed_this_frame[player2_keys.jump]) {  // 按下玩家 2 的跳躍鍵
                handlePlayerCommand(player2, CMD_JUMP);  // 處理玩家 2 跳躍指令
            }
            if (key_pressed_this_frame[player2_keys.crouch]) {  // 按下玩家 2 的蹲下鍵
                handlePlayerCommand(player2, CMD_CROUCH_START);  // 處理玩家 2 蹲下開始指令
            } else if (!current_key_states[player2_keys.crouch]) {  // 釋放玩家 2 的蹲下鍵
                handlePlayerCommand(player2, CMD_CROUCH_END);  // 處理玩家 2 蹲下結束指令
            }
            if (key_pressed_this_frame[player2_keys.attack]) {  // 按下玩家 2 的攻擊鍵
                handlePlayerCommand(player2, CMD_ATTACK);  // 處理玩家 2 攻擊指令
            }
            if (key_pressed_this_frame[player2_keys.block]) {  // 按下玩家 2 的防禦鍵
                handlePlayerCommand(player2, CMD_BLOCK_START);  // 處理玩家 2 防禦開始指令
            } else if (!current_key_states[player2_keys.block]) {  // 釋放玩家 2 的防禦鍵
                handlePlayerCommand(player2, CMD_BLOCK_END);  // 處理玩家 2 防禦結束指令
            }
            // 全局控制
            if (key_pressed_this_frame[SDL_SCANCODE_ESCAPE]) {  // 按下 ESC 鍵
                printf("返回角色選擇\n");
                current_input_state = INPUT_STATE_CHARACTER_SELECT;  // 切換到角色選擇狀態
            }
            break;

        case INPUT_STATE_GAME_OVER:  // 遊戲結束狀態
            if (key_pressed_this_frame[SDL_SCANCODE_SPACE]) {  // 按下空格鍵
                printf("再玩一次\n");
                current_input_state = INPUT_STATE_CHARACTER_SELECT;  // 切換到角色選擇狀態
            }
            if (key_pressed_this_frame[SDL_SCANCODE_ESCAPE]) {  // 按下 ESC 鍵
                printf("返回主選單\n");
                current_input_state = INPUT_STATE_MAIN_MENU;  // 切換到主選單狀態
            }
            break;
    }

    // 處理中立狀態
    if (!current_key_states[player1_keys.left] && !current_key_states[player1_keys.right]) {  // 如果玩家 1 沒有按住左右移動鍵
        handlePlayerCommand(player1, CMD_NONE);  // 處理玩家 1 中立指令
    }
    if (!current_key_states[player2_keys.left] && !current_key_states[player2_keys.right]) {  // 如果玩家 2 沒有按住左右移動鍵
        handlePlayerCommand(player2, CMD_NONE);  // 處理玩家 2 中立指令
    }
}
