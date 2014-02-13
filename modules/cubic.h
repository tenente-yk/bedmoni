/*! \file cubic.h
 *  \brief Cubic spline implementation
 */

#ifndef __CUBIC_H
#define __CUBIC_H

#define CUBIC_SPLINE_MAX_NUM_ITEMS         (80*6) // 80 sec at hr=300 bpms (80*5), 6 - reserve

typedef struct
{
  double a;
  double b;
  double c;
  double d;
  double x;
} cubic_spline_tuple_t;

typedef struct
{
  cubic_spline_tuple_t    tuple[CUBIC_SPLINE_MAX_NUM_ITEMS];
  int                     n;
} cubic_spline_t;

int  cubic_spline_init(void);

void cubic_spline_deinit(int fd);

void cubic_spline_clear_all(int fd);

void cubic_spline_add(int fd, double x, double y);

void cubic_spline_build_ex(int fd, const double *x, const double *y, int n);

void cubic_spline_build(int fd);

double cubic_spline_build_f(int fd, double x);

#endif // __CUBIC_H
