#include <math.h>
#include <stdlib.h>
#include "ci.h"

double grad_norm(model *m) {
  double d_norm = 0.0;
  for (int i = 0; i < m->dim; i++) {
    d_norm += pow(m->dx[i](m->x[i]), 2);
  }
  return sqrt(d_norm);
}

void grad(model *m) {
  for (int i = 0; i < m->dim; i++) {
    m->d[i] = -1 * m->dx[i](m->x[i]) / m->grad_norm(m);
  }
}

void deleteModel(model *m) {
  free(m->x);
  free(m->d);
}

model newModel(int dim, double *x, double (*fx)(double *), double (**dx)(double)) {
  model m;
  m.dim = dim;
  m.x = (double*)malloc(sizeof(double) * m.dim);
  for (int i = 0; i < dim; i++){
    m.x[i] = x[i];
  }
  m.y = fx(x);
  m.d = (double*)malloc(sizeof(double) * m.dim);
  m.fx = fx;
  m.dx = dx;
  m.grad_norm = grad_norm;
  m.grad = grad;
  m.del = deleteModel;
  return m;
}

void update_method(model *m, method *mthd) {
  mthd->function(m , mthd);
}

void deleteMethod(method *mthd) {
  free(mthd->h_params);
}

method newMethod(double* h_params, int params_n, void function(model *, method *)) {
  method mthd;
  mthd.h_params = (double*)malloc(sizeof(double) * params_n);
  for (int i = 0; i < params_n; i++) {
    mthd.h_params[i] = h_params[i];
  }
  mthd.function = function;
  mthd.update = update_method;
  return mthd;
}

void update_optimizer(model *m, optimizer *o) {
  o->mthd->update(m, o->mthd);
}

void deleteOptimizer(optimizer *o) {
}

optimizer newOptimizer(method *mthd) {
  optimizer o;
  o.mthd = mthd;
  o.update = update_optimizer;
  o.del = deleteOptimizer;
  return o;
}
