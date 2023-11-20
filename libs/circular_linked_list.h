#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include "gfx.h"

/* Linked List Data Structure 
 * - with descriptiors: head, tail and size 
 */
typedef struct node {
  char *value;
  struct node *next;
} Node;

typedef struct {
  Node *head;
  Node *tail;
  int size;
} List;

List *list_init();
Node *new_node(char *value);
void list_add(List *list, char *value);
void list_add_node_start(List *list, Node *n);
void list_add_node_end(List *list, Node *n);
void list_delete_node_start(List *list);
void list_delete_node_end(List *list);
void list_free(List *list);
void delete_list(List *list);
void list_rotate(List *list);
void list_print(List *list);
void list_swap(List *l1, List *l2);

// for drawing with gfx (X11)

// Linked List Drawing Functions
void gfx_draw_list(int x, int y, List *list, int size, int gap);
void gfx_draw_node_value(int x, int y, Node *n, int size);
void gfx_draw_node_next(int x, int y, int size);
void gfx_draw_node(int x, int y, Node *n, int size);
void gfx_draw_arrow(int x1, int y1, int x2, int y2);

// Circular Linked List Drawing Functions
void gfx_fill_right_polygon_list(int cx, int cy, int radius, Color bg, List *list, char *answer);
void gfx_fill_left_polygon_list(int cx, int cy, int radius, Color bg, List *list, char *answer);
#endif