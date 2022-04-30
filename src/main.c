#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <ncurses.h>
#include <sys/stat.h>
#include "grid.h"
#include "renderer.h"
#include "ci.h"
#include "method.h"
#include "visualizer.h"
#include "interpreter/repl.h"
#include "util.h"

double fx(double *x) {
  //return fabs(x[0] * x[0] * x[0] + 2 * x[0] * x[0] - 5 * x[0] + 6);
  return x[0] * x[0] + 4 * x[1] * x[1];
}

double grad_x1(double x) { return 2 * x; }
double grad_x2(double x) { return 8 * x; }

double *x_ptr;
double *d_ptr;

void command_mode(visualizer *v);

int main(int argc, char *argv[])
{
  srand((unsigned int)time(NULL));

  // Define Model
  double x[] = {-4.0, -2.0};
  //double x[] = {((double)rand())/((double)RAND_MAX+1.0) * (5 - (-5)) + (-5)};
  double (*dx[])(double) = {
    grad_x1,
    grad_x2,
  };
  model m = newModel(sizeof(x)/sizeof(double), x, fx, dx);
  x_ptr = m.x;
  d_ptr = m.d;

  // Define Method
  //double h_params[] = {1, pow(10, -4), 0.5};
  double h_params[] = {NAN};
  int params_n = sizeof(h_params) / sizeof(double);
  method mthd = newMethod(h_params, params_n, gradient_descent);
  double sub_h_params[] = {0, pow(10, 2), pow(10, -2), pow(10, -3)};
  int sub_params_n = sizeof(sub_h_params) / sizeof(double);
  method sub_mthd = newMethod(sub_h_params, sub_params_n, bisection_method);
  mthd.sub_mthd = &sub_mthd;
  /*
  double h_params[] = {0.1, 1};
  int params_n = sizeof(h_params) / sizeof(double);
  method mthd = newMethod(h_params, params_n, liner_method);
  double h_params[] = {0, 1};
  int params_n = sizeof(h_params) / sizeof(double);
  method mthd = newMethod(h_params, params_n, hill_climbing);
  */

  // Define Optimizer : optimizer is wrapper of updatation model by method
  optimizer o = newOptimizer(&mthd);

  // Definition Grid
  int width = 120;
  int height = 160;
  grid *g = grid_new(width, height);
  renderer_new(g);
  noecho();
  cbreak();
  keypad(stdscr, TRUE);
  start_color();
  init_pair(1, COLOR_BLACK, COLOR_CYAN);
  init_pair(2, COLOR_RED, COLOR_BLACK);

  // Define Visualizer
  int max_step = 15;
  double scale = 1;
  variable var_x_1 = newVariable(&m.x[0], "x_1");
  variable var_x_2 = newVariable(&m.x[1], "x_2");
  variable var_fx = newVariable(&m.y, "fx");
  variable var_alpha = newVariable(&mthd.h_params[0], "alpha");
  variable *vars[] = {&var_x_1, &var_x_2, &var_fx, &var_alpha};
  int vars_n = sizeof(vars) / sizeof(variable*);
  visualizer v = newVisualizer(g, vars, vars_n);
  for (int i = 0; i < (v.vars_n); i++) {
    v.figures[i].max_step = max_step;
    v.figures[i].scale = scale;
  }

  // argument parser
  for (int i = 0; i < argc; i++) {
    if (argv[i][0] == '-') {
      switch (argv[i][1]) {
        case 's':
          v.run_by_step = 1;
          break;
      }
    }
  }

  // Run Command
  char *filename = ".circ";
  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    return -1;
  }
  struct stat fstat;
  stat(filename, &fstat);
  char *cmd = malloc(fstat.st_size);
  while (fscanf(fp, "%[^\n] ", cmd) != EOF) {
    REP(&v, cmd);
  }
  fclose(fp);

  // main loop : could enter COMMANDMODE
  while(1){
    //x[0] = ((double)rand())/((double)RAND_MAX+1.0) * (5 - (-5)) + (-5);
    for (int i = 0; i < sizeof(x)/sizeof(double); i++) {
      m.x[i] = x[i];
    }
    m.y = m.fx(m.x);
    for (int i = 0; i< sizeof(h_params)/sizeof(double); i++) {
      mthd.h_params[i] = h_params[i];
    }

    v.visualizer_init(&v);

    // optimization loop
    for (int s = 0; s < v.figures[0].max_step; s++) {
      o.update(&m, &o);
      v.update(&v);

      if (v.run_by_step == 1) {
        renderer_update(v.g);
        switch(getch()) {
          default:
            break;
        }
      }
      if (m.fx(m.x) < pow(10, -5)) {
        break;
      }
    }
    renderer_update(v.g);

    switch(getch()) {
      case ':' :
        command_mode(&v);
        break;
      case 'q':
        printQuit(stdscr);
        renderer_update(v.g);
        m.del(&m);
        mthd.del(&mthd);
        o.del(&o);
        v.del(&v);
        renderer_free();
        grid_free(v.g);
        return 0;
      case KEY_LEFT:
        x[0] -= 0.1;
        break;
      case KEY_RIGHT:
        x[0] += 0.1;
        break;
      default:
        x[0] += 0.1;
        break;
    }
  }

  return 0;
}

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
