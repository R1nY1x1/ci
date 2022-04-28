#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include "grid.h"
#include "renderer.h"
#include "visualizer.h"

void update_textbox(textbox *t, char *text, int cursor) {
  if ((cursor-1) < t->row_n) {
    strcpy(t->texts[cursor-1], text);
  } else {
    strcpy(t->texts[(cursor-1) % t->row_n], text);
  }
}

void print_text(textbox *t) {
  for (int i = 0; i < t->row_n; i++) {
    mvprintw(t->row + i, t->column, "%s", t->texts[i]);
  }
}

void clear_textbox(textbox *t) {
  for (int i = 0; i < t->row_n; i++) {
    for (int j = 0; j < t->column_n; j++) {
      mvdelch(t->row+i, t->column);
    }
  }
}

textbox newTextbox(int x, int y, int row_n, int column_n) {
  textbox t;
  t.row = y / 4;
  t.column = x / 2;
  t.row_n = row_n;
  t.column_n = column_n;
  t.texts = (char**)malloc(sizeof(char*) * row_n);
  for (int i = 0; i < row_n; i++) {
    t.texts[i] = (char*)malloc(sizeof(char) * column_n);
  }
  t.update = update_textbox;
  t.print = print_text;
  t.clear_t = clear_textbox;
  return t;
}

void update_figure(figure *f) {
  f->pre_value = f->value;
  f->value = *(f->value_ptr);
  f->step++;
  char value_str[16];
  snprintf(value_str, sizeof(value_str), "%s=%.3lf", f->value_name, f->value);
  f->t.update(&(f->t), value_str, f->t.column_n/2-1);
}

void plot_figure(figure *f, grid *g) {
  grid_draw_line(
    g,
    (f->step - 1) * f->width / f->max_step + f->x,
    clamp(f->height/2 * (-1)*f->pre_value/f->scale + f->height/2, 0, f->height) + f->y,
    f->step * f->width / f->max_step + f->x,
    clamp(f->height/2 * (-1)*f->value/f->scale + f->height/2, 0, f->height) + f->y
  );
  f->t.print(&(f->t));
}

figure newFigure(int x, int y, int width, int height, char *value_name, double *value_ptr) {
  figure f;
  f.x = x;
  f.y = y;
  f.width = width;
  f.height = height;
  strcpy(f.value_name, value_name);
  f.value_ptr = value_ptr;
  f.value = *(value_ptr);
  f.t = newTextbox(x+width, y, height/4, 16);
  f.step = 0;
  f.max_step = 15;
  f.scale = 2;
  f.update = update_figure;
  f.plot = plot_figure;
  return f;
}

void visualizer_init(visualizer *v) {
  char value_name[8];
  for (int i = 0; i < v->m->dim; i++) {
    snprintf(value_name, sizeof(value_name), "x_%d", i+1);
    v->figures[i] = newFigure(0, v->g->height/(v->m->dim+1)*i, v->g->width, v->g->height/(v->m->dim+1), value_name, &(v->m->x[i]));
  }
  v->figures[v->m->dim] = newFigure(0, v->g->height/(v->m->dim+1)*v->m->dim, v->g->width, v->g->height/(v->m->dim+1), "fx", &(v->m->y));

  v->textboxs[0] = newTextbox(0, v->g->height, 17, v->g->width/2);
  v->grid_init(v);
}

void grid_init(visualizer *v) {
  grid_clear(v->g);
  for (int i = 0; i < (v->m->dim+1); i++) {
    v->figures[i].t.clear_t(&(v->figures[i].t));
  }
  v->textboxs[0].clear_t(&(v->textboxs[0]));
  grid_draw_line(v->g, 0, 0, 0, v->g->height-1);
  grid_draw_line(v->g, v->g->width - 1, 0, v->g->width - 1, v->g->height-1);
  grid_draw_line(v->g, 0, v->g->height - 1, v->g->width, v->g->height - 1);
  for (int i = 0; i < (v->m->dim+1); i++) {
    grid_draw_line(v->g, 0, v->g->height/(v->m->dim+1) * i, v->g->width, v->g->height/(v->m->dim+1) * i);
  }
  v->textboxs[0].update(&(v->textboxs[0]), "|  k |     x_1 |     x_2 |       fx |", 1);
  char table_str[v->textboxs[0].column_n];
  snprintf(table_str, sizeof(table_str), "|  0 | % 7.3lf | % 7.3lf | % 8.3lf |", v->m->x[0], v->m->x[1], v->m->y);
  v->textboxs[0].update(&(v->textboxs[0]), table_str, 2);
  v->textboxs[0].print(&(v->textboxs[0]));
}

void update_visualizer(visualizer *v) {
  for (int i = 0; i < (v->m->dim+1); i++) {
    v->figures[i].update(&(v->figures[i]));
    v->figures[i].plot(&(v->figures[i]), v->g);
  }
  char table_str[v->g->width/2];
  snprintf(table_str, sizeof(table_str), "| %2d | % 7.3lf | % 7.3lf | % 8.3lf |", v->figures[0].step, v->m->x[0], v->m->x[1], v->m->y);
  v->textboxs[0].update(&(v->textboxs[0]), table_str, v->figures[0].step+2);
  v->textboxs[0].print(&(v->textboxs[0]));
}

void free_visualizer(visualizer *v) {
  for (int i = 0; i < (v->m->dim+1); i++) {
    for (int j = 0; j < (v->figures[i].t.row_n); j++) {
      free(v->figures[i].t.texts[j]);
    }
    free(v->figures[i].t.texts);
  }
  free(v->textboxs[0].texts);
}

void deleteVisualizer(visualizer *v) {
  v->free(v);
  free(v->figures);
  free(v->textboxs);
}

visualizer newVisualizer(model *m, grid *g, int width, int height) {
  visualizer v;
  v.m = m;
  v.g = g;
  v.run_by_step = 0;
  v.figures = (figure*)malloc(sizeof(figure) * (v.m->dim + 1));
  v.textboxs = (textbox*)malloc(sizeof(textbox) * 1);
  v.visualizer_init = visualizer_init;
  v.grid_init = grid_init;
  v.update = update_visualizer;
  v.free = free_visualizer;
  v.del = deleteVisualizer;
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
