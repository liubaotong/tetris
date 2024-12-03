#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h>
#include <time.h>

#define WIDTH 12      // 游戏区域宽度
#define HEIGHT 20     // 游戏区域高度
#define EMPTY 0       // 空白区域
#define BLOCK 1       // 方块
#define WALL 2        // 墙壁

// 自定义颜色常量
#define FOREGROUND_CYAN FOREGROUND_GREEN | FOREGROUND_BLUE

// 游戏状态
int score = 0;
int gameOver = 0;
int board[HEIGHT][WIDTH];
int currentPiece[4][4];
int currentX, currentY;
int currentType;
int nextType;
HANDLE hConsole;

// 方块形状定义
const int PIECES[7][4][4] = {
    // I
    {
        {0,0,0,0},
        {1,1,1,1},
        {0,0,0,0},
        {0,0,0,0}
    },
    // J
    {
        {1,0,0,0},
        {1,1,1,0},
        {0,0,0,0},
        {0,0,0,0}
    },
    // L
    {
        {0,0,1,0},
        {1,1,1,0},
        {0,0,0,0},
        {0,0,0,0}
    },
    // O
    {
        {1,1,0,0},
        {1,1,0,0},
        {0,0,0,0},
        {0,0,0,0}
    },
    // S
    {
        {0,1,1,0},
        {1,1,0,0},
        {0,0,0,0},
        {0,0,0,0}
    },
    // T
    {
        {0,1,0,0},
        {1,1,1,0},
        {0,0,0,0},
        {0,0,0,0}
    },
    // Z
    {
        {1,1,0,0},
        {0,1,1,0},
        {0,0,0,0},
        {0,0,0,0}
    }
};

// 函数声明
void initGame();
void drawBoard();
void createNewPiece();
int canMove(int newX, int newY, int rotation);
void rotatePiece();
void mergePiece();
void clearLines();
void initConsole();
void drawNextPiece();

// 初始化游戏
void initGame() {
    int i, j;
    // 初始化游戏区域
    for(i = 0; i < HEIGHT; i++) {
        for(j = 0; j < WIDTH; j++) {
            if(j == 0 || j == WIDTH-1 || i == HEIGHT-1 || i == 0)
                board[i][j] = WALL;
            else
                board[i][j] = EMPTY;
        }
    }
    
    // 初始化随机数生成器
    srand(time(NULL));
    
    // 生成第一个方块
    nextType = rand() % 7;
    createNewPiece();
}

// 初始化控制台
void initConsole() {
    // 获取控制台句柄
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) {
        printf("Error getting console handle\n");
        exit(1);
    }

    // 设置控制台编码为UTF-8
    SetConsoleOutputCP(CP_UTF8);
    
    // 设置控制台窗口和缓冲区大小
    SMALL_RECT windowSize = {0, 0, WIDTH * 2 + 20, HEIGHT + 5};
    COORD bufferSize = {WIDTH * 2 + 21, HEIGHT + 6};
    
    // 先设置窗口大小为较小的值
    SMALL_RECT tempWindowSize = {0, 0, 1, 1};
    SetConsoleWindowInfo(hConsole, TRUE, &tempWindowSize);
    
    // 然后设置缓冲区大小
    if (!SetConsoleScreenBufferSize(hConsole, bufferSize)) {
        printf("Error setting buffer size: %lu\n", GetLastError());
        exit(1);
    }
    
    // 最后设置窗口大小
    if (!SetConsoleWindowInfo(hConsole, TRUE, &windowSize)) {
        printf("Error setting window size: %lu\n", GetLastError());
        exit(1);
    }

    // 隐藏光标
    CONSOLE_CURSOR_INFO cursorInfo;
    cursorInfo.dwSize = 1;
    cursorInfo.bVisible = FALSE;
    if (!SetConsoleCursorInfo(hConsole, &cursorInfo)) {
        printf("Error hiding cursor\n");
        exit(1);
    }

    // 清屏
    system("cls");
}

// 绘制下一个方块的预览
void drawNextPiece() {
    int k, l;
    COORD pos_coord;
    
    // 清除预览区域
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    for(k = 0; k < 4; k++) {
        pos_coord.X = WIDTH * 2 + 2;
        pos_coord.Y = 3 + k;
        SetConsoleCursorPosition(hConsole, pos_coord);
        printf("        ");  // 清除整行
    }
    
    // 绘制下一个方块
    SetConsoleTextAttribute(hConsole, FOREGROUND_CYAN | FOREGROUND_INTENSITY);
    for(k = 0; k < 4; k++) {
        for(l = 0; l < 4; l++) {
            if(PIECES[nextType][k][l]) {
                pos_coord.X = WIDTH * 2 + 2 + l * 2;
                pos_coord.Y = 3 + k;
                SetConsoleCursorPosition(hConsole, pos_coord);
                printf("[]");
            }
        }
    }
}

// 绘制游戏界面
void drawBoard() {
    int i, j, k, l;
    char buffer[HEIGHT * (WIDTH * 2 + 1) + 1];
    int pos = 0;
    WORD originalAttrs;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    
    // 保存原始属性
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    originalAttrs = csbi.wAttributes;

    // 绘制游戏区域
    for(i = 0; i < HEIGHT; i++) {
        for(j = 0; j < WIDTH; j++) {
            if(board[i][j] == WALL) {
                buffer[pos++] = '#';
                buffer[pos++] = '#';
            }
            else if(board[i][j] == BLOCK) {
                buffer[pos++] = '[';
                buffer[pos++] = ']';
            }
            else {
                buffer[pos++] = ' ';
                buffer[pos++] = ' ';
            }
        }
        buffer[pos++] = '\n';
    }
    buffer[pos] = '\0';

    // 移动光标到开始位置并输出游戏区域
    COORD pos_coord = {0, 0};
    SetConsoleCursorPosition(hConsole, pos_coord);
    printf("%s", buffer);

    // 绘制当前方块
    SetConsoleTextAttribute(hConsole, FOREGROUND_CYAN | FOREGROUND_INTENSITY);
    for(k = 0; k < 4; k++) {
        for(l = 0; l < 4; l++) {
            if(currentPiece[k][l] && currentY + k >= 0) {
                pos_coord.X = (currentX + l) * 2;
                pos_coord.Y = currentY + k;
                SetConsoleCursorPosition(hConsole, pos_coord);
                printf("[]");
            }
        }
    }

    // 显示分数
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    pos_coord.X = WIDTH * 2 + 2;
    pos_coord.Y = 0;
    SetConsoleCursorPosition(hConsole, pos_coord);
    printf("分数: %d", score);

    // 显示下一个方块标题
    pos_coord.Y = 2;
    SetConsoleCursorPosition(hConsole, pos_coord);
    printf("下一个:");

    // 绘制下一个方块预览
    drawNextPiece();

    // 显示控制说明
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    pos_coord.X = WIDTH * 2 + 2;
    pos_coord.Y = 8;
    SetConsoleCursorPosition(hConsole, pos_coord);
    printf("控制:");
    pos_coord.Y++;
    SetConsoleCursorPosition(hConsole, pos_coord);
    printf("← → : 移动");
    pos_coord.Y++;
    SetConsoleCursorPosition(hConsole, pos_coord);
    printf("↑    : 旋转");
    pos_coord.Y++;
    SetConsoleCursorPosition(hConsole, pos_coord);
    printf("↓    : 加速下落");
    pos_coord.Y++;
    SetConsoleCursorPosition(hConsole, pos_coord);
    printf("ESC   : 退出");

    // 恢复原始属性
    SetConsoleTextAttribute(hConsole, originalAttrs);
}

// 创建新方块
void createNewPiece() {
    int i, j;
    currentType = nextType;
    nextType = rand() % 7;
    
    // 复制方块形状
    for(i = 0; i < 4; i++)
        for(j = 0; j < 4; j++)
            currentPiece[i][j] = PIECES[currentType][i][j];
    
    // 设置初始位置
    currentX = WIDTH/2 - 2;
    currentY = 1;
    
    // 检查游戏是否结束
    if(!canMove(currentX, currentY, 0)) {
        gameOver = 1;
    }
}

// 检查是否可以移动
int canMove(int newX, int newY, int rotation) {
    int i, j;
    int tempPiece[4][4];
    
    // 如果需要旋转，先创建临时旋转后的方块
    if(rotation) {
        for(i = 0; i < 4; i++)
            for(j = 0; j < 4; j++)
                tempPiece[i][j] = currentPiece[3-j][i];
    }
    
    // 检查每个方块位置
    for(i = 0; i < 4; i++) {
        for(j = 0; j < 4; j++) {
            if((rotation ? tempPiece[i][j] : currentPiece[i][j])) {
                if(newX+j < 0 || newX+j >= WIDTH || newY+i >= HEIGHT)
                    return 0;
                if(board[newY+i][newX+j] != EMPTY)
                    return 0;
            }
        }
    }
    return 1;
}

// 旋转方块
void rotatePiece() {
    int i, j;
    int tempPiece[4][4];
    
    // 创建临时旋转后的方块
    for(i = 0; i < 4; i++)
        for(j = 0; j < 4; j++)
            tempPiece[i][j] = currentPiece[3-j][i];
    
    // 如果可以旋转，更新方块
    if(canMove(currentX, currentY, 1)) {
        for(i = 0; i < 4; i++)
            for(j = 0; j < 4; j++)
                currentPiece[i][j] = tempPiece[i][j];
    }
}

// 合并方块到游戏区域
void mergePiece() {
    int i, j;
    for(i = 0; i < 4; i++) {
        for(j = 0; j < 4; j++) {
            if(currentPiece[i][j] && currentY + i >= 0) {
                board[currentY + i][currentX + j] = BLOCK;
            }
        }
    }
}

// 清除已完成的行
void clearLines() {
    int i, j, k, l;
    int linesCleared = 0;
    
    for(i = HEIGHT-2; i > 0; i--) {  // 从倒数第二行开始检查，保护上边框
        int full = 1;
        for(j = 1; j < WIDTH-1; j++) {
            if(board[i][j] == EMPTY) {
                full = 0;
                break;
            }
        }
        
        if(full) {
            linesCleared++;
            // 将上面的行向下移动
            for(k = i; k > 1; k--) {  // 从k > 1开始，保护上边框
                for(l = 1; l < WIDTH-1; l++) {
                    board[k][l] = board[k-1][l];
                }
            }
            // 清空最上面的行（除了边框）
            for(l = 1; l < WIDTH-1; l++) {
                board[1][l] = EMPTY;  // 保持在第1行，而不是第0行
            }
            i++;  // 重新检查当前行
        }
    }
    
    // 更新分数
    score += linesCleared * 100;
}

int main() {
    int key;
    int dropCounter = 0;
    int gameSpeed = 20;  // 游戏速度（越小越快）
    int fastDrop = 0;    // 加速状态
    
    // 初始化
    system("mode con cols=50 lines=30");  // 预设控制台窗口大小
    initConsole();
    initGame();
    
    // 主游戏循环
    while(!gameOver) {
        // 检查下键是否被按住
        if(GetAsyncKeyState(VK_DOWN) & 0x8000) {
            fastDrop = 1;
        } else {
            fastDrop = 0;
        }

        // 处理其他按键输入
        if(_kbhit()) {
            key = _getch();
            switch(key) {
                case 27:  // ESC
                    gameOver = 1;
                    break;
                case 72:  // 上箭头
                    rotatePiece();
                    break;
                case 75:  // 左箭头
                    if(canMove(currentX-1, currentY, 0))
                        currentX--;
                    break;
                case 77:  // 右箭头
                    if(canMove(currentX+1, currentY, 0))
                        currentX++;
                    break;
            }
        }
        
        // 自动下落（根据是否加速调整速度）
        if(dropCounter++ > (fastDrop ? 2 : gameSpeed)) {
            dropCounter = 0;
            if(canMove(currentX, currentY+1, 0)) {
                currentY++;
            } else {
                mergePiece();
                clearLines();
                createNewPiece();
            }
        }
        
        drawBoard();
        Sleep(30);
    }
    
    // 游戏结束
    system("cls");
    printf("\n\n  游戏结束!\n");
    printf("  最终分数: %d\n\n", score);
    printf("  按任意键退出...");
    _getch();
    
    return 0;
}
