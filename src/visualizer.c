#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "../dotdotdot/src/grid.h"
#include "../dotdotdot/src/renderer.h"
#include "visualizer.h"
#include "util.h"

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
      mvaddch(t->row+i, t->column+j, ' ');
    }
    t->texts[i][0] = '\0';
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
  char str[16];
  snprintf(str, sizeof(str), "[ %.3lf]", f->scale);
  f->t.update(&(f->t), str, 1);
  snprintf(str, sizeof(str), " name : %s", f->var->name);
  f->t.update(&(f->t), str, 3);
  snprintf(str, sizeof(str), " value: % 5.3lf", f->var->value);
  f->t.update(&(f->t), str, 4);
  snprintf(str, sizeof(str), " diff : %+5.3lf", f->var->value - f->var->pre_value);
  f->t.update(&(f->t), str, 5);
  snprintf(str, sizeof(str), "[-%.3lf]", f->scale);
  f->t.update(&(f->t), str, f->t.row_n);
}

void plot_figure(figure *f, grid *g) {
  grid_draw_line(
    g,
    fit_figure_x(f->step - 1, f),
    fit_figure_y(f->var->pre_value, f),
    fit_figure_x(f->step, f),
    fit_figure_y(f->var->value, f)
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
  grid_init(v);

  char str[v->textboxs[1].column_n];
  char cell[12];
  for (int i = 0; i < v->vars_n; i++) {
    snprintf(cell, sizeof(cell), "| %8s ", v->figures[i].var->name);
    for (int j = 0; j < 12; j++) {
      str[11*i+j] = cell[j];
    }
  }
  snprintf(cell, sizeof(cell), "| %8s ", "step");
  insert_head(str, cell);
  v->textboxs[1].update(&(v->textboxs[1]), str, 1);
  for (int i = 0; i < v->vars_n; i++) {
    snprintf(cell, sizeof(cell), "| %8.3lf ", v->figures[i].var->value);
    for (int j = 0; j < 12; j++) {
      str[11*i+j] = cell[j];
    }
  }
  snprintf(cell, sizeof(cell), "| %8d ", v->figures[0].step);
  insert_head(str, cell);
  v->textboxs[1].update(&(v->textboxs[1]), str, 2);
  v->textboxs[1].print(&(v->textboxs[1]));
}

void grid_init(visualizer *v) {
  grid_clear(v->g);
  for (int i = 0; i < (v->vars_n); i++) {
    v->figures[i].step = 0;
    v->figures[i].t.clear_t(&(v->figures[i].t));
    v->figures[i].var->update(v->figures[i].var);
  }
  attron(COLOR_PAIR(2) | A_BOLD);
  v->textboxs[0].print(&(v->textboxs[0]));
  v->textboxs[1].clear_t(&(v->textboxs[1]));
  renderer_update(v->g);
  attroff(COLOR_PAIR(2) | A_BOLD);

  grid_draw_line(v->g, 0, 0, 0, v->g->height-1);
  grid_draw_line(v->g, v->g->width - 1, 0, v->g->width - 1, v->g->height-1);
  grid_draw_line(v->g, 0, v->g->height - 1, v->g->width, v->g->height - 1);
  for (int i = 0; i < (v->vars_n); i++) {
    grid_draw_line(v->g, 0, v->g->height/(v->vars_n) * i, v->g->width, v->g->height/(v->vars_n) * i);
  }
}

void update_visualizer(visualizer *v) {
  for (int i = 0; i < (v->vars_n); i++) {
    v->figures[i].update(&(v->figures[i]));
    v->figures[i].plot(&(v->figures[i]), v->g);
  }
  char str[v->textboxs[1].column_n];
  char cell[12];
  for (int i = 0; i < v->vars_n; i++) {
    snprintf(cell, sizeof(cell), "| %8.3lf ", v->figures[i].var->value);
    for (int j = 0; j < 12; j++) {
      str[11*i+j] = cell[j];
    }
  }
  snprintf(cell, sizeof(cell), "| %8d ", v->figures[0].step);
  insert_head(str, cell);
  v->textboxs[1].update(&(v->textboxs[1]), str, v->figures[0].step+2);
  v->textboxs[1].print(&(v->textboxs[1]));
}

void deleteVisualizer(visualizer *v) {
  for (int i = 0; i < (v->vars_n); i++) {
    for (int j = 0; j < (v->figures[i].t.row_n); j++) {
      free(v->figures[i].t.texts[j]);
    }
    free(v->figures[i].t.texts);
  }
  free(v->textboxs[0].texts);
  free(v->textboxs[1].texts);
  free(v->figures);
  free(v->textboxs);
}

visualizer newVisualizer(grid *g, variable *vars[], int vars_n) {
  visualizer v;
  v.g = g;
  v.vars_n = vars_n;
  v.vars = vars;
  v.figures = (figure*)malloc(sizeof(figure) * vars_n);
  v.textboxs = (textbox*)malloc(sizeof(textbox) * 2);
  for (int i = 0; i < v.vars_n; i++) {
    v.figures[i] = newFigure(0, v.g->height/(v.vars_n)*i, v.g->width, v.g->height/(v.vars_n), v.vars[i]);
  }
  v.textboxs[0] = newTextbox(0, 0, v.g->height/4, v.g->width/2);
  v.textboxs[1] = newTextbox(0, v.g->height, 17, v.g->width/2);
  v.run_by_step = 0;
  v.visualizer_init = visualizer_init;
  v.grid_init = grid_init;
  v.update = update_visualizer;
  v.del = deleteVisualizer;
  return v;
}

int fit_figure_x(int step, figure *f) {
  int x;
  x = step * f->width / f->max_step + f->x;
  return x;
}

int fit_figure_y(double value, figure *f) {
  int y;
  y = clamp(f->height/2 * (-1)*value/f->scale + f->height/2, 0, f->height) + f->y;
  return y;
}
