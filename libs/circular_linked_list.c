#include <stdlib.h>
#include <math.h>

#include "circular_linked_list.h"

List *list_init() {
  List *list = malloc(sizeof(List));
  list->head = NULL;
  list->size = 0;
  return list;
}

Node *new_node(char *value) {
  Node *node = malloc(sizeof(Node));
  if(node != NULL) {
    node->value = value;
    node->next = NULL;
  }
  return node;
}

void list_swap(List *l1, List *l2) {
  char *value = l1->head->value;
  l1->head->value = l2->head->value;
  l2->head->value = value;
}

void list_add(List *list, char *value) {
  Node *node = new_node(value);
  if(node == NULL) {
    // printf("Error: malloc failed\n");
    return;
  }
  if (list->head == NULL) {
    list->head = node;
  } else {
    Node *aux = list->head;
    while (aux->next != list->head) {
      aux = aux->next;
    }
    aux->next = node;
  }
  node->next = list->head;
  list->size++;
}

void list_add_node_start(List *list, Node *node) {
  if(!list || !node) return;
  node->next = list->head ? list->head : node;
  list->head = node;
  list->size++;
}

void list_add_node_end(List *list, Node *node) {
  if(!list || !node) return;
  if(!list->head) {
    list->head = node;
    node->next = node;
  } else {
    Node *aux = list->head;
    for(int i = 1; i < list->size;i++) {
      aux = aux->next;
    }
    aux->next = node;
    node->next = list->head;
  }
  list->size++;
}

void list_delete_node_start(List *list) {
  if(!list || !list->head) return;
  if(list->size == 1) {
    free(list->head);
    list->head = NULL;
  } else if(list->size > 1) {
    Node *next = list->head->next;
    free(list->head);
    list->head = next;
  }
  list->size--;
}

void list_delete_node_end(List *list) {
  if(!list || !list->head) return;
  if(list->size == 1) {
    free(list->head);
    list->head = NULL;
  } else if(list->size > 1) {
    Node *aux = list->head;
    for(int i = 1; i < list->size-1;i++) {
      aux = aux->next;
    }
    free(aux->next);
    aux->next = list->head;
  }
  list->size--;
}

void list_free(List *list) {
  if(!list) return;
  Node *node = list->head;
  while (list->size > 0) {
    Node *next = node->next;
    free(node);
    node = next;
    list->size--;
  }
}

void delete_list(List *list) {
  if(!list) return;
  Node *node = list->head;
  while (list->size > 0) {
    Node *next = node->next;
    free(node);
    node = next;
    list->size--;
  }
  free(list);
}

void list_print(List *list) {
  Node *node = list->head;
  int i = 0;
  while (i < list->size) {
    node = node->next; i++;
  }
}

void list_rotate(List *list) {
  list->head = list->head->next;
}

// for drawing with gfx (X11) --------------------------------------------------------------------------------------------

// Linked List Drawing Functions
void gfx_draw_node_value(int x, int y, Node *n, int size) {
  gfx_color(0, 0, 255);
  gfx_fill_rectangle(x-(size/2.), y-(size/2.), size, size);
  // gfx_circle(x-(10/4.), y-10, 10);
  gfx_color(255, 255, 255);
  gfx_text(x-5, y+5, n->value, MEDIUM);
}

void gfx_draw_node_next(int x, int y, int size) {
  gfx_color(255, 0, 0);
  gfx_fill_rectangle(x-(size/2.), y-(size/2.), size, size);
  gfx_color(255, 255, 255);
  gfx_fill_circle(x-(size/4.), y-(size/4), size/2.);
}

void gfx_draw_node(int x, int y, Node *n, int size) {
  gfx_draw_node_value(x, y, n, size);
  gfx_draw_node_next(x+size, y, size);
  gfx_draw_arrow(x+size, y, x+(3*size/2.) + 10, y);
}

void gfx_draw_list(int x, int y, List *list, int size, int gap) {
  Node *n = list->head;
  for(int i = 0; i < list->size; i++) {
    gfx_draw_node_value(x, y, n, size);
    gfx_draw_node_next(x+size, y, size);
    gfx_color(255,255, 255);
    gfx_draw_arrow(x+size, y, x+(3*size/2.)+gap, y);
    x += 2*size + gap;
    n = n->next;
  }
}

void gfx_draw_arrow(int x1, int y1, int x2, int y2) {
  gfx_line(x1, y1, x2, y2);
  gfx_line(x2, y2, x2-5, y2-5);
  gfx_line(x2, y2, x2-5, y2+5);
}

// Circular Linked List Drawing Functions
void gfx_fill_left_polygon_list(int cx, int cy, int radius, Color bg, List *list, char *answer) {
  Node *node = list->head;
  int x;
  int y;
  int i = 0;
  float twopi = 2 * 3.1415;
  for(float theta = 0; theta < twopi; theta += twopi/list->size) {
    x = cx + radius * cos(theta);
    y = cy - radius * sin(theta);
    node->value[0] == answer[i] ? gfx_color(255, 255, 0) : gfx_color(bg.r, bg.g, bg.b);
    gfx_fill_circle(x-(radius/2.), y-(radius/2.), radius);
    gfx_color(255, 255, 255);
    node->value[0] == answer[i] ? gfx_color(255, 0, 0) : gfx_color(255, 255, 0);
    node->value[0] == answer[i] ? gfx_text(x-5, y+10, node->value, LARGE) : gfx_text(x-5, y+5, node->value, MEDIUM);
    node = node->next;
    i++;
    // printf("%s\n", node->value);
  }
}

void gfx_fill_right_polygon_list(int cx, int cy, int radius, Color bg, List *list, char *answer) {
  Node *node = list->head;
  int x;
  int y;
  int i = 0;
  float twopi = 2 * 3.1415;
  for(float theta = 0; theta < twopi; theta += twopi/list->size) {
    x = cx - radius * cos(theta);
    y = cy - radius * sin(theta);
    node->value[0] == answer[i] ? gfx_color(255, 255, 0) : gfx_color(bg.r, bg.g, bg.b);
    gfx_fill_circle(x-(radius/2.), y-(radius/2.), radius);
    gfx_color(255, 255, 255);
    node->value[0] == answer[i] ? gfx_color(255, 0, 0) : gfx_color(255, 255, 0);
    node->value[0] == answer[i] ? gfx_text(x-5, y+10, node->value, LARGE) : gfx_text(x-5, y+5, node->value, MEDIUM);
    node = node->next;
    i++;
  }
}