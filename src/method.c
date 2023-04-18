#include <stdlib.h>
#include <math.h>
#include <ncurses.h>
#include "ci.h"
#include "method.h"
#include "util.h"

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
    temp += m->d[i] * m->dx[i](m->x, i);
  }
  for (int i = 0; i < 8; i++) {
    mthd->h_params[0] = pow(mthd->h_params[1], i);
    if (m->fx(epi_x, m->dim) <= (m->fx(m->x, m->dim) + (mthd->h_params[0] * mthd->h_params[2] * temp))) {
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
    if (m->fx(u, m->dim) >= m->fx(v, m->dim)) {
      a = u[0];
    } else if (m->fx(u, m->dim) < m->fx(v, m->dim)) {
      b = v[0];
    }
    m->x[0] = (a + b) / 2;
    if (fabs(b - a) < mthd->h_params[2]) {
      break;
    }
  }
}

double sub_fx(double *x, int dim) {
  extern double *x_ptr;
  extern double *d_ptr;
  double temp[dim];
  for (int i = 0; i < dim; i++) {
    temp[i] = x_ptr[i] + x[0] * d_ptr[i];
  }
  return temp[0] * temp[0] + 4 * temp[1] * temp[1];
}

void gradient_descent(model *m, method *mthd){
  /*
  h_params[0] : alpha = step_width
  */
  m->grad(m);

  double x[] = {mthd->h_params[0]};
  double (*dx[])(double *, int) = {};
  model alpha_model = newModel(sizeof(x)/sizeof(double), x, sub_fx, dx);
  mthd->sub_mthd->update(&alpha_model, mthd->sub_mthd);
  mthd->h_params[0] = alpha_model.x[0];

  for (int i = 0; i < m->dim; i++) {
    m->x[i] = m->x[i] + mthd->h_params[0] * m->d[i];
  }
  m->y = m->fx(m->x, m->dim);
}

void liner_method(model *m, method *mthd) {
  /*
  h_params[0] : step_width
  h_params[1] : target_index
  */
  m->x[(int)mthd->h_params[1]] += mthd->h_params[0];
  m->y = m->fx(m->x, m->dim);
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
  x_rand[0] = rand_normal(mthd->h_params[0], mthd->h_params[1]) * 1 + m->x[0];

  if (m->fx(x_rand, m->dim) < m->fx(m->x, m->dim)) {
    m->x[0] = x_rand[0];
  }
  m->y = m->fx(m->x, m->dim);
}

void simulated_annealing(model *m, method *mthd) {
  /*
  h_params[0] : T
  h_params[1] : T_min
  h_params[2] : T_max
  h_params[3] : alpha
  h_params[4] : n => step
  h_params[5] : mu
  h_params[6] : sigma
  h_params[7] : epsi_min
  h_params[8] : epsi_max
  h_params[9] : max_step
  */
  double x_rand[m->dim];
  double dx_sum = 0;

  mthd->h_params[4]++;
  mthd->h_params[0] = (mthd->h_params[2] - mthd->h_params[1]) * pow(mthd->h_params[3], mthd->h_params[4]) + mthd->h_params[1];
  for (int i = 0; i < m->dim; i++) {
    x_rand[i] = ((mthd->h_params[7] - mthd->h_params[8]) * (mthd->h_params[4] / mthd->h_params[9]) + mthd->h_params[8]) *
                rand_normal(mthd->h_params[5], mthd->h_params[6]) + m->x[i];
    //x_rand[i] = clampf(x_rand[i], -5.12, 5.12);
  }

  if (m->fx(x_rand, m->dim) < m->fx(m->x, m->dim)) {
    for (int i = 0; i < m->dim; i++){
      m->x[i] = x_rand[i];
    }
    if (m->fx(x_rand, m->dim) < m->fx(m->x_best, m->dim)) {
      for (int i = 0; i < m->dim; i++){
        m->x_best[i] = m->x[i];
      }
    }
  } else {
    for (int i = 0; i < m->dim; i++) {
      dx_sum += (x_rand[i] - m->x[i]);
    }
    if (uniform() <= exp(-1 * fabs(dx_sum) / mthd->h_params[0])) {
      for (int i = 0; i < m->dim; i++){
        m->x[i] = x_rand[i];
      }
    }
  }
  m->y = m->fx(m->x_best, m->dim);
}

void nelder_mead(model *m, method *mthd) {
  /*
    h_params[0] : M
    h_params[1] : alpha
    h_params[2] : gamma
    h_params[3] : beta
    h_params[4] : idx_low
    h_params[5] : idx_second
    h_params[6] : idx_high
    h_params[7] : fx_call
  */
  int fx_call = 0;
  double y_low = m->fx(m->x_candidates[(int)mthd->h_params[4]], m->dim);
  fx_call++;
  double y_second = m->fx(m->x_candidates[(int)mthd->h_params[5]], m->dim);
  fx_call++;
  double y_high = m->fx(m->x_candidates[(int)mthd->h_params[6]], m->dim);
  fx_call++;

  for (int i = 0; i < mthd->h_params[0]; i++) {
    if (m->fx(m->x_candidates[i], m->dim) < m->fx(m->x_candidates[(int)mthd->h_params[4]], m->dim)) {
      mthd->h_params[4] = i;
    } else if (m->fx(m->x_candidates[i], m->dim) > m->fx(m->x_candidates[(int)mthd->h_params[6]], m->dim)) {
      mthd->h_params[6] = i;
    } else if (m->fx(m->x_candidates[i], m->dim) > m->fx(m->x_candidates[(int)mthd->h_params[5]], m->dim)) {
      mthd->h_params[5] = i;
    }
  } 

  double x_center[m->dim];
  for (int i = 0; i < m->dim; i++) {
    x_center[i] = 0;
    for (int j = 0; j < mthd->h_params[0]; j++) {
      if (j != mthd->h_params[6]) {
        x_center[i] += m->x_candidates[j][i];
      }
    }
    x_center[i] /= (mthd->h_params[0] - 1);
  }

  double x_reflect[m->dim];
  for (int i = 0; i < m->dim; i++) {
    x_reflect[i] = x_center[i] + mthd->h_params[1] * (x_center[i] - m->x_candidates[(int)mthd->h_params[6]][i]);
  }
  double y_reflect = m->fx(x_reflect, m->dim);
  fx_call++;

  if (y_reflect < y_low) {
    double x_expansion[m->dim];
    for (int i = 0; i < m->dim; i++) {
      x_expansion[i] = x_center[i] + mthd->h_params[2] * (x_reflect[i] - x_center[i]);
    }
    double y_expansion = m->fx(x_expansion, m->dim);
    fx_call++;
    if (y_expansion < y_reflect) {
      for (int i = 0; i < m->dim; i++) {
        m->x_candidates[(int)mthd->h_params[6]][i] = x_expansion[i];
      }
    } else {
      for (int i = 0; i < m->dim; i++) {
        m->x_candidates[(int)mthd->h_params[6]][i] = x_reflect[i];
      }
    }
  } else if ((y_low < y_reflect) && (y_reflect < y_second)) {
    for (int i = 0; i < m->dim; i++) {
      m->x_candidates[(int)mthd->h_params[6]][i] = x_reflect[i];
    }
  } else {
    double x_shrink[m->dim];
    for (int i = 0; i < m->dim; i++) {
      x_shrink[i] = x_center[i] + mthd->h_params[3] * (m->x_candidates[(int)mthd->h_params[6]][i] - x_center[i]);
    }
    double y_shrink = m->fx(x_shrink, m->dim);
    fx_call++;
    if (y_shrink < y_high) {
      for (int i = 0; i < m->dim; i++) {
        m->x_candidates[(int)mthd->h_params[6]][i] = x_shrink[i];
      }
    } else {
      for (int i = 0; i < mthd->h_params[0]; i++) {
        if (i != mthd->h_params[4]) {
          for (int j = 0; j < m->dim; j++) {
            m->x_candidates[i][j] = m->x_candidates[(int)mthd->h_params[4]][j] + 0.5 * (m->x_candidates[i][j] - m->x_candidates[(int)mthd->h_params[4]][j]);
          }
        }
      }
    }
  }

  for (int i = 0; i < m->dim; i++) {
    m->x_best[i] = m->x_candidates[(int)mthd->h_params[4]][i];
  }
  m->y = m->fx(m->x_best, m->dim);
}

void particale_swarm_optimization(model *m, method *mthd) {
  /*
    h_params[0] : M
    h_params[1] : w
    h_params[2] : c
  */
  double y_candidates;

  for (int i = 0; i < mthd->h_params[0]; i++) {
    y_candidates = m->fx(m->x_candidates[i], m->dim);
    if (y_candidates < m->fx(m->x_candidates_best[i], m->dim)) {
      for (int j = 0; j < m->dim; j++) {
        m->x_candidates_best[i][j] = m->x_candidates[i][j];
      }
      if (m->fx(m->x_candidates_best[i], m->dim) < m->fx(m->x_best, m->dim)) {
        for (int j = 0; j < m->dim; j++) {
          m->x_best[j] = m->x_candidates_best[i][j];
        }
      }
    }
  }

  double r1, r2;
  for (int i = 0; i < mthd->h_params[0]; i++) {
    for (int j = 0; j < m->dim; j++) {
      r1 = uniform();
      r2 = uniform();
      m->v_candidates[i][j] = 
          mthd->h_params[1] * m->v_candidates[i][j]
        + mthd->h_params[2] * r1 * (m->x_candidates_best[i][j] - m->x_candidates[i][j])
        + mthd->h_params[2] * r2 * (m->x_best[j] - m->x_candidates[i][j]);
      m->x_candidates[i][j] = m->x_candidates[i][j] + m->v_candidates[i][j];
    }
  }

  m->y = m->fx(m->x_best, m->dim);
}

void differential_evolution_best1bin(model *m, method *mthd){
  /*
    h_params[0] : M
    h_params[1] : Cr
    h_params[2] : Fw
  */
  int ida = 0;
  int idb = rand() / (RAND_MAX / mthd->h_params[0] + 1);
  int idc = rand() / (RAND_MAX / mthd->h_params[0] + 1);
  int jr;
  double ri;
  double v[m->dim];
  double u[m->dim];
  double x_new[(int)mthd->h_params[0]][m->dim];

  for (int i = 0; i < mthd->h_params[0]; i++) {
    for (int j = 0; j < mthd->h_params[0]; j++) {
      if (m->fx(m->x_candidates[j], m->dim) < m->fx(m->x_candidates[ida], m->dim)) {
        ida = j;
      }
    }
    if (ida == i) {
      //continue;
    }
    while (idb == ida || idb == i) {
      idb = rand() / (RAND_MAX / mthd->h_params[0] + 1);
    }
    while (idc == ida || idc == idb || idc == i) {
      idc = rand() / (RAND_MAX / mthd->h_params[0] + 1);
    }
    for (int j = 0; j < m->dim; j++) {
      v[j] = m->x_candidates[ida][j] + mthd->h_params[2] * (m->x_candidates[idb][j] - m->x_candidates[idc][j]);
    }

    jr = rand() / (RAND_MAX / m->dim + 1);
    for (int j = 0; j < m->dim; j++) {
      ri = uniform();
      u[j] = ((ri <= mthd->h_params[1]) || (j == jr)) ? v[j] : m->x_candidates[i][j];
    }
    if (m->fx(u, m->dim) < m->fx(m->x_candidates[i], m->dim)) {
      for (int j = 0; j < m->dim; j++) {
        x_new[i][j] = u[j];
        m->x_candidates[i][j] = u[j];
      }
    } else {
      for (int j = 0; j < m->dim; j++) {
        x_new[i][j] = m->x_candidates[i][j];
      }
    }
  }

  for (int i = 0; i < mthd->h_params[0]; i++) {
    for (int j = 0; j < m->dim; j++) {
      m->x_candidates[i][j] = x_new[i][j];
    }
  }

  for (int i = 0; i < mthd->h_params[0]; i++) {
    if (m->fx(m->x_candidates[i], m->dim) < m->fx(m->x_best, m->dim)) {
      for (int j = 0; j < m->dim; j++) {
        m->x_best[j] = m->x_candidates[i][j];
      }
    }
  }

  m->y = m->fx(m->x_best, m->dim);
}

void differential_evolution_rand1bin(model *m, method *mthd){
  /*
    h_params[0] : M
    h_params[1] : Cr
    h_params[2] : Fw
  */
  int ida = rand() / (RAND_MAX / mthd->h_params[0] + 1);
  int idb = rand() / (RAND_MAX / mthd->h_params[0] + 1);
  int idc = rand() / (RAND_MAX / mthd->h_params[0] + 1);
  int jr;
  double ri;
  double v[m->dim];
  double u[m->dim];
  double x_new[(int)mthd->h_params[0]][m->dim];

  for (int i = 0; i < mthd->h_params[0]; i++) {
    while (ida == i) {
      ida = rand() / (RAND_MAX / mthd->h_params[0] + 1);
    }
    while (idb == ida || idb == i) {
      idb = rand() / (RAND_MAX / mthd->h_params[0] + 1);
    }
    while (idc == ida || idc == idb || idc == i) {
      idc = rand() / (RAND_MAX / mthd->h_params[0] + 1);
    }
    for (int j = 0; j < m->dim; j++) {
      v[j] = m->x_candidates[ida][j] + mthd->h_params[2] * (m->x_candidates[idb][j] - m->x_candidates[idc][j]);
    }

    jr = rand() / (RAND_MAX / m->dim + 1);
    for (int j = 0; j < m->dim; j++) {
      ri = uniform();
      u[j] = ((ri <= mthd->h_params[1]) || (j == jr)) ? v[j] : m->x_candidates[i][j];
    }
    if (m->fx(u, m->dim) < m->fx(m->x_candidates[i], m->dim)) {
      for (int j = 0; j < m->dim; j++) {
        x_new[i][j] = u[j];
        m->x_candidates[i][j] = u[j];
      }
    } else {
      for (int j = 0; j < m->dim; j++) {
        x_new[i][j] = m->x_candidates[i][j];
      }
    }
  }

  for (int i = 0; i < mthd->h_params[0]; i++) {
    for (int j = 0; j < m->dim; j++) {
      m->x_candidates[i][j] = x_new[i][j];
    }
  }

  for (int i = 0; i < mthd->h_params[0]; i++) {
    if (m->fx(m->x_candidates[i], m->dim) < m->fx(m->x_best, m->dim)) {
      for (int j = 0; j < m->dim; j++) {
        m->x_best[j] = m->x_candidates[i][j];
      }
    }
  }

  m->y = m->fx(m->x_best, m->dim);
}
