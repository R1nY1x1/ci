#include <math.h>
#include <stdlib.h>
#include "ci.h"

double grad_norm(model *m) {
  double d_norm = 0.0;
  for (int i = 0; i < m->dim; i++) {
    d_norm += pow(m->dx[i](m->x, i), 2);
  }
  return sqrt(d_norm);
}

void grad(model *m) {
  for (int i = 0; i < m->dim; i++) {
    m->d[i] = -1 * m->dx[i](m->x, i) / m->grad_norm(m);
  }
}

void grad_approx(model *m) {
  double tmp[m->dim];
  double delta = pow(10, -3);
  for (int i = 0; i < m->dim; i++) {
    tmp[i] = m->x[i] + delta;
  }
  m->d[0] = (m->fx(m->x, m->dim) - m->fx(tmp, m->dim)) / delta;
}

void deleteModel(model *m) {
  free(m->x);
  free(m->x_best);
  free(m->d);
}

model newModel(int dim, double *x, double (*fx)(double *, int), double (**dx)(double *, int)) {
  model m;
  m.dim = dim;
  m.x = (double*)malloc(sizeof(double) * m.dim);
  m.x_best = (double*)malloc(sizeof(double) * m.dim);
  for (int i = 0; i < dim; i++){
    m.x[i] = x[i];
    m.x_best[i] = x[i];
  }
  m.y = fx(x, dim);
  m.d = (double*)malloc(sizeof(double) * m.dim);
  m.fx = fx;
  m.dx = dx;
  m.grad_norm = grad_norm;
  m.grad = grad;
  m.grad_approx = grad_approx;
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
  mthd.del = deleteMethod;
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
