#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <ncurses.h>
#include "grid.h"
#include "renderer.h"
#include "ci.h"
#include "visualizer.h"
#include "interpreter/repl.h"

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

  grid *g = grid_new(width, height);
  renderer_new(g);
  noecho();
  cbreak();
  keypad(stdscr, TRUE);

  start_color();
  init_pair(1, COLOR_BLACK, COLOR_CYAN);


  double x[DIM] = {-4.0, -2.0};
  double (*dx[])(double) = {
    grad_x1,
    grad_x2,
  };
  model m = newModel(DIM, x, fx, dx);

  double h_params[] = {pow(10, -4), 0.5, 1};
  int params_n = sizeof(h_params) / sizeof(double);
  optimizer o = newOptimizer(h_params, params_n, gradient_descent);

  int max_step = 16;
  double scale = 0.2;
  visualizer v = newVisualizer(&m, g, width, height);
  v.visualizer_init(&v);
  for (int i = 0; i < (v.m->dim+1); i++) {
    v.figures[i].max_step = max_step;
    v.figures[i].scale = scale;
  }

  for (int i = 0; i < argc; i++) {
    if (argv[i][0] == '-') {
      switch (argv[i][1]) {
        case 's':
          v.run_by_step = 1;
          break;
      }
    }
  }

  while(1){
    x[0] += 0.1;
    m = newModel(DIM, x, fx, dx);
    v.m = &m;

    v.visualizer_init(&v);
    for (int i = 0; i < (v.m->dim+1); i++) {
      v.figures[i].max_step = max_step;
      v.figures[i].scale = scale;
    }

    for (int s = 0; s < max_step; s++) {
      o.update(&m, &o);
      v.update(&v);

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
    renderer_update(g);

    switch(getch()) {
      case ':' :
        command_mode(&v);
        break;
      case 'q':
        m.del(&m);
        o.del(&o);
        v.del(&v);
        renderer_free();
        grid_free(g);
        return 0;
      case KEY_LEFT:
        break;
      case KEY_RIGHT:
        break;
      default:
        break;
    }

    v.free(&v);
  }

  return 0;
}
