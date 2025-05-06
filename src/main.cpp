#include "Game.h" // 只需要包含 Game.h

int main(int argc, char* argv[]) {
    Game game; // 創建 Game 物件

    if (game.initialize()) { // 初始化遊戲
        game.run(); // 運行遊戲主迴圈
    }

    game.cleanup(); // 清理資源

    return 0; // 程式結束
}