#ifndef _UTIL_H
#define _UTIL_H

#include <ncurses.h>

void printLogo(WINDOW *win);
void printQuit(WINDOW *win);

void insert_head(char *str, char *head);
int clamp(int value, int min, int max);
#endif
