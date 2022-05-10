#ifndef _VISUALIZER_H
#define _VISUALIZER_H

#include "grid.h"

struct variable {
  double pre_value;
  double value;
  double *ptr;
  char name[8];
  void (*update)(struct variable *);
};
typedef struct variable variable;

void update_variable(variable *var);

variable newVariable(double *ptr, char *name);

struct textbox {
  int row;
  int column;
  int row_n;
  int column_n;
  char **texts;
  void (*update)(struct textbox *, char *, int);
  void (*print)(struct textbox *);
  void (*clear_t)(struct textbox *);
};
typedef struct textbox textbox;

void update_textbox(textbox *t, char *text, int cur_num);
void print_text(textbox *t);
void clear_textbox(textbox *t);

textbox newTextbox(int row, int column, int row_n, int column_n);

struct figure {
  int x;
  int y;
  int width;
  int height;
  variable *var;
  textbox t;
  int step;
  int max_step;
  double scale;
  void (*update)(struct figure *);
  void (*plot)(struct figure *, grid *);
};
typedef struct figure figure;

void update_figure(figure *f);
void plot_figure(figure *f, grid *g);

figure newFigure(int x, int y, int width, int height, variable *var);

struct visualizer {
  grid *g;
  int vars_n;
  variable **vars;
  figure *figures;
  textbox *textboxs;
  int run_by_step;
  void (*visualizer_init)(struct visualizer *);
  void (*grid_init)(struct visualizer *);
  void (*update)(struct visualizer *);
  void (*del)(struct visualizer *);
};
typedef struct visualizer visualizer;

void visualizer_init(visualizer *v);
void grid_init(visualizer *v);
void update_visualizer(visualizer *v);
void deleteVisualizer(visualizer *v);

visualizer newVisualizer(grid *g, variable *vars[], int vars_n);

int fit_figure_x(int step, figure *f);
int fit_figure_y(double value, figure *f);
#endif
