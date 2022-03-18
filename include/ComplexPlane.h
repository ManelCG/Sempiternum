#ifndef COMPLEXPLANE_H
#define COMPLEXPLANE_H

#include <complex.h>

struct ComplexPlane{
  unsigned char *plot;
  unsigned char *drawn_plot;
  char *plot_type;
  double z[2];
  double center[2];
  double Sx[2], Sy[2];
  double SpanX;
  double SpanY;
  int polynomial_order;
  complex double *polynomial;
  int w, h, N, N_line;
};

void draw_sequence_lines(struct ComplexPlane *C, double point[2], int w, int h);
void draw_sequence_lines_polynomial(struct ComplexPlane *C, complex *polynomial, int order, double point[2], int w, int h);

#endif //COMPLEXPLANE_H
