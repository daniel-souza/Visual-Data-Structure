#include <stdlib.h>     /* srand, rand, malloc */
#include <time.h>       /* time */

#include "circulargame.h"

void generateAnswers(char *answerListLeft, char *answerListRight, char *values) {
	char selectedValues[10] = "          ";
	int index = 0;
	while(index < 10) {
		int random = rand() % 10;
		if(values[random] != ' ') {
			selectedValues[index] = values[random];
			values[random] = ' ';
			index++;
		}
	}
	for(int i = 0; i < 5; i++) {
		answerListLeft[i] = selectedValues[i];
		answerListRight[i] = selectedValues[i+5];
	}
}

int circularGameWon(List *listLeft, char *answerListLeft, List *listRight,  char *answerListRight) {
	int won = 1;
	Node *nodeLeft = listLeft->head;
	for(int i = 0; i < 5; i++) {
		if(nodeLeft->value[0] != answerListLeft[i]) {
			won = 0;
			break;
		}
		nodeLeft = nodeLeft->next;
	}
	Node *nodeRight = listRight->head;
	for(int i = 0; i < 5; i++) {
		if(nodeRight->value[0] != answerListRight[i]) {
			won = 0;
			break;
		}
		nodeRight = nodeRight->next;
	}
	return won;
}

void circularListGame() {
	srand (time(NULL));

	char answerListLeft[5];
	char answerListRight[5];
	char values[10] = "12345ABCDE";
	generateAnswers(answerListLeft, answerListRight, values);
	 

	List *list1 = list_init();
	list_add(list1, "1");
	list_add(list1, "2");
	list_add(list1, "3");
	list_add(list1, "4");
	list_add(list1, "5");

	List *list2 = list_init();
	list_add(list2, "A");
	list_add(list2, "B");
	list_add(list2, "C");
	list_add(list2, "D");
	list_add(list2, "E");

	
	char c;
	while(1) {
		gfx_clear();
		gfx_fill_left_polygon_list(100, 125, 40, (Color) {0, 0, 255}, list1, answerListLeft);
		gfx_fill_right_polygon_list(250, 125, 40, (Color) {255, 0, 0}, list2, answerListRight);
		if(circularGameWon(list1, answerListLeft, list2, answerListRight)) {
			gfx_color(0,255,0);
			gfx_text(30, 30, "You won!", LARGE);
			gfx_wait();
			break;
		}
		// Wait for the user to press a character.
		c = gfx_wait();
		if(c=='\x1b') { // 0x1b = ESC
			delete_list(list1);
			delete_list(list2);
			break;
		}
		switch(c) {
			case '\x51': // Left
				list_rotate(list1);
				break;
			case '\x52': // Up
			case '\x54': // Down
				list_swap(list1, list2);
				break;
			case '\x53': // Right
				list_rotate(list2);
				break;
			default:
				break;
		}
	}
}