#include <math.h>
#include <stdio.h>
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
  m.fx_ret = fx(x);
  m.d = (double*)malloc(sizeof(double) * m.dim);
  m.fx = fx;
  m.dx = dx;
  m.grad_norm = grad_norm;
  m.grad = grad;
  m.del = deleteModel;
  return m;
}

void deleteOptimizer(optimizer *o) {
  free(o->h_params);
}

optimizer newOptimizer(double* h_params, int params_n, void method(model *, optimizer *)) {
  optimizer o;
  o.h_params = (double*)malloc(sizeof(double) * params_n);
  for (int i = 0; i < params_n; i++) {
    o.h_params[i] = h_params[i];
  }
  o.update = method;
  o.del = deleteOptimizer;
  return o;
}

int armijo_rule(model *m, optimizer *o) {
  double epi_x[m->dim];
  double temp = 0;
  for (int i = 0; i < m->dim; i++) {
    epi_x[i] = m->x[i] + o->h_params[2] * m->d[i];
    temp += m->d[i] * m->dx[i](m->x[i]);
  }
  return (m->fx(epi_x) <= (m->fx(m->x) + (o->h_params[0] * o->h_params[2] * temp)));
}

void gradient_descent(model *m, optimizer *o){
  /*
  h_params[0] : xi
  h_params[1] : rho
  h_params[2] : alpha
  */
  m->grad(m);

  for (int i = 0; i < 8; i++) {
    o->h_params[2] = pow(o->h_params[1], i);
    if (armijo_rule(m, o)) {
      break;
    }
  }

  for (int i = 0; i < m->dim; i++) {
    m->x[i] = m->x[i] + o->h_params[2] * m->d[i];
  }
  m->fx_ret = m->fx(m->x);
}
