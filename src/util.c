#include <string.h>
#include <ncurses.h>

void printLogo(WINDOW *win) {
  char str[8][56] = {
    "              __           1           __           __ ",
    "     __      /_/          x           /_/ _   __   /_/ ",
    "    / /___  __  ______   1  __   __  __  | | / /  __   ",
    "   / ____/ / / / __  /  Y  / /  / / / /   |/ /   / /   ",
    "  / /     / / / / / /     / /__/ / / /   / /|   / /    ",
    " /_/     /_/ /_/ /_/  n  /____  / /_/  /_/ |_| /_/     ",
    "                     1      _/ /                       ",
    "                    R      /__/                        ",
  };
  int x = getmaxx(win);
  int y = getmaxy(win);

  for (int i = 0; i < 8; i++) {
    attron(A_BOLD);
    mvprintw(y/2 - (6/2) + i, x/2 - (56/2), str[i]);
    attroff(A_BOLD);
  }
}

void printQuit(WINDOW *win) {
  char str[6][32] = {
    "                     __    __  ",
    "   _______  __  __  /_/ __/ /_ ",
    "  / __   / / / / / __  /_  __/ ",
    " / /_/  / / /_/ / / /   / /_   ",
    "/____  / /_____/ /_/   /___/   ",
    "    /_/                        ",
  };
  int x = getmaxx(win);
  int y = getmaxy(win);

  for (int i = 0; i < 6; i++) {
    attron(A_BOLD | A_BLINK);
    mvprintw(y/2 - (6/2) + i, x/2 - (32/2), str[i]);
    attroff(A_BOLD | A_BLINK);
  }
}

void insert_head(char *str, char *head) {
  int str_l = strlen(str);
  int head_l = strlen(head);
  for (int i = str_l; i >= 0; i--) {
    str[head_l+i] = str[i];
  }
  for (int i = 0; i < head_l; i++) {
    str[i] = head[i];
  }
}

int clamp(int value, int min, int max)
{
    if (value < min) {
        return min;
    }
    else if (value > max) {
        return max;
    }
    return value;
}
