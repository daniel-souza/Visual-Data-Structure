#ifndef SNAKEGAME_H
#define SNAKEGAME_H

#define NodeSize 10

typedef enum direction {
  UP,
  DOWN,
  LEFT,
  RIGHT
} Direction;

typedef struct body {
  int x;
  int y;
  struct body *next;
  struct body *prev;
} Body;

typedef struct {
  Body *head;
  Body *tail;
  int size;
  Direction direction;
} Snake;

typedef struct {
  int x;
  int y;
} Food;

typedef struct {
  int width;
  int height;
} Size;

typedef struct {
  Size size;
  int score;
  Food food;
  Snake snake;
  int wallCollision;
  double speed;
  int transformX;
  int transformY;
} Game;

Body *createBody(int x, int y);
void createFood(Game *game);
Game *newGame(int wallColisition, int width, int height, double speed);
void addBody(Game *game, int x, int y);
int checkFood(Game *game, int x, int y);
void drawMap(Game *game);
void drawFood(Game *game);
void drawSnake(Game *game);
void drawScore(Game *game);
int checkCollision(Game *game, int x, int y);
void moveSnake(Game *game, int x, int y);
void snake_game();

#endif