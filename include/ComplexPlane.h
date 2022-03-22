#ifndef COMPLEXPLANE_H
#define COMPLEXPLANE_H

#include <complex.h>

typedef struct ComplexPlane{
  unsigned char *plot;
  unsigned char *drawn_plot;
  int pixel_stride;

  char *plot_type;
  complex double param;
  complex double center;

  double Sx[2], Sy[2];
  double SpanX;
  double SpanY;
  int polynomial_order;
  int polynomial_parameter;
  complex double *polynomial;
  int w, h, a, N, N_line;
} ComplexPlane;

//TODO:

//Complex algebra
double complex_plane_get_norm(complex double z);


//Complex plane setters and getters
int complex_plane_set_dimensions(ComplexPlane *, int w, int h);
int complex_plane_get_width(ComplexPlane *);
int complex_plane_get_height(ComplexPlane *);
int complex_plane_get_area(ComplexPlane *);

void complex_plane_set_iterations(ComplexPlane *, int);

void complex_plane_set_quadratic_parameter(ComplexPlane *, complex double);
complex double complex_plane_get_quadratic_parameter(ComplexPlane *);
double complex_plane_get_quadratic_parameter_real(ComplexPlane *);
double complex_plane_get_quadratic_parameter_imag(ComplexPlane *);

void complex_plane_set_center(ComplexPlane *, complex double);
complex double complex_plane_get_center(ComplexPlane *);
double complex_plane_get_center_real(ComplexPlane *);
double complex_plane_get_center_imag(ComplexPlane *);

void complex_plane_set_plot_type(ComplexPlane *, char *);


//Complex plane blueprints
void complex_plane_set_mandelbrot_parameters(ComplexPlane *);


//Complex plane image functions
void draw_sequence_lines(ComplexPlane *C, double point[2], int w, int h);
void draw_sequence_lines_polynomial(ComplexPlane *C, complex *polynomial, int order, double point[2], int w, int h);

#endif //COMPLEXPLANE_H
