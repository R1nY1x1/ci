#pragma once

struct model {
  int dim;
  double *x;
  double fx_ret;
  double *d;
  double (*fx)(double *);
  double (**dx)(double);
  double (*grad_norm)(struct model *);
  void (*grad)(struct model *);
  void (*del)(struct model *);
};
typedef struct model model;

double grad_norm(model *m);
void grad(model *m);
void deleteModel(model *m);

model newModel(int dim, double *x, double (*fx)(double *), double (**dx)(double));

struct optimizer {
  double *h_params;
  void (*update)(model *, struct optimizer *);
  void (*del)(struct optimizer *);
};
typedef struct optimizer optimizer;

optimizer newOptimizer(double* h_params, int n_params, void method(model *, optimizer *));
void deleteOptimizer(optimizer *o);

int armijo_rule(model *m, optimizer *o);
void gradient_descent(model *m, optimizer *o);
