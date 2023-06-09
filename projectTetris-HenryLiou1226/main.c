#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <windows.h>
#include <conio.h>

#define LEFT_KEY 0x25   // The key to move left, default = 0x25 (left arrow)
#define RIGHT_KEY 0x27  // The key to move right, default = 0x27 (right arrow)
#define ROTATE_KEY 0x26 // The key to rotate, default = 0x26 (up arrow)
#define DOWN_KEY 0x28   // The key to move down, default = 0x28 (down arrow)
#define FALL_KEY 0x20   // The key to fall, default = 0x20 (spacebar)

#define RENDER_DELAY 100 // The delay between each frame, default = 100

#define LEFT_FUNC() GetAsyncKeyState(LEFT_KEY) & 0x8000
#define RIGHT_FUNC() GetAsyncKeyState(RIGHT_KEY) & 0x8000
#define ROTATE_FUNC() GetAsyncKeyState(ROTATE_KEY) & 0x8000
#define DOWN_FUNC() GetAsyncKeyState(DOWN_KEY) & 0x8000
#define FALL_FUNC() GetAsyncKeyState(FALL_KEY) & 0x8000

#define CANVAS_WIDTH 10
#define CANVAS_HEIGHT 20

int FALL_DELAY;
typedef enum
{
    RED = 41,
    GREEN,
    YELLOW,
    BLUE,
    PURPLE,
    CYAN,
    WHITE,
    BLACK = 0,
    High_Intensty_WHITE = 107,
} Color;

typedef enum
{
    EMPTY = -1,
    I,
    J,
    L,
    O,
    S,
    T,
    Z
} ShapeId;

typedef struct
{
    ShapeId shape;
    Color color;
    int size;
    char rotates[4][4][4];
} Shape;

typedef struct
{
    bool hold;
    ShapeId hold_shape;
    int x;
    int y;
    int dead;
    int score;
    int rotate;
    int fallTime;
    ShapeId queue[4];
} State;

typedef struct
{
    Color color;
    ShapeId shape;
    bool current;
} Block;

Shape shapes[7] = {
    {.shape = I,
     .color = CYAN,
     .size = 4,
     .rotates =
         {
             {{0, 0, 0, 0},
              {1, 1, 1, 1},
              {0, 0, 0, 0},
              {0, 0, 0, 0}},
             {{0, 0, 1, 0},
              {0, 0, 1, 0},
              {0, 0, 1, 0},
              {0, 0, 1, 0}},
             {{0, 0, 0, 0},
              {0, 0, 0, 0},
              {1, 1, 1, 1},
              {0, 0, 0, 0}},
             {{0, 1, 0, 0},
              {0, 1, 0, 0},
              {0, 1, 0, 0},
              {0, 1, 0, 0}}}},
    {.shape = J,
     .color = BLUE,
     .size = 3,
     .rotates =
         {
             {{1, 0, 0},
              {1, 1, 1},
              {0, 0, 0}},
             {{0, 1, 1},
              {0, 1, 0},
              {0, 1, 0}},
             {{0, 0, 0},
              {1, 1, 1},
              {0, 0, 1}},
             {{0, 1, 0},
              {0, 1, 0},
              {1, 1, 0}}}},
    {.shape = L,
     .color = YELLOW,
     .size = 3,
     .rotates =
         {
             {{0, 0, 1},
              {1, 1, 1},
              {0, 0, 0}},
             {{0, 1, 0},
              {0, 1, 0},
              {0, 1, 1}},
             {{0, 0, 0},
              {1, 1, 1},
              {1, 0, 0}},
             {{1, 1, 0},
              {0, 1, 0},
              {0, 1, 0}}}},
    {.shape = O,
     .color = WHITE,
     .size = 2,
     .rotates =
         {
             {{1, 1},
              {1, 1}},
             {{1, 1},
              {1, 1}},
             {{1, 1},
              {1, 1}},
             {{1, 1},
              {1, 1}}}},
    {.shape = S,
     .color = GREEN,
     .size = 3,
     .rotates =
         {
             {{0, 1, 1},
              {1, 1, 0},
              {0, 0, 0}},
             {{0, 1, 0},
              {0, 1, 1},
              {0, 0, 1}},
             {{0, 0, 0},
              {0, 1, 1},
              {1, 1, 0}},
             {{1, 0, 0},
              {1, 1, 0},
              {0, 1, 0}}}},
    {.shape = T,
     .color = PURPLE,
     .size = 3,
     .rotates =
         {
             {{0, 1, 0},
              {1, 1, 1},
              {0, 0, 0}},

             {{0, 1, 0},
              {0, 1, 1},
              {0, 1, 0}},
             {{0, 0, 0},
              {1, 1, 1},
              {0, 1, 0}},
             {{0, 1, 0},
              {1, 1, 0},
              {0, 1, 0}}}},
    {.shape = Z,
     .color = RED,
     .size = 3,
     .rotates =
         {
             {{1, 1, 0},
              {0, 1, 1},
              {0, 0, 0}},
             {{0, 0, 1},
              {0, 1, 1},
              {0, 1, 0}},
             {{0, 0, 0},
              {1, 1, 0},
              {0, 1, 1}},
             {{0, 1, 0},
              {1, 1, 0},
              {1, 0, 0}}}},
};

void setBlock(Block *block, Color color, ShapeId shape, bool current)
{
    block->color = color;
    block->shape = shape;
    block->current = current;
}
void resetBlock(Block *block)
{
    block->color = BLACK;
    block->shape = EMPTY;
    block->current = false;
}
bool drop_predict(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH], int X_position, int Y_position, int newRotate, ShapeId shapeId)
{
    Shape shapeData = shapes[shapeId];
    for (int i = 0; i < shapeData.size; i++)
    {
        for (int j = 0; j < shapeData.size; j++)
        {
            if (shapeData.rotates[newRotate][i][j])
            {
                if (X_position + j < 0 || X_position + j >= CANVAS_WIDTH || Y_position + i < 0 || Y_position + i >= CANVAS_HEIGHT)
                {
                    return false;
                }
                if (!canvas[Y_position + i][X_position + j].current && canvas[Y_position + i][X_position + j].shape != EMPTY)
                {
                    return false;
                }
            }
        }
    }
    return true;
}
bool move(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH], int originalX, int originalY, int originalRotate, int newX, int newY, int newRotate, ShapeId shapeId)
{
    Shape shapeData = shapes[shapeId];
    int size = shapeData.size;
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            if (shapeData.rotates[newRotate][i][j])
            {
                if (newX + j < 0 || newX + j >= CANVAS_WIDTH || newY + i < 0 || newY + i >= CANVAS_HEIGHT)
                {
                    return false;
                }
                if (!canvas[newY + i][newX + j].current && canvas[newY + i][newX + j].shape != EMPTY)
                {
                    return false;
                }
            }
        }
    }

    // remove the old position
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            if (shapeData.rotates[originalRotate][i][j])
            {
                resetBlock(&canvas[originalY + i][originalX + j]);
            }
        }
    }

    // move the block
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            if (shapeData.rotates[newRotate][i][j])
            {
                setBlock(&canvas[newY + i][newX + j], shapeData.color, shapeId, true);
            }
        }
    }

    return true;
}
void printCanvas(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH], State *state)
{
    printf("\033[0;0H\n");
    Shape shapeData = shapes[state->queue[0]];
    int y_position = state->y + 1;
    for (int i = 0; i < CANVAS_HEIGHT; i++)
    {
        printf("|");
        for (int j = 0; j < CANVAS_WIDTH; j++)
        {
            if (canvas[i][j].color != 0)
                printf("\033[%dm\u3000", canvas[i][j].color);
            else
                printf("\033[38;5;241;40m。\033[0m");
        }
        printf("\033[0m|\n");
    }
    while (drop_predict(canvas, state->x, y_position, state->rotate, state->queue[0]))
    {
        y_position++;
    }
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (i < shapeData.size && j < shapeData.size && shapeData.rotates[state->rotate][i][j])
            {
                printf("\033[%d;%dH\033[%dm口\033[0m", y_position + i + 1, 2 * (state->x + j) + 2, 37);
            }
        }
    }
    shapeData = shapes[state->queue[1]];
    printf("\033[%d;%dHNext:", 3, CANVAS_WIDTH * 2 + 5);
    for (int i = 1; i <= 3; i++)
    {
        shapeData = shapes[state->queue[i]];
        for (int j = 0; j < 4; j++)
        {
            printf("\033[%d;%dH", i * 4 + j, CANVAS_WIDTH * 2 + 15);
            for (int k = 0; k < 4; k++)
            {
                if (j < shapeData.size && k < shapeData.size && shapeData.rotates[0][j][k])
                {
                    printf("\x1b[%dm  ", shapeData.color);
                }
                else
                {
                    printf("\x1b[0m  ");
                }
            }
        }
    }
    printf("\033[%d;%dHScore : %d\n", 23, 7, state->score);
    return;
}

int clearLine(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH])
{
    for (int i = 0; i < CANVAS_HEIGHT; i++)
    {
        for (int j = 0; j < CANVAS_WIDTH; j++)
        {
            if (canvas[i][j].current)
            {
                canvas[i][j].current = false;
            }
        }
    }

    int linesCleared = 0;

    for (int i = CANVAS_HEIGHT - 1; i >= 0; i--)
    {
        bool isFull = true;
        for (int j = 0; j < CANVAS_WIDTH; j++)
        {
            if (canvas[i][j].shape == EMPTY)
            {
                isFull = false;
                break;
            }
        }

        if (isFull)
        {
            linesCleared += 1;
            if (FALL_DELAY > 100)
            {
                FALL_DELAY -= 20;
            }
            for (int j = i; j > 0; j--)
            {
                for (int k = 0; k < CANVAS_WIDTH; k++)
                {
                    setBlock(&canvas[j][k], canvas[j - 1][k].color, canvas[j - 1][k].shape, false);
                    resetBlock(&canvas[j - 1][k]);
                }
            }
            i++;
        }
    }

    return linesCleared;
}

void logic(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH], State *state)
{
    if (ROTATE_FUNC())
    {
        int newRotate = (state->rotate + 1) % 4;
        if (move(canvas, state->x, state->y, state->rotate, state->x, state->y, newRotate, state->queue[0]))
        {
            state->rotate = newRotate;
        }
    }
    else if (LEFT_FUNC())
    {
        if (move(canvas, state->x, state->y, state->rotate, state->x - 1, state->y, state->rotate, state->queue[0]))
        {
            state->x -= 1;
        }
    }
    else if (RIGHT_FUNC())
    {
        if (move(canvas, state->x, state->y, state->rotate, state->x + 1, state->y, state->rotate, state->queue[0]))
        {
            state->x += 1;
        }
    }
    else if (DOWN_FUNC())
    {
        state->fallTime = FALL_DELAY;
    }
    else if (FALL_FUNC())
    {
        state->fallTime += FALL_DELAY * CANVAS_HEIGHT;
    }

    state->fallTime += RENDER_DELAY;

    while (state->fallTime >= FALL_DELAY)
    {
        state->fallTime -= FALL_DELAY;

        if (move(canvas, state->x, state->y, state->rotate, state->x, state->y + 1, state->rotate, state->queue[0]))
        {
            state->y++;
        }
        else
        {
            state->score += clearLine(canvas);

            state->x = CANVAS_WIDTH / 2;
            state->y = 0;
            state->rotate = 0;
            state->fallTime = 0;
            state->queue[0] = state->queue[1];
            state->queue[1] = state->queue[2];
            state->queue[2] = state->queue[3];
            state->queue[3] = rand() % 7;

            if (!move(canvas, state->x, state->y, state->rotate, state->x, state->y, state->rotate, state->queue[0]))
            {
                state->dead = 1;
                break;
            }
        }
    }
    return;
}
int key_pressed(void) /*偵測按下的按鍵並回傳*/
{
    if (_kbhit() != 0)
        return _getch();
    return 0;
}
int main()
{
    srand(time(NULL));
    printf("\tPress enter to start the game.\n");
    printf("    _____   ___   _____   ___    ___    ___   \n");
    printf("   |_   _| | __| |_   _| | _ \\  |_ _|  / __|  \n");
    printf("     | |   | _|    | |   |   /   | |   \\__ \\  \n");
    printf("     |_|   |___|   |_|   |_|_\\  |___|  |___/ \n");
    while (1)
    {
        int start = key_pressed();
        State state = {
            .hold = FALSE,
            .hold_shape = EMPTY,
            .x = CANVAS_WIDTH / 2,
            .y = 0,
            .dead = 0,
            .score = 0,
            .rotate = 0,
            .fallTime = 0};
        for (int i = 0; i < 4; i++)
        {
            state.queue[i] = rand() % 7;
        }
        Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH];
        for (int i = 0; i < CANVAS_HEIGHT; i++)
        {
            for (int j = 0; j < CANVAS_WIDTH; j++)
            {
                resetBlock(&canvas[i][j]);
            }
        }
        move(canvas, state.x, state.y, state.rotate, state.x, state.y, state.rotate, state.queue[0]);
        if (start == 0)
        {
            continue;
        }
        else if (start == 27)
        {
            break;
        }
        else if (start == 13)
        {
            system("cls");
            printf("\tPress key to choose difficulty:\n");
            printf("\t\t1. Easy\n");
            printf("\t\t2. Normal\n");
            printf("\t\t3. Hard\n");
            printf("\t\t4. Impossible\n");
            printf("     _____   ___   _____   ___    ___    ___   \n");
            printf("    |_   _| | __| |_   _| | _ \\  |_ _|  / __|  \n");
            printf("      | |   | _|    | |   |   /   | |   \\__ \\  \n");
            printf("      |_|   |___|   |_|   |_|_\\  |___|  |___/ \n");
            while (1)
            {
                int hard = key_pressed();
                if (hard == 0)
                {
                    continue;
                }
                else if (hard == 49)
                {
                    FALL_DELAY = 1000;
                    break;
                }
                else if (hard == 50)
                {
                    FALL_DELAY = 500;
                    break;
                }
                else if (hard == 51)
                {
                    FALL_DELAY = 300;
                    break;
                }
                else if (hard == 52)
                {
                    FALL_DELAY = 100;
                    break;
                }
            }
            system("cls");
            while (1)
            {
                if (state.dead == 1)
                {
                    break;
                }
                int restart = key_pressed();
                logic(canvas, &state);
                printCanvas(canvas, &state);
                if (restart == 114)
                {
                    break;
                }
                else if (restart == 27)
                {
                    system("cls");
                    printf("\tpress 1 to continue \n\tpress 2 to restart \n\tpress 3 to exit\n");
                    while (1)
                    {
                        int choose = key_pressed();
                        if (choose == 49)
                        {
                            break;
                        }
                        else if (choose == 50)
                        {
                            state.dead = 1;
                            break;
                        }
                        else if (choose == 51)
                        {
                            system("cls");
                            return 0;
                        }
                    }
                    system("cls");
                }
                Sleep(100);
            }
        }
        else
        {
            continue;
        }
        system("cls");
        printf("\n");
        printf("\t                  Game Over");
        printf("\n\t              Your score is %d", state.score);
        printf("\n\tPress enter to try again or press esc to exit.\n");
    }
}