#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <ncurses.h>
#include "ci.h"

void armijo_rule(model *m, method *mthd) {
  /*
  x[0] : alpha -> step_width
  h_params[1] : xi
  h_params[2] : rho
  */
  double epi_x[m->dim];
  double temp = 0;
  for (int i = 0; i < m->dim; i++) {
    epi_x[i] = m->x[i] + mthd->h_params[2] * m->d[i];
    temp += m->d[i] * m->dx[i](m->x[i]);
  }
  for (int i = 0; i < 8; i++) {
    mthd->h_params[0] = pow(mthd->h_params[1], i);
    if (m->fx(epi_x) <= (m->fx(m->x) + (mthd->h_params[0] * mthd->h_params[2] * temp))) {
      break;
    }
  }
}

void bisection_method(model *m, method *mthd) {
  /*
  x[0] : alpha = step_width
  h_params[0] : a
  h_params[1] : b
  h_params[2] : epsi
  h_params[3] : delta
  */
  double a = mthd->h_params[0];
  double b = mthd->h_params[1];
  double u[1];
  double v[1];
  m->x[0] = (a + b) / 2;
  for (int i = 0; i < 16; i++) {
    u[0] = m->x[0] - mthd->h_params[3];
    v[0] = m->x[0] + mthd->h_params[3];
    if (m->fx(u) >= m->fx(v)) {
      a = u[0];
    } else if (m->fx(u) < m->fx(v)) {
      b = v[0];
    }
    m->x[0] = (a + b) / 2;
    if (fabs(b - a) < mthd->h_params[2]) {
      break;
    }
  }
}

double sub_fx(double *x) {
  extern double *x_ptr;
  extern double *d_ptr;
  double temp[2];
  for (int i = 0; i < 2; i++) {
    temp[i] = x_ptr[i] + x[0] * d_ptr[i];
  }
  return temp[0] * temp[0] + 4 * temp[1] * temp[1];
}

void gradient_descent(model *m, method *mthd){
  /*
  h_params[0] : alpha = step_width
  */
  m->grad(m);

  double x[] = {1};
  double (*dx[])(double) = {};
  model alpha_model = newModel(sizeof(x)/sizeof(double), x, sub_fx, dx);
  mthd->sub_mthd->update(&alpha_model, mthd->sub_mthd);
  mthd->h_params[0] = alpha_model.x[0];

  for (int i = 0; i < m->dim; i++) {
    m->x[i] = m->x[i] + mthd->h_params[0] * m->d[i];
  }
  m->y = m->fx(m->x);
}

void liner_method(model *m, method *mthd) {
  /*
  h_params[0] : step_width
  h_params[1] : target_index
  */
  m->x[(int)mthd->h_params[1]] += mthd->h_params[0];
  m->y = m->fx(m->x);
}


double uniform(void) {
  return ((double)rand()) / ((double)RAND_MAX + 1.0);
}


double rand_normal(double mu, double sigma) {
  return mu + sigma * sqrt(-2.0 * log(uniform())) * sin(2.0 * M_PI * uniform());
  
}

void hill_climbing(model *m, method *mthd) {
  /*
  h_params[0] : mu
  h_params[1] : sigma
  */
  double x_rand[1];
  if ((double)rand() == 16807) {
    srand((unsigned int)time(NULL));
  }
  x_rand[0] = rand_normal(mthd->h_params[0], mthd->h_params[1]) + m->x[0];

  if (m->fx(x_rand) < m->fx(m->x)) {
    m->x[0] = x_rand[0];
  }
  m->y = m->fx(m->x);
}
