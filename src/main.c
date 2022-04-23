#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>
#include "grid.h"
#include "renderer.h"
#include "ci.h"
#include "visualizer.h"
#include "interpreter/repl.h"

#define SPLIT (GRAPH_N*2)
#define GRAPH_N (DIM+2)
#define DIM 2

double fx(double *x) {
  return x[0] * x[0] + 4 * x[1] * x[1];
}

double grad_x1(double x) { return 2 * x; }
double grad_x2(double x) { return 8 * x; }

void command_mode(visualizer *v) {
  char cmd[32];
  echo();
  nocbreak();
  attron(COLOR_PAIR(1));
  move(0, 0);
  insertln();
  printw("COMMAND MODE :");
  attroff(COLOR_PAIR(1));
  printw(" ");
  getstr(cmd);
  REP(v, cmd);
  move(0, 0);
  deleteln();
  noecho();
  cbreak();
}

int main(int argc, char *argv[])
{
  int width = 120;
  int height = 160;

  int max_step = 15;
  // int scale = 2;
  /*
  model m;
  visualizer v = newVisualizer(&m, width, height);

  for (int i = 0; i < argc; i++) {
    if (argv[i][0] == '-') {
      switch (argv[i][1]) {
        case 's':
          v.run_by_step = 1;
          break;
      }
    }
  }
  */

  int count = 0;
  while(1){
    double x[DIM] = {-4.0, -2.0};
    x[0] += count * 0.1;
    double (*dx[])(double) = {
      grad_x1,
      grad_x2,
    };
    model m = newModel(DIM, x, fx, dx);
    visualizer v = newVisualizer(&m, width, height);

    double h_params[] = {pow(10, -4), 0.5, 1};
    int n_params = sizeof(h_params) / sizeof(double);
    optimizer o = newOptimizer(h_params, n_params, gradient_descent);

    v.visualizer_init(&v);

    start_color();
    init_pair(1, COLOR_BLACK, COLOR_CYAN);

    mvprintw(0, 0, "run_by_step = %d", v.run_by_step);
    getchar();

    for (int s = 0; s < max_step; s++) {
      o.update(&m, &o);
      v.update(&v);
      mvprintw(1, 0, "step = %s", s);
      getchar();

      if (v.run_by_step == 1) {
        renderer_update(v.g);
        switch(getch()) {
          case ':' :
            command_mode(&v);
            s = max_step;
            break;
          default:
            break;
        }
      }
      // if (fx(x[0], x[1]) < 0.001) { break; }
    }

    switch(getch()) {
      case ':' :
        command_mode(&v);
        break;
      case KEY_LEFT:
        count--;
        break;
      case KEY_RIGHT:
        count++;
        break;
      default:
        count++;
        break;
    }

    free(m.pre_x);
    free(m.d);
    free(o.pre_h_params);
    v.free(&v);
  }

  //v.free(&v);

  return 0;
}
