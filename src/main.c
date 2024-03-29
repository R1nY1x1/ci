#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <ncurses.h>
#include <sys/stat.h>
#include "../dotdotdot/src/grid.h"
#include "../dotdotdot/src/renderer.h"
#include "ci.h"
#include "method.h"
#include "visualizer.h"
#include "analyser.h"
#include "interpreter/repl.h"
#include "util.h"

double fx(double *x, int dim) {
  double sum = 0;
  for (int i = 0; i < dim; i++) {
    //sum += x[i] * x[i];
    sum += x[i] * x[i] - 10 * cos(2 * M_PI * x[i]) + 10;
  }
  return sum;
}

double grad_fx(double *x, int dim) {
  return 2 * x[dim];
  //return 2 * x[dim] + 20 * M_PI * sin(2 * M_PI * x[dim]);
}

double *x_ptr;
double *d_ptr;

void run_command(visualizer *v);
void command_mode(visualizer *v);

int main(int argc, char *argv[])
{
  srand((unsigned int)time(NULL));

  // Define Model
  int dim = 2;
  double x[dim];
  for (int i = 0; i < dim; i++) {
    x[i] = ((double)rand())/((double)RAND_MAX+1.0) * (5.12 - (-5.12)) + (-5.12);
  }
  double (*dx[dim])(double *, int);
  for (int i = 0; i < dim; i++) {
    dx[i] = grad_fx;
  }
  model m = newModel(dim, x, fx, dx);
  x_ptr = m.x;
  d_ptr = m.d;

  // Define Method
  int M = 30;
  m.x_candidates = (double**)malloc(sizeof(double*) * M);
  m.x_candidates_best = (double**)malloc(sizeof(double*) * M);
  m.v_candidates = (double**)malloc(sizeof(double*) * M);
  for (int i = 0; i < M; i++) {
    m.x_candidates[i] = (double*)malloc(sizeof(double) * dim);
    m.x_candidates_best[i] = (double*)malloc(sizeof(double) * dim);
    m.v_candidates[i] = (double*)malloc(sizeof(double) * dim);
    for (int j = 0; j < dim; j++) {
      m.x_candidates[i][j] = ((double)rand())/((double)RAND_MAX+1.0) * (5.12 - (-5.12)) + (-5.12);
      m.x_candidates_best[i][j] = m.x_candidates[i][j];
      m.v_candidates[i][j] = 0;
    }
    if (m.fx(m.x_candidates_best[i], m.dim) < m.fx(m.x_best, m.dim)) {
      for (int j = 0; j < dim; j++) {
        m.x_best[j] = m.x_candidates_best[i][j];
      }
    }
  }

  int idx_low = 0;
  int idx_second = 0;
  int idx_high = 0;
  for (int i = 0; i < M; i++) {
    if (fx(m.x_candidates[i], m.dim) < fx(m.x_candidates[idx_low], m.dim)) {
      idx_low = i;
    } else if (fx(m.x_candidates[i], m.dim) > fx(m.x_candidates[idx_high], m.dim)) {
      idx_high = i;
    } else if (fx(m.x_candidates[i], m.dim) > fx(m.x_candidates[idx_second], m.dim)) {
      idx_second = i;
    }
  } 
  double h_params[] = {M, 1.0, 2, 0.5, idx_low, idx_second, idx_high};
  int params_n = sizeof(h_params) / sizeof(double);
  method mthd = newMethod(h_params, params_n, nelder_mead);
  /*
  double h_params[] = {M, 0.729, 1.494};
  int params_n = sizeof(h_params) / sizeof(double);
  method mthd = newMethod(h_params, params_n, particale_swarm_optimization);
  double h_params[] = {M, 0.5, 0.9};
  int params_n = sizeof(h_params) / sizeof(double);
  method mthd = newMethod(h_params, params_n, differential_evolution_rand1bin);
  method mthd = newMethod(h_params, params_n, differential_evolution_best1bin);
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
  int max_step = 100;
  double scale = 10;
  //variable var_x_1 = newVariable(&m.x_best[0], "x_1");
  //variable var_x_2 = newVariable(&m.x_best[1], "x_2");
  variable var_x_1 = newVariable(&m.x_candidates[0][0], "x_1");
  variable var_x_2 = newVariable(&m.x_candidates[0][1], "x_2");
  variable var_fx = newVariable(&m.y, "fx");
  variable *vars[] = {&var_x_1, &var_x_2, &var_fx};
  int vars_n = sizeof(vars) / sizeof(variable*);
  visualizer v = newVisualizer(g, vars, vars_n);
  for (int i = 0; i < (v.vars_n); i++) {
    v.figures[i].max_step = max_step;
    v.figures[i].scale = scale;
  }

  // Define Analyser
  int max_loop = 100;
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
        case 'c':
          run_command(&v);
          break;
      }
    }
  }

  // main loop : could enter COMMANDMODE and Change Initial x
  int earlystop_cnt = 0;
  int stop_step_sum = 0;
  for (int l = 0; l < max_loop; l++) {
    // TODO Model.init and Method.init
    for (int i = 0; i < sizeof(x)/sizeof(double); i++) {
      m.x[i] = x[i];
      m.x_best[i] = x[i];
    }
    for (int i = 0; i < M; i++) {
      for (int j = 0; j < dim; j++) {
        //m.x_candidates[i][j] = ((double)rand())/((double)RAND_MAX+1.0) * (5.12 - (-5.12)) + (-5.12);
        m.x_candidates_best[i][j] = m.x_candidates[i][j];
        m.v_candidates[i][j] = 0;
      }
      if (m.fx(m.x_candidates_best[i], m.dim) < m.fx(m.x_best, m.dim)) {
        for (int j = 0; j < dim; j++) {
          m.x_best[j] = m.x_candidates_best[i][j];
        }
      }
    }
    m.y = m.fx(m.x, m.dim);
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
      if (m.fx(m.x_best, m.dim) < pow(10, -5)) {
        earlystop_cnt++;
        break;
      }
    }
    renderer_update(v.g);
    stop_step_sum += a.step;

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
    for (int i = 0; i < dim; i++) {
      x[i] = ((double)rand())/((double)RAND_MAX+1.0) * (5.12 - (-5.12)) + (-5.12);
    }
    for (int i = 0; i < M; i++) {
      for (int j = 0; j < dim; j++) {
        m.x_candidates[i][j] = ((double)rand())/((double)RAND_MAX+1.0) * (5.12 - (-5.12)) + (-5.12);
      }
    }
  }

  printQuit(stdscr);
  renderer_update(v.g);
  a.save_var(&a, "data.csv", var_fx.name);
  m.del(&m);
  mthd.del(&mthd);
  o.del(&o);
  v.del(&v);
  a.del(&a);
  renderer_free();
  grid_free(v.g);
  printf("success: %d\n", earlystop_cnt);
  printf("step sum: %d\n", stop_step_sum);
  printf("average stop t: %lf\n", (double)stop_step_sum/(double)(max_loop-1));
  return 0;
}

void run_command(visualizer *v) {
  char *filename = ".circ";
  FILE *fp = fopen(filename, "r");
  struct stat fstat;
  stat(filename, &fstat);
  char *cmd = malloc(fstat.st_size);
  while (fscanf(fp, "%[^\n] ", cmd) != EOF) {
    REP(v, cmd);
  }
  fclose(fp);
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
