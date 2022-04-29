#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include "grid.h"
#include "renderer.h"
#include "visualizer.h"

void update_variable(variable *var) {
  var->pre_value = var->value;
  var->value = *(var->ptr);
}

variable newVariable(double *ptr, char *name) {
  variable var;
  var.ptr = ptr;
  strcpy(var.name, name);
  var.update = update_variable;
  return var;
}

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
  f->var->update(f->var);
  f->step++;
  char value_str[16];
  snprintf(value_str, sizeof(value_str), "%s=%.3lf", f->var->name, f->var->value);
  f->t.update(&(f->t), value_str, 2);
}

void plot_figure(figure *f, grid *g) {
  grid_draw_line(
    g,
    (f->step - 1) * f->width / f->max_step + f->x,
    clamp(f->height/2 * (-1)*f->var->pre_value/f->scale + f->height/2, 0, f->height) + f->y,
    f->step * f->width / f->max_step + f->x,
    clamp(f->height/2 * (-1)*f->var->value/f->scale + f->height/2, 0, f->height) + f->y
  );
  f->t.print(&(f->t));
}

figure newFigure(int x, int y, int width, int height, variable *var) {
  figure f;
  f.x = x;
  f.y = y;
  f.width = width;
  f.height = height;
  f.var = var;
  f.var->update(f.var);
  f.t = newTextbox(x+width, y, height/4, 16);
  f.step = 0;
  f.max_step = 15;
  f.scale = 2;
  f.update = update_figure;
  f.plot = plot_figure;
  return f;
}

void visualizer_init(visualizer *v) {
  for (int i = 0; i < v->vars_n; i++) {
    v->figures[i] = newFigure(0, v->g->height/(v->vars_n)*i, v->g->width, v->g->height/(v->vars_n), v->vars[i]);
  }

  v->textboxs[0] = newTextbox(0, v->g->height, 17, v->g->width/2);
  v->grid_init(v);
}

void insert_head(char *str, char *head) {
  int str_l = strlen(str);
  int head_l = strlen(head);
  for (int i = str_l; i >= 0; i--) {
    str[head_l+i] = str[i];
  }
  for (int i = 0; i < head_l; i++) {
    str[i] = head[i];
  }
}

void grid_init(visualizer *v) {
  grid_clear(v->g);
  for (int i = 0; i < (v->vars_n); i++) {
    v->figures[i].t.clear_t(&(v->figures[i].t));
  }
  v->textboxs[0].clear_t(&(v->textboxs[0]));
  grid_draw_line(v->g, 0, 0, 0, v->g->height-1);
  grid_draw_line(v->g, v->g->width - 1, 0, v->g->width - 1, v->g->height-1);
  grid_draw_line(v->g, 0, v->g->height - 1, v->g->width, v->g->height - 1);
  for (int i = 0; i < (v->vars_n); i++) {
    grid_draw_line(v->g, 0, v->g->height/(v->vars_n) * i, v->g->width, v->g->height/(v->vars_n) * i);
  }

  char str[v->textboxs[0].column_n];
  char cell[12];
  for (int i = 0; i < v->vars_n; i++) {
    snprintf(cell, sizeof(cell), "| %8s ", v->figures[i].var->name);
    for (int j = 0; j < 12; j++) {
      str[11*i+j] = cell[j];
    }
  }
  snprintf(cell, sizeof(cell), "| %8s ", "step");
  insert_head(str, cell);
  v->textboxs[0].update(&(v->textboxs[0]), str, 1);
  for (int i = 0; i < v->vars_n; i++) {
    snprintf(cell, sizeof(cell), "| %8.3lf ", v->figures[i].var->value);
    for (int j = 0; j < 12; j++) {
      str[11*i+j] = cell[j];
    }
  }
  snprintf(cell, sizeof(cell), "| %8d ", v->figures[0].step);
  insert_head(str, cell);
  v->textboxs[0].update(&(v->textboxs[0]), str, 2);
  v->textboxs[0].print(&(v->textboxs[0]));
}

void update_visualizer(visualizer *v) {
  for (int i = 0; i < (v->vars_n); i++) {
    v->figures[i].update(&(v->figures[i]));
    v->figures[i].plot(&(v->figures[i]), v->g);
  }
  char str[v->textboxs[0].column_n];
  char cell[12];
  for (int i = 0; i < v->vars_n; i++) {
    snprintf(cell, sizeof(cell), "| %8.3lf ", v->figures[i].var->value);
    for (int j = 0; j < 12; j++) {
      str[11*i+j] = cell[j];
    }
  }
  snprintf(cell, sizeof(cell), "| %8d ", v->figures[0].step);
  insert_head(str, cell);
  v->textboxs[0].update(&(v->textboxs[0]), str, v->figures[0].step+2);
  v->textboxs[0].print(&(v->textboxs[0]));
}

void free_visualizer(visualizer *v) {
  for (int i = 0; i < (v->vars_n); i++) {
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

visualizer newVisualizer(grid *g, variable *vars[], int vars_n) {
  visualizer v;
  v.g = g;
  v.vars_n = vars_n;
  v.vars = vars;
  v.figures = (figure*)malloc(sizeof(figure) * vars_n);
  v.textboxs = (textbox*)malloc(sizeof(textbox) * 1);
  v.run_by_step = 0;
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
