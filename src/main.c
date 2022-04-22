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
  int run_by_step = 0;
  for (int i = 0; i < argc; i++) {
    if (argv[i][0] == '-') {
      switch (argv[i][1]) {
        case 's':
          run_by_step = 1;
          break;
      }
    }
  }

  int width = 120;
  int height = 160;

  int max_step = 15;
  int scale = 2;
  visualizer v = newVisualizer(width, height, GRAPH_N, scale, max_step, run_by_step);

  int count = 0;
  while(1){
    double x[DIM] = {-4.0, -2.0};
    x[0] += count * 0.1;
    double (*dx[])(double) = {
      grad_x1,
      grad_x2,
    };
    model m = newModel(DIM, x, fx, dx);

    double h_params[] = {pow(10, -4), 0.5, 1};
    int n_params = sizeof(h_params) / sizeof(double);
    optimizer o = newOptimizer(h_params, n_params, gradient_descent);

    v.graph_init(&v);
    for (int i = 0; i < v.max_step; i++) {
      move(height / 4, 0);
      deleteln();
    }

    start_color();
    init_pair(1, COLOR_BLACK, COLOR_CYAN);

    attron(A_BOLD);
    mvprintw(height / 4, 0, "|  k |     x_1 |     x_2 |      fx |   alpha |");
    attroff(A_BOLD);
    mvprintw(height / 4 + 1, 0, "|  0 | % 7.3lf | % 7.3lf | % 7.3lf | %7.3lf |", m.x[0], m.x[1], m.fx(x), o.h_params[2]);

    for (int s = 0; s < v.max_step; s++) {
      o.update(&m, &o);

      for (int i = 0; i < DIM; i++) {
        v.draw_graph_1s(&v, m.pre_x[i], m.x[i], s, i+1);
      }
      v.draw_graph_1s(&v, m.fx(m.pre_x), m.fx(m.x), s, DIM+1);
      v.draw_graph_1s(&v, o.pre_h_params[2], o.h_params[2], s, DIM+2);

      char str[16];
      for (int i = 0; i < DIM; i++) {
        snprintf(str, sizeof(str), "x_%d=%.3lf", i+1, m.x[i]);
        v.print_graph_value(&v, str, i+1);
      }
      snprintf(str, sizeof(str), "x_%d=%.3lf", DIM+1, m.fx(m.x));
      v.print_graph_value(&v, str, DIM+1);
      snprintf(str, sizeof(str), "x_%d=%.3lf", DIM+2, o.h_params[2]);
      v.print_graph_value(&v, str, DIM+2);

      // move(height / 4 + 1, 0); deleteln();
      mvprintw(height / 4 + 1 + s + 1, 0, "| %2.d | % 7.3lf | % 7.3lf | % 7.3lf | %7.3lf |", s+1, m.x[0], m.x[1], m.fx(x), o.h_params[2]);

      if (v.run_by_step == 1) {
        renderer_update(v.g);
        switch(getch()) {
          case ':' :
            command_mode(&v);
            s = v.max_step;
            break;
          default:
            break;
        }
      }
      // if (fx(x[0], x[1]) < 0.001) { break; }
    }

    renderer_update(v.g);

    free(m.pre_x);
    free(m.d);
    free(o.pre_h_params);

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
  }

  v.free_g(&v);

  return 0;
}
