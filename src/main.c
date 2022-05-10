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
#include "analyser.h"
#include "interpreter/repl.h"
#include "util.h"

double fx(double *x) {
  return fabs(x[0] * x[0] * x[0] + 2 * x[0] * x[0] - 5 * x[0] + 6);
}

double *x_ptr;
double *d_ptr;

void command_mode(visualizer *v);

int main(int argc, char *argv[])
{
  srand((unsigned int)time(NULL));

  /*
  for (int i = 0; i < 100; i++) {
    printf("%lf\n", rand_normal(0, 1));
  }
  return 0;
  */

  // Define Model
  double x[] = {((double)rand())/((double)RAND_MAX+1.0) * (5 - (-5)) + (-5)};
  //double x[] = {-5.0};
  double (*dx[])(double) = {
  };
  model m = newModel(sizeof(x)/sizeof(double), x, fx, dx);
  x_ptr = m.x;
  d_ptr = m.d;

  // Define Method
  /*
  double h_params[] = {0.01, 0};
  int params_n = sizeof(h_params) / sizeof(double);
  method mthd = newMethod(h_params, params_n, liner_method);
  */
  double h_params[] = {0, 1};
  int params_n = sizeof(h_params) / sizeof(double);
  method mthd = newMethod(h_params, params_n, hill_climbing);

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
  int max_step = 1000;
  double scale = 5;
  variable var_x_1 = newVariable(&m.x[0], "x_1");
  variable var_fx = newVariable(&m.y, "fx");
  variable *vars[] = {&var_x_1, &var_fx};
  int vars_n = sizeof(vars) / sizeof(variable*);
  visualizer v = newVisualizer(g, vars, vars_n);
  for (int i = 0; i < (v.vars_n); i++) {
    v.figures[i].max_step = max_step;
    v.figures[i].scale = scale;
  }

  // Define Analyser
  int max_loop = 101;
  int run_by_loop = 1;
  analyser a = newAnalyser(vars, vars_n, max_step, max_loop);

  // argument parser
  for (int i = 0; i < argc; i++) {
    if (argv[i][0] == '-') {
      switch (argv[i][1]) {
        case 's':
          v.run_by_step = 1;
          break;
        case 'l':
          run_by_loop = 0;
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

  // main loop : could enter COMMANDMODE and Change Initial x
  int earlystop_cnt = 0;
  for (int l = 0; l < max_loop; l++) {
    for (int i = 0; i < sizeof(x)/sizeof(double); i++) {
      m.x[i] = x[i];
    }
    m.y = m.fx(m.x);
    for (int i = 0; i< sizeof(h_params)/sizeof(double); i++) {
      mthd.h_params[i] = h_params[i];
    }

    v.visualizer_init(&v);
    a.init(&a);

    // optimization loop
    for (int s = 0; s < v.figures[0].max_step; s++) {
      o.update(&m, &o);
      v.update(&v);
      a.update(&a);

      if (v.run_by_step == 1) {
        renderer_update(v.g);
        switch(getch()) {
          default:
            break;
        }
      }
      if (m.fx(m.x) < pow(10, -5)) {
        earlystop_cnt++;
        break;
      }
    }
    renderer_update(v.g);

    if (run_by_loop) {
      switch(getch()) {
        case ':' :
          command_mode(&v);
          break;
        case 'q':
          printQuit(stdscr);
          renderer_update(v.g);
          a.save_var(&a, "date.csv", var_fx.name);
          //a.save_loop(&a, "date.csv", 1);
          m.del(&m);
          mthd.del(&mthd);
          o.del(&o);
          v.del(&v);
          a.del(&a);
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
    x[0] = ((double)rand())/((double)RAND_MAX+1.0) * (5 - (-5)) + (-5);
  }

  printQuit(stdscr);
  renderer_update(v.g);
  a.save_var(&a, "date.csv", var_fx.name);
  m.del(&m);
  mthd.del(&mthd);
  o.del(&o);
  v.del(&v);
  a.del(&a);
  renderer_free();
  grid_free(v.g);
  printf("success: %d\n", earlystop_cnt);
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
