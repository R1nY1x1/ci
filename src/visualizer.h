#pragma once

#include "grid.h"

struct visualizer {
  grid *g;
  int graph_n;
  double scale;
  int max_step;
  int run_by_step;
  void (*graph_init)(struct visualizer *);
  void (*draw_graph_1s)(struct visualizer *, double, double, int, int);
  void (*print_graph_value)(struct visualizer *, char *, int);
  void (*free_g)(struct visualizer *);
};
typedef struct visualizer visualizer;

void graph_init(visualizer *v);
void draw_graph_1s(visualizer *v, double pre_value, double value, int step, int num);
void print_graph_value(visualizer *v, char *str, int num);
void free_g(visualizer *v);

visualizer newVisualizer(int width, int height, int graph_n, double scale, int max_step, int run_by_step);

int clamp(int value, int min, int max);
