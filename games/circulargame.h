#ifndef CIRCULARGAME_H
#define CIRCULARGAME_H

#include "../libs/circular_linked_list.h"

void generateAnswers(char *answerListLeft, char *answerListRight, char *values);
int circularGameWon(List *listLeft, char *answerListLeft, List *listRight,  char *answerListRight);
void circularListGame();
#endif