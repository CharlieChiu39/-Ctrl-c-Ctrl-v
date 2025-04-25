# 編譯器設定
CC = g++
CFLAGS = -g -I"./include" -L"./lib"
LDFLAGS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_mixer -lSDL2_image -mwindows
TARGET = main.exe

# 源文件設定
SRC_DIR = src
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:.c=.o)

# 默認目標
all: $(TARGET)

# 主目標鏈接
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# 通用編譯規則
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 清理
clean:
	del /Q $(subst /,\,$(OBJS)) $(TARGET) 2>nul

.PHONY: all clean