#include "input.h"
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>

// 當前輸入處理狀態，決定如何處理玩家輸入
static InputState current_input_state = INPUT_STATE_MAIN_MENU;

// SDL鍵盤狀態陣列指針，用於檢測按鍵按住狀態
static const Uint8* current_key_states = NULL;

// 本幀按鍵按下標記陣列，用於檢測按鍵按下瞬間
static bool key_pressed_this_frame[SDL_NUM_SCANCODES] = {false};

// === 選擇狀態追蹤變數 ===
static bool character_selected_p1 = false; ///< 玩家1是否已確認角色選擇
static bool character_selected_p2 = false; ///< 玩家2是否已確認角色選擇
static bool glove_selected_p1 = false;    ///< 玩家1是否已確認拳套選擇
static bool glove_selected_p2 = false;    ///< 玩家2是否已確認拳套選擇

/**
 * @brief 初始化輸入系統
 * 
 * 獲取SDL鍵盤狀態陣列指針，必須在SDL初始化後呼叫
 */
void init_input() {
    current_key_states = SDL_GetKeyboardState(NULL);
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
 * @param player1 玩家1物件指針
 * @param player2 玩家2物件指針
 * @param game_state 遊戲狀態指針
 */
void process_input(Player* player1, Player* player2, GameState* game_state) {
    SDL_Event event;
    
    // === 重置本幀按鍵按下標記 ===
    // 每幀開始時重置所有按鍵的"按下瞬間"標記
    for (int i = 0; i < SDL_NUM_SCANCODES; i++) {
        key_pressed_this_frame[i] = false;
    }
    
    // === 處理所有SDL事件 ===
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:  // 窗口關閉事件
                game_state->is_running = false;
                break;
                
            case SDL_KEYDOWN:  // 鍵盤按下事件
                // 標記該按鍵在本幀被按下
                key_pressed_this_frame[event.key.keysym.scancode] = true;
                break;
                
            // 注意：我們不處理SDL_KEYUP，因為使用current_key_states檢測按住狀態
        }
    }
    
    // === 根據當前遊戲狀態處理輸入 ===
    switch (current_input_state) {
        // === 主選單狀態處理 ===
        case INPUT_STATE_MAIN_MENU:
            // 按Enter或Space鍵開始遊戲
            if (key_pressed_this_frame[SDL_SCANCODE_RETURN] || 
                key_pressed_this_frame[SDL_SCANCODE_SPACE]) {
                printf("主選單：開始遊戲，進入角色選擇\n");
                current_input_state = INPUT_STATE_CHARACTER_SELECT;
            }
            break;
            
        // === 角色選擇狀態處理 ===
        case INPUT_STATE_CHARACTER_SELECT:
            // --- 玩家1角色選擇控制 ---
            // W鍵：選擇上一個角色
            if (key_pressed_this_frame[SDL_SCANCODE_W]) {
                printf("玩家1選擇上一個角色\n");
                // 這裡應呼叫角色選擇的實際邏輯
            }
            // S鍵：選擇下一個角色
            if (key_pressed_this_frame[SDL_SCANCODE_S]) {
                printf("玩家1選擇下一個角色\n");
                // 這裡應呼叫角色選擇的實際邏輯
            }
            
            // --- 玩家2角色選擇控制 ---
            // 上箭頭鍵：選擇上一個角色
            if (key_pressed_this_frame[SDL_SCANCODE_UP]) {
                printf("玩家2選擇上一個角色\n");
                // 這裡應呼叫角色選擇的實際邏輯
            }
            // 下箭頭鍵：選擇下一個角色
            if (key_pressed_this_frame[SDL_SCANCODE_DOWN]) {
                printf("玩家2選擇下一個角色\n");
                // 這裡應呼叫角色選擇的實際邏輯
            }
            
            // --- 確認角色選擇 ---
            // 玩家1按J鍵確認選擇
            if (key_pressed_this_frame[SDL_SCANCODE_J]) {
                printf("玩家1確認角色選擇\n");
                character_selected_p1 = true;
            }
            // 玩家2按1鍵確認選擇
            if (key_pressed_this_frame[SDL_SCANCODE_1]) {
                printf("玩家2確認角色選擇\n");
                character_selected_p2 = true;
            }
            
            // 雙方都確認後進入拳套選擇狀態
            if (character_selected_p1 && character_selected_p2) {
                printf("雙方玩家已確認角色，進入拳套選擇\n");
                current_input_state = INPUT_STATE_GLOVE_SELECT;
                // 重置選擇狀態
                character_selected_p1 = false;
                character_selected_p2 = false;
            }
            
            // ESC鍵返回主選單
            if (key_pressed_this_frame[SDL_SCANCODE_ESCAPE]) {
                printf("返回主選單\n");
                current_input_state = INPUT_STATE_MAIN_MENU;
            }
            break;
            
        // === 拳套選擇狀態處理 ===
        case INPUT_STATE_GLOVE_SELECT:
            // --- 玩家1拳套選擇控制 ---
            // W鍵：選擇上一個拳套
            if (key_pressed_this_frame[SDL_SCANCODE_W]) {
                printf("玩家1選擇上一個拳套\n");
                // 這裡應呼叫拳套選擇的實際邏輯
            }
            // S鍵：選擇下一個拳套
            if (key_pressed_this_frame[SDL_SCANCODE_S]) {
                printf("玩家1選擇下一個拳套\n");
                // 這裡應呼叫拳套選擇的實際邏輯
            }
            
            // --- 玩家2拳套選擇控制 ---
            // 上箭頭鍵：選擇上一個拳套
            if (key_pressed_this_frame[SDL_SCANCODE_UP]) {
                printf("玩家2選擇上一個拳套\n");
                // 這裡應呼叫拳套選擇的實際邏輯
            }
            // 下箭頭鍵：選擇下一個拳套
            if (key_pressed_this_frame[SDL_SCANCODE_DOWN]) {
                printf("玩家2選擇下一個拳套\n");
                // 這裡應呼叫拳套選擇的實際邏輯
            }
            
            // --- 確認拳套選擇 ---
            // 玩家1按J鍵確認選擇
            if (key_pressed_this_frame[SDL_SCANCODE_J]) {
                printf("玩家1確認拳套選擇\n");
                glove_selected_p1 = true;
            }
            // 玩家2按1鍵確認選擇
            if (key_pressed_this_frame[SDL_SCANCODE_1]) {
                printf("玩家2確認拳套選擇\n");
                glove_selected_p2 = true;
            }
            
            // 雙方都確認後進入遊戲狀態
            if (glove_selected_p1 && glove_selected_p2) {
                printf("雙方玩家已確認拳套，開始遊戲！\n");
                current_input_state = INPUT_STATE_GAMEPLAY;
                // 重置選擇狀態
                glove_selected_p1 = false;
                glove_selected_p2 = false;
                
                // 初始化玩家狀態
                // 玩家1初始位置在左側(200,400)，面向右(1)
                init_player(player1, 200, 400, 1);
                // 玩家2初始位置在右側(1000,400)，面向左(-1)
                init_player(player2, 1000, 400, -1);
            }
            
            // ESC鍵返回角色選擇
            if (key_pressed_this_frame[SDL_SCANCODE_ESCAPE]) {
                printf("返回角色選擇\n");
                current_input_state = INPUT_STATE_CHARACTER_SELECT;
                // 重置選擇狀態
                glove_selected_p1 = false;
                glove_selected_p2 = false;
            }
            break;
            
        // === 遊戲進行狀態處理 ===
        case INPUT_STATE_GAMEPLAY:
            // --- 玩家1遊戲控制 ---
            // A鍵：向左移動
            if (current_key_states[SDL_SCANCODE_A]) {
                handlePlayerCommand(player1, CMD_LEFT);
            }
            // D鍵：向右移動
            if (current_key_states[SDL_SCANCODE_D]) {
                handlePlayerCommand(player1, CMD_RIGHT);
            }
            // W鍵：跳躍(按下瞬間)
            if (key_pressed_this_frame[SDL_SCANCODE_W]) {
                handlePlayerCommand(player1, CMD_JUMP);
            }
            // S鍵：蹲下(按下開始蹲，釋放結束蹲)
            if (key_pressed_this_frame[SDL_SCANCODE_S]) {
                handlePlayerCommand(player1, CMD_CROUCH_START);
            } else if (!current_key_states[SDL_SCANCODE_S]) {
                handlePlayerCommand(player1, CMD_CROUCH_END);
            }
            // J鍵：攻擊(按下瞬間)
            if (key_pressed_this_frame[SDL_SCANCODE_J]) {
                handlePlayerCommand(player1, CMD_ATTACK);
            }
            // K鍵：防禦(按下開始防，釋放結束防)
            if (key_pressed_this_frame[SDL_SCANCODE_K]) {
                handlePlayerCommand(player1, CMD_BLOCK_START);
            } else if (!current_key_states[SDL_SCANCODE_K]) {
                handlePlayerCommand(player1, CMD_BLOCK_END);
            }
            
            // --- 玩家2遊戲控制 ---
            // 左箭頭鍵：向左移動
            if (current_key_states[SDL_SCANCODE_LEFT]) {
                handlePlayerCommand(player2, CMD_LEFT);
            }
            // 右箭頭鍵：向右移動
            if (current_key_states[SDL_SCANCODE_RIGHT]) {
                handlePlayerCommand(player2, CMD_RIGHT);
            }
            // 上箭頭鍵：跳躍(按下瞬間)
            if (key_pressed_this_frame[SDL_SCANCODE_UP]) {
                handlePlayerCommand(player2, CMD_JUMP);
            }
            // 下箭頭鍵：蹲下(按下開始蹲，釋放結束蹲)
            if (key_pressed_this_frame[SDL_SCANCODE_DOWN]) {
                handlePlayerCommand(player2, CMD_CROUCH_START);
            } else if (!current_key_states[SDL_SCANCODE_DOWN]) {
                handlePlayerCommand(player2, CMD_CROUCH_END);
            }
            // 1鍵：攻擊(按下瞬間)
            if (key_pressed_this_frame[SDL_SCANCODE_1]) {
                handlePlayerCommand(player2, CMD_ATTACK);
            }
            // 2鍵：防禦(按下開始防，釋放結束防)
            if (key_pressed_this_frame[SDL_SCANCODE_2]) {
                handlePlayerCommand(player2, CMD_BLOCK_START);
            } else if (!current_key_states[SDL_SCANCODE_2]) {
                handlePlayerCommand(player2, CMD_BLOCK_END);
            }
            
            // --- 全局控制 ---
            // ESC鍵返回角色選擇
            if (key_pressed_this_frame[SDL_SCANCODE_ESCAPE]) {
                printf("返回角色選擇\n");
                current_input_state = INPUT_STATE_CHARACTER_SELECT;
            }
            break;
            
        // === 遊戲結束狀態處理 ===
        case INPUT_STATE_GAME_OVER:
            // Space鍵再玩一次
            if (key_pressed_this_frame[SDL_SCANCODE_SPACE]) {
                printf("再玩一次\n");
                current_input_state = INPUT_STATE_CHARACTER_SELECT;
            }
            // ESC鍵返回主選單
            if (key_pressed_this_frame[SDL_SCANCODE_ESCAPE]) {
                printf("返回主選單\n");
                current_input_state = INPUT_STATE_MAIN_MENU;
            }
            break;
    }
    
    // === 處理中立狀態 ===
    // 當沒有移動按鍵被按下時，發送CMD_NONE指令
    if (!current_key_states[SDL_SCANCODE_A] && !current_key_states[SDL_SCANCODE_D]) {
        handlePlayerCommand(player1, CMD_NONE);
    }
    if (!current_key_states[SDL_SCANCODE_LEFT] && !current_key_states[SDL_SCANCODE_RIGHT]) {
        handlePlayerCommand(player2, CMD_NONE);
    }
}