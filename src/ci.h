#ifndef _CI_H
#define _CI_H

struct model {
  int dim;
  double *x;
  double y;
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

struct method {
  double *h_params;
  struct method *sub_mthd;
  void (*function)(model *, struct method *);
  void (*update)(model *, struct method *);
  void (*del)(struct method *);
};
typedef struct method method;

void update_method(model *m, method *mthd);
void deleteMethod(method *mthd);

method newMethod(double* h_params, int params_n, void function(model *, method *));

struct optimizer {
  method *mthd;
  void (*update)(model *, struct optimizer *);
  void (*del)(struct optimizer *);
};
typedef struct optimizer optimizer;

void update_optimizer(model *m, optimizer *o);
void deleteOptimizer(optimizer *o);

optimizer newOptimizer(method *mthd);
#endif
