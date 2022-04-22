#include <unistd.h>
#include <ncurses.h>
#include "grid.h"
#include "renderer.h"
#include "visualizer.h"

void graph_init(visualizer *v) {
  grid_clear(v->g);
  grid_draw_line(v->g, 0, 0, 0, v->g->height-1);
  grid_draw_line(v->g, v->g->width - 1, 0, v->g->width - 1, v->g->height-1);
  grid_draw_line(v->g, 0, v->g->height - 1, v->g->width, v->g->height - 1);
  for (int i = 0; i < v->graph_n; i++) {
    grid_draw_line(v->g, 0, v->g->height / v->graph_n * i, v->g->width, v->g->height / v->graph_n * i);
  }
}

void draw_graph_1s(visualizer *v, double pre_value, double value, int step, int num) {
  pre_value /= v->scale;
  value /= v->scale;
  grid_draw_line(
    v->g,
    step * v->g->width / v->max_step,
    clamp(
      v->g->height/(v->graph_n*2) * (-1)*pre_value + v->g->height/(v->graph_n*2)*(num*2-1),
      v->g->height/(v->graph_n*2)*((num-1)*2),
      v->g->height/(v->graph_n*2)*num*2),
    (step + 1) * v->g->width / v->max_step,
    clamp(
      v->g->height/(v->graph_n*2) * (-1)*value + v->g->height/(v->graph_n*2)*(num*2-1),
      v->g->height/(v->graph_n*2)*((num-1)*2),
      v->g->height/(v->graph_n*2)*num*2)
  );
}

void print_graph_value(visualizer *v, char *str, int num) {
  mvprintw(v->g->height / (v->graph_n * 2) / 4 * (num * 2 - 1) - 1, v->g->width / 2, str);
}

void free_g(visualizer *v) {
  renderer_free();
  grid_free(v->g);
}

visualizer newVisualizer(int width, int height, int graph_n, double scale, int max_step, int run_by_step) {
  visualizer v;
  grid *g = grid_new(width, height);
  renderer_new(g);
  v.g = g;
  noecho();
  cbreak();
  keypad(stdscr, TRUE);
  v.graph_n = graph_n;
  v.scale = scale;
  v.max_step = max_step;
  v.run_by_step = run_by_step;
  v.graph_init = graph_init;
  v.draw_graph_1s = draw_graph_1s;
  v.print_graph_value = print_graph_value;
  v.free_g = free_g;
  return v;
}

int clamp(int value, int min, int max)
{
    if (value < min) {
        return min;
    }
    else if (value > max) {
        return max;
    }
    return value;
}
