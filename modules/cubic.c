/*! \file cubic.c
 *  \brief Cubic spline implementation
 */

#include <stdlib.h>
#include <assert.h>
#include "cubic.h"

static double alpha[CUBIC_SPLINE_MAX_NUM_ITEMS];
static double beta[CUBIC_SPLINE_MAX_NUM_ITEMS];
static double A, B, C, F, h_i, h_i1, z;

int  cubic_spline_init(void)
{
  cubic_spline_t * p;
  p = (cubic_spline_t*)calloc(1, sizeof(cubic_spline_t));
  return (int)(p);
}

void cubic_spline_deinit(int fd)
{
  if (!fd) return;
  free((void*)fd);
}

void cubic_spline_clear_all(int fd)
{
  cubic_spline_t * p;
  p = (cubic_spline_t*) fd;
  p->n = 0;
}

void cubic_spline_add(int fd, double x, double y)
{
  cubic_spline_t * p;
  cubic_spline_tuple_t * splines;

  p = (cubic_spline_t*) fd;
  splines = p->tuple;

  splines[p->n].x = x;
  splines[p->n].a = y;
  p->n += 1;
  if (p->n >= CUBIC_SPLINE_MAX_NUM_ITEMS) p->n -= 1;
}

void cubic_spline_build_ex(int fd, const double *x, const double *y, int n)
{
  cubic_spline_t * p;
  cubic_spline_tuple_t * splines;
  int i;

  p = (cubic_spline_t*) fd;
  splines = p->tuple;

  if (n >= CUBIC_SPLINE_MAX_NUM_ITEMS) n = CUBIC_SPLINE_MAX_NUM_ITEMS-1;
  p->n = n;

  for (i = 0; i < n; ++i)
  {
    splines[i].x = x[i];
    splines[i].a = y[i];
  }
  splines[0].c = 0.;

  alpha[0] = beta[0] = 0.;
  for (i = 1; i < n - 1; ++i)
  {
    h_i = x[i] - x[i - 1], h_i1 = x[i + 1] - x[i];
    A = h_i;
    C = 2. * (h_i + h_i1);
    B = h_i1;
    F = 6. * ((y[i + 1] - y[i]) / h_i1 - (y[i] - y[i - 1]) / h_i);
    z = (A * alpha[i - 1] + C);
    alpha[i] = -B / z;
    beta[i] = (F - A * beta[i - 1]) / z;
  }

  splines[n - 1].c = (F - A * beta[n - 2]) / (C + A * alpha[n - 2]);

  for (i = n - 2; i > 0; --i)
    splines[i].c = alpha[i] * splines[i + 1].c + beta[i];

  for (i = n - 1; i > 0; --i)
  {
    double h_i = x[i] - x[i - 1];
    splines[i].d = (splines[i].c - splines[i - 1].c) / h_i;
    splines[i].b = h_i * (2. * splines[i].c + splines[i - 1].c) / 6. + (y[i] - y[i - 1]) / h_i;
  }
}

void cubic_spline_build(int fd)
{
  cubic_spline_t * p;
  cubic_spline_tuple_t * splines;
  int i, n;

  p = (cubic_spline_t*) fd;
  splines = p->tuple;
  n = p->n;

  splines[0].c = 0.;

  alpha[0] = beta[0] = 0.;
  for (i = 1; i < n - 1; ++i)
  {
    h_i = splines[i].x - splines[i-1].x, h_i1 = splines[i+1].x - splines[i].x;
    A = h_i;
    C = 2. * (h_i + h_i1);
    B = h_i1;
    F = 6. * ((splines[i+1].a - splines[i].a) / h_i1 - (splines[i].a - splines[i-1].a) / h_i);
    z = (A * alpha[i - 1] + C);
    alpha[i] = -B / z;
    beta[i] = (F - A * beta[i - 1]) / z;
  }

  splines[n - 1].c = (F - A * beta[n - 2]) / (C + A * alpha[n - 2]);

  for (i = n - 2; i > 0; --i)
    splines[i].c = alpha[i] * splines[i + 1].c + beta[i];

  for (i = n - 1; i > 0; --i)
  {
    double h_i = splines[i].x - splines[i-1].x;
    splines[i].d = (splines[i].c - splines[i - 1].c) / h_i;
    splines[i].b = h_i * (2. * splines[i].c + splines[i - 1].c) / 6. + (splines[i].a - splines[i-1].a) / h_i;
  }
}

double cubic_spline_build_f(int fd, double x)
{
  cubic_spline_t * p;
  cubic_spline_tuple_t * splines;
  int n;

  p = (cubic_spline_t*) fd;
  n = p->n;
  splines = p->tuple;

#if 0
  if (!splines)
    return std::numeric_limits<double>::quiet_NaN(); // ���� ������� ��� �� ��������� - ���������� NaN
#endif
  assert(splines);

  cubic_spline_tuple_t *s;
  if (x <= splines[0].x) // ���� x ������ ����� ����� x[0] - ���������� ������ ��-��� �������
    s = splines + 1;
  else if (x >= splines[n - 1].x) // ���� x ������ ����� ����� x[n - 1] - ���������� ��������� ��-��� �������
    s = splines + n - 1;
  else // ����� x ����� ����� ���������� ������� ����� - ���������� �������� ����� ������� ��-�� �������
  {
    int i = 0, j = n - 1;
    while (i + 1 < j)
    {
      int k = i + (j - i) / 2;
      if (x <= splines[k].x)
        j = k;
      else
        i = k;
    }
    s = splines + j;
  }

  double dx = (x - s->x);
  return s->a + (s->b + (s->c / 2. + s->d * dx / 6.) * dx) * dx; // ��������� �������� ������� � �������� ����� �� ����� ������� (� ��������, "�����" ���������� �������� �� ����� ������� ���, �� ���� �� ��� ��� ����, ��� �������)
}
