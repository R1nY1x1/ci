#pragma once

#include "ci.h"
#include "grid.h"

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
  double value;
  double pre_value;
  char value_name[8];
  double *value_ptr;
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

figure newFigure(int x, int y, int width, int height, char *value_name, double *value_ptr);

struct visualizer {
  model *m;
  grid *g;
  int run_by_step;
  figure *figures;
  textbox *textboxs;
  void (*visualizer_init)(struct visualizer *);
  void (*grid_init)(struct visualizer *);
  void (*update)(struct visualizer *);
  void (*free)(struct visualizer *);
  void (*del)(struct visualizer *);
};
typedef struct visualizer visualizer;

void visualizer_init(visualizer *v);
void grid_init(visualizer *v);
void update_visualizer(visualizer *v);
void free_visualizer(visualizer *v);
void deleteVisualizer(visualizer *v);

visualizer newVisualizer(model *m, grid *g, int width, int height);

int clamp(int value, int min, int max);
