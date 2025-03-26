#include <SDL2/SDL.h>

// 定義窗口尺寸
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

int main(int argc, char* argv[]) {
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

    // 初始化 SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("SDL 初始化失敗: %s", SDL_GetError());
        return -1;
    }

    // 創建窗口
    window = SDL_CreateWindow(
        "SDL2 測試窗口",                 // 標題
        SDL_WINDOWPOS_CENTERED,          // X 位置
        SDL_WINDOWPOS_CENTERED,          // Y 位置
        WINDOW_WIDTH,                    // 寬度
        WINDOW_HEIGHT,                   // 高度
        SDL_WINDOW_SHOWN                 // 標誌
    );

    if (!window) {
        SDL_Log("窗口創建失敗: %s", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    // 創建渲染器
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_Log("渲染器創建失敗: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // 主循環標誌
    int is_running = 1;
    SDL_Event event;

    // 主循環
    while (is_running) {
        // 處理事件
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                is_running = 0;
            }
        }

        // 清空畫面（黑色背景）
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // 繪製一個白色十字
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawLine(renderer, 0, WINDOW_HEIGHT/2, WINDOW_WIDTH, WINDOW_HEIGHT/2); // 水平線
        SDL_RenderDrawLine(renderer, WINDOW_WIDTH/2, 0, WINDOW_WIDTH/2, WINDOW_HEIGHT);  // 垂直線

        // 更新畫面
        SDL_RenderPresent(renderer);
    }

    // 清理資源
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}