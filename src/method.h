#ifndef _METHOD_H
#define _METHOD_H

#include "ci.h"

void armijo_rule(model *m, method *mthd);
void bisection_method(model *m, method *mthd);
void gradient_descent(model *m, method *mthd);
void liner_method(model *m, method *mthd);

double uniform(void);
double rand_normal(double mu, double sigma);

void hill_climbing(model *m, method *mthd);
void simulated_annealing(model *m, method *mthd);
#endif
