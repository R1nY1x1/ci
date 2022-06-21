#ifndef _ANALYSER_H
#define _ANALYSER_H

#include "visualizer.h"

struct analyser {
  int vars_n;
  variable **vars;
  char **legends;
  double ***historys;
  int step;
  int max_step;
  int *steps;
  int loop;
  int max_loop;
  int fx_call;
  void (*init)(struct analyser *);
  void (*update)(struct analyser *);
  void (*save_var)(struct analyser *, char *, char *);
  void (*save_loop)(struct analyser *, char *, int);
  void (*del)(struct analyser *);
};
typedef struct analyser analyser;

void init_analyser(analyser *a);
void update_analyser(analyser *a);
void save_analyser_variable(analyser *a, char *filename, char *varname);
void save_analyser_loop(analyser *a, char *filename, int loop);
void deleteAnalyser(analyser *a);

analyser newAnalyser(variable **vars, int vars_n, int max_step, int max_loop);
#endif
