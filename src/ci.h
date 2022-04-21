#pragma once

struct model {
  int dim;
  double *x;
  double *pre_x;
  double *d;
  double (*fx)(double *);
  double (**dx)(double);
  double (*grad_norm)(struct model *);
  void (*grad)(struct model *);
};
typedef struct model model;

double grad_norm(model *m);
void grad(model *m);

model newModel(int dim, double *x, double (*fx)(double *), double (**dx)(double));

struct optimizer {
  double *h_params;
  double *pre_h_params;
  void (*update)(model *, struct optimizer *);
};
typedef struct optimizer optimizer;

optimizer newOptimizer(double* h_params, int n_params, void method(model *, optimizer *));

int armijo_rule(model *m, optimizer *o);
void gradient_descent(model *m, optimizer *o);
