#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "analyser.h"

void init_analyser(analyser *a) {
  a->step = 0;
  a->loop++;
  a->update(a);
}

void update_analyser(analyser *a) {
  for (int i = 0; i < a->vars_n; i++) {
    a->historys[a->loop][i][a->step] = a->vars[i]->value;
  }
  a->step++;
}

void save_analyser_variable(analyser *a, char *filename, char *varname) {
  FILE *fp = fopen(filename, "w");
  for (int i = 0; i < a->vars_n; i++) {
    if (strcmp(a->vars[i]->name, varname) == 0) {
      for (int l = 0; l < a->loop; l++) {
        for (int s = 0; s < a->max_step; s++) {
          fprintf(fp, "%.3lf, ", a->historys[l][i][s]);
        }
        fprintf(fp, "\n");
      }
    }
  }
  fclose(fp);
}

void save_analyser_loop(analyser *a, char *filename, int loop) {
  FILE *fp = fopen(filename, "w");
  for (int i = 0; i < a->vars_n; i++) {
    fprintf(fp, "%s, ", a->legends[i]);
  }
  fprintf(fp, "\n");
  for (int s = 0; s < a->max_step; s++) {
    for (int i = 0; i < a->vars_n; i++) {
      fprintf(fp, "%.3lf, ", a->historys[loop][i][s]);
    }
    fprintf(fp, "\n");
  }
  fclose(fp);
}

void deleteAnalyser(analyser *a) {
  free(a->vars);
  for (int i = 0; i < a->vars_n; i++) {
    free(a->legends[i]);
  }
  free(a->legends);
  for (int l = 0; l < a->max_loop; l++) {
    for (int i = 0; i < a->vars_n; i++) {
      free(a->historys[l][i]);
    }
    free(a->historys[l]);
  }
  free(a->historys);
}

analyser newAnalyser(variable **vars, int vars_n, int max_step, int max_loop) {
  analyser a;
  a.vars_n = vars_n;
  a.vars = (variable**)malloc(sizeof(variable*) * vars_n);
  for (int i = 0; i < vars_n; i++) {
    a.vars[i] = vars[i];
  }
  a.legends = (char**)malloc(sizeof(char*) * vars_n);
  for (int i = 0; i < vars_n; i++) {
    a.legends[i] = (char*)malloc(sizeof(char) * 8);
    strcpy(a.legends[i], vars[i]->name);
  }
  // bad
  a.historys = (double***)malloc(sizeof(double**) * max_loop);
  for (int i = 0; i < max_loop; i++) {
    a.historys[i] = (double**)malloc(sizeof(double*) * vars_n);
    for (int j = 0; j < vars_n; j++) {
      a.historys[i][j] = (double*)malloc(sizeof(double) * max_step);
    }
  }
  a.step = 0;
  a.max_step = max_step;
  a.loop = -1;
  a.max_loop = max_loop;
  a.init = init_analyser;
  a.update = update_analyser;
  a.save_var = save_analyser_variable;
  a.save_loop = save_analyser_loop;
  a.del = deleteAnalyser;
  return a;
}
