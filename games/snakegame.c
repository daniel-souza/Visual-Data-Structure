#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "../libs/gfx.h"
#include "snakegame.h"

Game *game = NULL;

Body *createBody(int x, int y) {
  Body *body = malloc(sizeof(Body));
  if(!body) {
    // printf("Error: malloc failed\n");
    exit(1);
  }
  body->x = x;
  body->y = y;
  body->next = NULL;
  body->prev = NULL;
  return body;
}

void createFood(Game *game) {
  srand(time(NULL));
  while(1) {
    // printf("create food loop\n");
    game->food.x = (rand() % game->size.width) * NodeSize;
    game->food.y = (rand() % game->size.height)* NodeSize;
    Body *body = game->snake.head;
    while(body) {
      if(body->x == game->food.x && body->y == game->food.y) {
        break;
      }
      body = body->next;
    }
    // if body is null, then the food is not in the snake
    if(!body) break;
  }
}

Game *newGame(int wallCollision, int width, int height, double speed) {
  Game *game = malloc(sizeof(Game));
  if(!game) {
    // printf("Error: malloc failed\n");
    exit(1);
  }
  game->wallCollision = wallCollision;
  game->size.width = width;
  game->size.height = height;
  game->score = 0;
  game->speed = speed;
  game->snake.head = createBody(0, 0);
  game->snake.tail = game->snake.head;
  game->snake.size = 1;
  game->snake.direction = RIGHT;
  createFood(game);
  // transforme x and y to the center of the screen
  game->transformX = gfx_xsize()/2-(game->size.width/2*NodeSize);
  game->transformY = gfx_ysize()/2-(game->size.height/2*NodeSize);
  return game;
}

void addBody(Game *game, int x, int y) {
  Body *body = createBody(x, y);
  // if(!body) { printf("Error: malloc failed\n"); exit(1); }
  body->next = game->snake.head;
  game->snake.head->prev = body;
  game->snake.head = body;
  game->snake.size++;
}

int checkFood(Game *game, int x, int y) {
  if(x == game->food.x && y == game->food.y) {
    return 1;
  }
  return 0;
}

void drawMap(Game *game) {
  gfx_color(255, 255, 255);
  gfx_fill_rectangle(game->transformX, game->transformY, game->size.width * NodeSize, game->size.height * NodeSize);
}

void drawFood(Game *game) {
  gfx_color(255, 0, 0);
  gfx_fill_rectangle(game->transformX + game->food.x, game->transformY + game->food.y, NodeSize, NodeSize);
}

void drawSnake(Game *game) {
  gfx_color(0, 255, 0);
  Body *body = game->snake.head;
  while(body) {
    gfx_fill_rectangle(game->transformX + body->x, game->transformY + body->y, NodeSize, NodeSize);
    body = body->next;
  }
}

void drawScore(Game *game) {
  char score[10];
  sprintf(score, "Score: %d", game->score);
  gfx_text(30, 30, score, LARGE);
}

int wallCollision(Game *game, int *x, int *y) {
  if(*x < 0 || *x > (game->size.width-1)*NodeSize || *y < 0 || *y > (game->size.height-1)*NodeSize) {
    
    if(game->wallCollision) return 1;
    else {
      // pass the wall to the other side
      if(*x < 0) *x = (game->size.width-1)*NodeSize;
      else if(*x > (game->size.width-1)*NodeSize) *x = 0;
      else if(*y < 0) *y = (game->size.height-1)*NodeSize;
      else if(*y > (game->size.height-1)*NodeSize) *y = 0;
    }
  }
  return 0;
}

int bodyCollision(Game *game, int x, int y) {
  
  Body *body = game->snake.head;
  while(body) {
    if(body->x == x && body->y == y) {
      return 1;
    }
    body = body->next;
  }
  return 0;
}

int render_game(char c) {
    // printf("direction: %d :: %d\n", game->snake.direction, RIGHT);
    gfx_clear();
    gfx_color(200, 100, 100);
    gfx_fill_rectangle(0, 0, gfx_xsize(), gfx_ysize());
    gfx_color(0, 0, 0);
    int x = game->snake.head->x, y = game->snake.head->y;
    switch(c) {
      case 'w': case '\x52': if(game->snake.direction != DOWN) game->snake.direction = UP;
        break;
      case 's': case '\x54': if(game->snake.direction != UP) game->snake.direction = DOWN;
        break;
      case 'a': case '\x51': if(game->snake.direction != RIGHT) game->snake.direction = LEFT;
        break;
      case 'd': case '\x53': if(game->snake.direction != LEFT) game->snake.direction = RIGHT;
        break;
    }

    switch(game->snake.direction) {
      case UP: y -= NodeSize;
        break;
      case DOWN: y += NodeSize;
        break;
      case LEFT: x -= NodeSize;
        break;
      case RIGHT: x += NodeSize;
        break;
    }
    
    drawMap(game);
    drawFood(game);
    drawSnake(game);
    drawScore(game);
    
    if(wallCollision(game, &x, &y) || bodyCollision(game, x, y)) {
      gfx_text_utf8(gfx_xsize()/2-50, game->transformY-50, "Game Over", LARGE);
      while(1)
        switch(gfx_wait()) {
          case '\x52': case 'w':
          case '\x54': case 's': 
          case '\x51': case 'a':
          case '\x53': case 'd':
            break;
          default: return 1;
        }
    }
    moveSnake(game, x, y);
    gfx_flush();
    // sleep for 0.5 seconds
    usleep(1000000/(game->speed));
    return 0;
}

void snake_game() {
  
  char *options[3][4] = {
		{"Speed", "slow", "medium", "fast"},
    {"Wall Collision", "Off", "On"},
    {"Map Size:", "10x10", "20x20", "30x30"}
	};
  // printf("after options\n");

  int speed_index = 1;
  int wall_index = 2;
  int map_index = 1;
  int menu_index = 0;
	char c;
	while(1) {
		gfx_clear();
		gfx_color(255,255,0);
    gfx_text(30, 30, "Game Options", LARGE);

		for(int i = 0; i < 3; i++) {
        // printf("i: %d\n", i);
				if(i == menu_index) {
					gfx_color(255,255,0);
					gfx_text(30, 75 + 30*i, options[i][0], LARGE);
          int j = 1;
          while(options[i][j] && j < 4) {
            if(i == 0 && j == speed_index) gfx_color(255,255,0);
            else if(i == 1 && j == wall_index) gfx_color(255,255,0);
            else if(i == 2 && j == map_index) gfx_color(255,255,0);
            else gfx_color(0,255,0);
            gfx_text(30 + 200 + (j-1)*100, 75 + 30*i, options[i][j], LARGE);
            j++;
          }
				}
				else {
					gfx_color(0,255,0);
					gfx_text(30, 75 + 30*i, options[i][0], MEDIUM);
          int j = 1;
          while(options[i][j] && j < 4) {
            if(i == 0 && j == speed_index) gfx_color(255,255,0);
            else if(i == 1 && j == wall_index) gfx_color(255,255,0);
            else if(i == 2 && j == map_index) gfx_color(255,255,0);
            else gfx_color(0,255,0);
            gfx_text(30 + 200 + (j-1)*75, 75 + 30*i, options[i][j], MEDIUM);
            j++;
          }
				}
				gfx_color(255,255,0);
				gfx_text(30, 75 + 30*4 + 25, "Press Enter to Start", LARGE);
		}
    // printf("after for loop\n");
		// Wait for the user to press a character.
		c = gfx_wait();
		if(c=='\x1b') break; // 0x1b = ESC
		switch(c) {
			case '\x52': // Up
				// printf("Up\n");
				menu_index = menu_index > 0 ? menu_index-1 : 3;
				break;
			case '\x54': // Down	
				// printf("Down - menu_index: %d\n", menu_index);
				menu_index = menu_index < 3 ? menu_index+1 : 0;
				// printf("After assignment - menu_index: %d\n", menu_index);
				break;
			case '\x51': // Left
        if(menu_index == 0 && speed_index > 1) speed_index--;
        else if(menu_index == 1 && wall_index > 1) wall_index--;
        else if(menu_index == 2 && map_index > 1) map_index--;
				break;
			case '\x53': // Right
        if(menu_index == 0 && speed_index < 3) speed_index++;
        else if(menu_index == 1 && wall_index < 2) wall_index++;
        else if(menu_index == 2 && map_index < 3) map_index++;
				break;
			default:
				break;
		}
    if(c == '\xd') {
      game = newGame(wall_index-1, map_index*10, map_index*10, speed_index*3);
      gfx_iddle(render_game, '\x1b');
      free(game);
    }
	}
  
  
  
}

void moveSnake(Game *game, int x, int y) {
  // printf("Moving snake\n");
  // printf("snake x: %d, snake y: %d\n", game->snake.head->x, game->snake.head->y);
  // printf("x: %d, y: %d\n", x, y);
  // printf("food x: %d, food y: %d\n", game->food.x, game->food.y);

  if(checkFood(game, x, y)) {
    addBody(game, x, y);
    game->score++;
    game->speed += .200;
    createFood(game);
    return;
  }

  Body *body = game->snake.tail;
  while(body->prev) {
    body->x = body->prev->x;
    body->y = body->prev->y;
    body = body->prev;
  }
  body->x = x;
  body->y = y;
}
