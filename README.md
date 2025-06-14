# -Ctrl-c-Ctrl-v 拳上-頂上戰爭

## 專案簡介
這是一款使用 C++ 和 SDL2 函式庫開發的 2D 格鬥遊戲，靈感來自經典的快打旋風。玩家可以選擇不同的角色和拳套，進行刺激的雙人對戰。遊戲包含多種模式和特色，有普通模式和混亂模式。

## 主要功能
* **雙人本地對戰**：支援兩名玩家在同一台電腦上進行對戰。
* **角色選擇**：
    * 目前提供兩名特色鮮明的角色：「統神」 和「國動」。
    * 每位角色擁有獨特的動畫、特殊技能和基本招式。
        * 統神特殊技能：「只剩一張帥臉」 - 以其英俊的帥臉使對手被帥到倒在地上無法自拔。
        * 國動特殊技能：「瘋狗衝刺」 - 向前衝刺，對路徑上的對手造成猶如坦克車輾過的衝擊。
* **拳套選擇系統**：
    * 提供三種不同重量的拳套 (10oz 蹦闆拿的, 14oz 等國動拿, 18oz 統神拿的) 供玩家選擇。
    * 不同的拳套會影響角色的攻擊力及攻擊冷卻時間。
* **回合制戰鬥**：
    * 遊戲採回合制，先贏得指定回合數 (預設為 2 回合) 的玩家獲得最終勝利。
* **豐富的戰鬥動作**：
    * 移動 (左右)、跳躍、躺下。
    * 普通攻擊。
    * 格擋 (可抵擋普通攻擊與氣功)。
    * 氣功攻擊。
* **混亂模式 (Chaos Mode)**：
    * 可選的遊戲模式，在對戰中會隨機觸發各種事件，增加遊戲的不可預測性和趣味性。
    * 目前事件包含：「超級控制大混亂!」(玩家操作方向顛倒) 和「血條交換!」(雙方血量百分比互換)。
* **多樣的遊戲介面**：
    * 開始畫面、角色選擇介面、拳套選擇介面。
    * 遊戲中暫停選單 (繼續遊戲、重新開始、回到主選單)。
    * 角色介紹畫面，展示角色背景 ("統神-最拉風的辣個男人", "國動-瘋狗的外號十歲就有") 和技能。
* **遊戲記錄**：
    * 自動保存最近五場遊戲的對戰記錄，包含對戰時間、角色、勝利者。
    * 可在開始畫面查看。
* **音效與背景音樂**：
    * 包含背景音樂以及攻擊、受傷、死亡、勝利等音效，提升遊戲沉浸感。

## 技術棧
* **程式語言**: C++
* **核心函式庫**:
    * SDL2: 用於視窗管理、事件處理、2D 渲染等底層操作。
    * SDL_image: 用於載入多種格式的圖片檔案 (如 PNG, JPG)。
    * SDL_mixer: 用於載入和播放音訊檔案 (如 WAV, MP3, OGG)。
    * SDL_ttf: 用於渲染 TrueType 字型文字。

## 如何編譯與執行

### 環境需求
* C++ 編譯器 (例如 GCC/G++, Clang)
* SDL2 函式庫
* SDL2_image 函式庫
* SDL2_mixer 函式庫
* SDL2_ttf 函式庫

### 編譯步驟 (以 g++ 為例)
1.  確保上述函式庫已正確安裝於您的系統中。
2.  打開終端機或命令提示字元，導航至專案的 `src` 目錄。
3.  執行以下編譯指令：
    ```bash
    g++ main.cpp Game.cpp Player.cpp AnimationData.cpp TextureManager.cpp AudioManager.cpp -o StreetFighterGame -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf
    ```
    *(請根據您的系統和函式庫安裝路徑調整連結器參數。您可能需要加入 `-I` 來指定 SDL 標頭檔路徑，以及 `-L` 來指定函式庫路徑。)*

### 運行遊戲
1.  編譯成功後，會在 `src` 目錄下產生名為 `StreetFighterGame` (或您指定的輸出檔名) 的執行檔。
2.  **重要**: 遊戲需要 `assets` 資料夾來載入資源。請確保 `assets` 資料夾 (包含如 `fonts/msjh.ttf`, `start_picture.jpg`, `asiagodton.png`, `bgm.wav` 等資源) 與執行檔位於同一目錄層級，或者您需要修改程式碼中的資源相對路徑。
3.  執行遊戲：
    ```bash
    ./StreetFighterGame
    ```

## 遊戲操控說明

### 通用
* **ESC**:
    * 在遊戲中：暫停遊戲，打開暫停選單。
    * 在暫停選單/角色介紹/遊戲記錄畫面：返回上一層或主選單。

### 玩家 1 (P1) - 鍵盤左側
* **移動**:
    * `A`: 向左移動
    * `D`: 向右移動
* **動作**:
    * `W`: 跳躍
    * `S`: 躺下
    * `J`: 普通攻擊
    * `U`: 發射氣功
    * `K` (按住): 格擋
    * `I`: 特殊技能
* **選單操作**:
    * `W` / `S`: 上下選擇
    * `Enter`: 確認

### 玩家 2 (P2) - 鍵盤右側 (部分使用小鍵盤)
* **移動**:
    * `方向鍵 ←`: 向左移動
    * `方向鍵 →`: 向右移動
* **動作**:
    * `方向鍵 ↑`: 跳躍
    * `方向鍵 ↓`: 躺下
    * `小鍵盤 1`: 普通攻擊
    * `小鍵盤 4`: 發射氣功
    * `小鍵盤 2` (按住): 格擋
    * `小鍵盤 5`: 特殊技能
* **選單操作**:
    * `方向鍵 ↑` / `方向鍵 ↓`: 上下選擇
    * `小鍵盤 Enter`: 確認

### 混亂模式 - 控制反轉
* 當「超級控制大混亂!」事件觸發時，以上所有玩家的移動、跳躍、蹲下、攻擊、氣功等按鍵功能將會左右或上下顛倒。 例如，P1 的 `A` 鍵將變為向右移動，`D` 鍵變為向左移動。

## 專案結構 (`src` 資料夾內)
src/

├── main.cpp                  # 程式主入口點，初始化並運行遊戲

├── Game.h/.cpp               # 遊戲核心邏輯，狀態管理，事件處理，渲染迴圈

├── Player.h/.cpp             # 玩家角色類別，處理玩家動作、狀態、碰撞、動畫

├── AnimationData.h/.cpp      # 管理角色動畫幀數據與定義

├── TextureManager.h/.cpp     # 靜態類別，用於載入、管理和釋放遊戲紋理

├── AudioManager.h/.cpp       # 靜態類別，用於載入、管理和播放背景音樂與音效

├── Constants.h               # 定義遊戲中使用的全域常數 (如螢幕尺寸、物理參數、遊戲規則等)

├── record.txt                # 文字檔案，用於儲存最近的遊戲記錄

└── assets/                   # 存放所有遊戲資源

├── fonts/                # 字型檔案 (例如 msjh.ttf)

├── images/               # 圖片檔案 (例如 asiagodton.png, godon.png, start_picture.jpg, image.png 等)

└── sounds/               # 音效檔案 (例如 bgm.wav, hurt0.wav, fire0.wav, lose0.wav, victory0.wav 等)
