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

model newModel(int dim, double *x, double (*fx)(double *), double (**dx)(double)) {
  model m;
  m.dim = dim;
  m.x = x;
  m.pre_x = (double*)malloc(sizeof(double) * m.dim);
  m.d = (double*)malloc(sizeof(double) * m.dim);
  m.fx = fx;
  m.dx = dx;
  m.grad_norm = grad_norm;
  m.grad = grad;
  return m;
}

optimizer newOptimizer(double* h_params, int n_params, void method(model *, optimizer *)) {
  optimizer o;
  o.h_params = h_params;
  o.pre_h_params = (double*)malloc(sizeof(double) * n_params);
  o.update = method;
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

  o->pre_h_params[2] = o->h_params[2];
  for (int i = 0; i < 8; i++) {
    o->h_params[2] = pow(o->h_params[1], i);
    if (armijo_rule(m, o)) {
      break;
    }
  }

  for (int i = 0; i < m->dim; i++) {
    m->pre_x[i] = m->x[i];
    m->x[i] = m->x[i] + o->h_params[2] * m->d[i];
  }
}
