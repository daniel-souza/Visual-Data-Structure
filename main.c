/*
A simple example of using the gfx library.
CSE 20211
9/7/2011
by Prof. Thain
*/

#include <stdio.h>
#include <stdlib.h>     /* srand, rand, malloc */
#include <string.h>
#include <locale.h>

#include "libs/gfx.h"
#include "libs/pdi_img.h"
#include "libs/circular_linked_list.h"

#include "games/snakegame.h"
#include "games/circulargame.h"

t_image authorImg;
t_image logoIESBImg;
t_image logoIDPImg;

void draw_image(int x, int y, t_image img) {
	x -= img.width/2; y -= img.height/2;
	for(int i = 0; i < img.width; i++) {
    for(int j = 0; j < img.height; j++) {
      gfx_color(img.data.rgb[i + (j * img.width)].r, img.data.rgb[i + (j * img.width)].g, img.data.rgb[i + (j * img.width)].b);
			gfx_point(i+x, j+y);
		}
	}
}

void print_games_menu() {
	char *options[] = {
		"Find the Pattern (Circular List)",
		"Snake Game (Linked List)"
	};
	
	char c;
	int index = 0;
	while(1) {
		gfx_clear();
		for(int i = 0; i < 2; i++) {
				if(i == index) {
					gfx_color(255,255,0);
					gfx_text(30, 30 + 30*i, options[i], LARGE);
				}
				else {
					gfx_color(0,255,0);
					gfx_text(30, 30 + 30*i, options[i], MEDIUM);
				}
		}
		// Wait for the user to press a character.
		c = gfx_wait();
		if(c=='\x1b') break; // 0x1b = ESC
		switch(c) {
			case '\x51': // Left
				// printf("Left\n");
				break;
			case '\x52': // Up
				// printf("Up\n");
				index = index > 0 ? index-1 : 0;
				break;
			case '\x53': // Right
				// printf("Right\n");
				break;
			case '\x54': // Down	
				// printf("Down - index: %d\n", index);
				index = index < 1 ? index+1 : 0;
				// printf("After assignment - index: %d\n", index);
				break;
			case '\xd': // Enter (Return)
				// printf("Enter\n");
				switch(index) {
					case 0:
						circularListGame();
						break;
				case 1:
						snake_game();
						break;
					default:
						break;
				}
				break;
			default:
				break;
		}
	}
}

void print_credits() {
	gfx_clear();
	char *credits[] = {
		"Made by:",
		"Prof. Daniel Souza (© 2023)",
		"Centro Universitário IESB",
		"Instituto Brasileiro de Ensino, Desenvolvimento e Pesquisa (IDP)"
	};
	draw_image(gfx_xsize()/2., gfx_ysize()/2., authorImg);
	draw_image(gfx_xsize()/2. -250, gfx_ysize()/2., logoIESBImg);
	draw_image(gfx_xsize()/2. +250, gfx_ysize()/2., logoIDPImg);
	int y = gfx_ysize()/2. + authorImg.height/2.;
	gfx_color(0,0,128);
	gfx_fill_rectangle(0, 0, gfx_xsize(), gfx_ysize()/2. - (authorImg.height/2.));
	gfx_fill_rectangle(0, y, gfx_xsize(), gfx_ysize());
	for(int i = 0; i < 4; i++) {
		// navy color
		// fill all the rectangle with the navy color on top
		// fill all the rectangle with the navy color on botton
		y += 45;
		gfx_color(255,150,150);
		gfx_text_utf8(gfx_xsize()/2.-(strlen(credits[i])/2*12), y, credits[i], LARGE);
	}
	gfx_wait();
}

void insert_linked_list_menu(List *list, char position) {
	char *value = malloc(sizeof(char)*3); value[0] = '_'; value[1] = '_'; value[2] = '\0';
	Node *node = new_node(value);
	int chars = 0;
	while(1) {
		gfx_clear();
		gfx_color(255,255,0);
		list->size > 0
			? gfx_draw_list(30, 30, list, 30, 10)
			: gfx_text(30, 45, "Empty list", LARGE);
		gfx_draw_node(30, 90, node, 30);
		char c = gfx_wait();
		// printf("%x\n", c);
		if(c=='\x1b') {
			free(node->value);
			free(node);
			break; // 0x1b = ESC
		}
		if(c=='\x8') { // 0x7f = Backspace
			if(chars-1 >= 0 && node->value[chars-1] != '_') {
				node->value[--chars] = '_';
			}
		} else if(c=='\xd') { // 0x1b = Enter (Return)
			// printf("Enter\n");
			// printf("chars: %d\n", chars);
			if(chars == 0) continue;
			node->value[chars] = '\0';
			if(position == 's') list_add_node_start(list, node);
			else if(position == 'e') list_add_node_end(list, node);
			break;
		} else if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) {
			if(chars == 2) continue;
			node->value[chars] = c;
			chars++;
		}
	}
}

void linked_list_menu() {
	char *options[] = {
		"1 - Insert at begginning",
		"2 - Insert at endding",
		"3 - Delete at begginning",
		"4 - Delete at endding",
	};

	List *list = list_init(); // Create a new list
	int index = 0;
	while(1) {
		gfx_clear();
		gfx_color(255,255,0);
		list->size > 0 ?  gfx_draw_list(30, 30, list, 30, 10) : gfx_text(30, 45, "Empty list", LARGE);
		for(int i = 0; i < 4; i++) {
			if(i == index) {
				gfx_color(255,255,0);
				gfx_text(30, 90 + 30*i, options[i], LARGE);
			}
			else {
				gfx_color(0,255,0);
				gfx_text(30, 90 + 30*i, options[i], MEDIUM);
			}
		}
		// Wait for the user to press a character.
		char c = gfx_wait();
		// printf("%x\n", c);
		if(c=='\x1b') { // 0x1b = ESC // zerar lista
			list_free(list);
			break;
		}
		switch(c) {
			case '\x51': // Left
				// printf("Left\n");
				break;
			case '\x52': // Up
				// printf("Up\n");
				index = index > 0 ? index-1 : 0;
				break;
			case '\x53': // Right
				// printf("Right\n");
				break;
			case '\x54': // Down
				// printf("Down\n");
				index = index < 3 ? index+1 : 3;
				break;
			case '\xd': // Enter (Return)
				// printf("Enter\n");
				switch(index) {
					case 0: {
						insert_linked_list_menu(list, 's');
						break;
					} case 1:
						insert_linked_list_menu(list, 'e');
						break;
					case 2:
						list_delete_node_start(list);
						break;
					case 3:
						list_delete_node_end(list);
						break;
					default:
						break;
				}
		}
	}
}

void print_data_structures_menu() {
	char *options[] = {
		"1 - Linked List",
		"2 - Stack",
		"3 - Queue",
		"4 - Circular Linked List",
		"5 - Binary Tree",
		"6 - Binary Search Tree",
		"7 - AVL Tree",
		"8 - Red-Black Tree",
		"9 - Hash Table",
		"10 - Graph"
	};
	int index = 0;
	while(1) {
		gfx_clear();
		for(int i = 0; i < 10; i++) {
			if(i == index) {
				gfx_color(255,255,0);
				gfx_text(30, 30 + 30*i, options[i], LARGE);
			}
			else {
				gfx_color(0,255,0);
				gfx_text(30, 30 + 30*i, options[i], MEDIUM);
			}
		}
		// Wait for the user to press a character.
		char c = gfx_wait();
		// printf("%x\n", c);
		if(c=='\x1b') break; // 0x1b = ESC
		switch(c) {
			case '\x51': // Left
				// printf("Left\n");
				break;
			case '\x52': // Up
				// printf("Up\n");
				index = index > 0 ? index-1 : 0;
				break;
			case '\x53': // Right
				// printf("Right\n");
				break;
			case '\x54': // Down	
				// printf("Down\n");
				index = index < 9 ? index+1 : 9;
				break;
			case '\xd': // Enter (Return)
				// printf("Enter\n");
				switch (index) {
				case 0:
					linked_list_menu();
					break;
				default:
					break;
				}
				break;
			default:
				break;
		}
	}
}

void print_main_menu() {
	char *options[] = {
		"1 - Learn Data Structures",
		"2 - Data Structures' Games",
		"3 - Credits"
	};
	int index = 0;
	while(1) {
		gfx_clear();
		for(int i = 0; i < 3; i++) {
			if(i == index) {
				gfx_color(255,255,0);
				gfx_text(30, 30 + 30*i, options[i], LARGE);
			}
			else {
				gfx_color(0,255,0);
				gfx_text(30, 30 + 30*i, options[i], MEDIUM);
			}
		}
		// Wait for the user to press a character.
		char c = gfx_wait();
		// printf("%x\n", c);

		// Quit if it is the letter q.
		if(c=='\x1b') break; // 0x1b = ESC
		switch(c) {
			case '\x51': // Left
				// printf("Left\n");
				break;
			case '\x52': // Up
				// printf("Up\n");
				index = index > 0 ? index-1 : 0;
				break;
			case '\x53': // Right
				// printf("Right\n");
				break;
			case '\x54': // Down	
				// printf("Down\n");
				index = index < 2 ? index+1 : 2;
				break;
			case '\xd': // Enter (Return)
				// printf("Enter\n");
				switch(index) {
					case 0:
						print_data_structures_menu();
						break;
					case 1:
					  print_games_menu();
						break;
					case 2:
						print_credits();
						break;
					default:
						gfx_flush();
						break;
				}
				break;
			default:
				break;
		}
	}
}

void scene1() {
	gfx_color(255,0,0);
	gfx_line(100,100,200,100);
	gfx_line(200,100,150,150);
	gfx_line(150,150,100,100);
}

void scene2() {
	gfx_color(0,255,0);
	gfx_line(100,100,200,100);
	gfx_line(200,100,150,150);
	gfx_line(150,150,100,100);
}

void scene3() {
	gfx_color(0,0,255);
	gfx_line(100,100,200,100);
	gfx_line(200,100,150,150);
	gfx_line(150,150,100,100);
}



int main()
{
	// char * locale = setlocale(LC_ALL, "pt_BR.UTF-8");
	// char * locale = setlocale(LC_ALL, "POSIX");
	// char *locale = setlocale(LC_ALL, "en_US.UTF-8");
	// printf("locale: %s\n", locale);
	setlocale(LC_ALL, "en_US.UTF-8");
	authorImg = pdi_openImg("./assets/author.ppm");
	logoIESBImg = pdi_openImg("./assets/logoIESB.ppm");
	logoIDPImg = pdi_openImg("./assets/logoIDP.ppm");

	int xsize = 800;
	int ysize = 600;

	// Open a new window for drawing.
	gfx_open(xsize,ysize,"Data Structures - Prof. Daniel Souza (IESB/IDP)");

	// Set the current drawing color to green.
	// gfx_color(255,255,0);
	// gfx_bgcolor(255,0,0);

	print_main_menu();
	
	return 0;
}
