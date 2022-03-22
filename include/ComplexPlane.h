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
  int w, h, a, stride;
  int N, N_line;
} ComplexPlane;

//TODO:

//---Complex plane setters and getters

//Generates new plane, returns it and saves it in argument
ComplexPlane *complex_plane_new(ComplexPlane **);

//Dimensions
int complex_plane_set_dimensions(ComplexPlane *, int w, int h);
int complex_plane_get_width(ComplexPlane *);
int complex_plane_get_height(ComplexPlane *);
int complex_plane_get_area(ComplexPlane *);
void complex_plane_set_stride(ComplexPlane *, int);
int complex_plane_get_stride(ComplexPlane *);
int complex_plane_get_size(ComplexPlane *);

//Iters
void complex_plane_set_iterations(ComplexPlane *, int);
void complex_plane_set_line_iterations(ComplexPlane *, int);
int complex_plane_get_iterations(ComplexPlane *);
int complex_plane_get_line_iterations(ComplexPlane *);

//Parameters
void complex_plane_set_quadratic_parameter(ComplexPlane *, complex double);
void complex_plane_set_quadratic_parameter_real(ComplexPlane *, double);
void complex_plane_set_quadratic_parameter_imag(ComplexPlane *, double);
double complex_plane_get_quadratic_parameter_real(ComplexPlane *);
double complex_plane_get_quadratic_parameter_imag(ComplexPlane *);
complex double complex_plane_get_quadratic_parameter(ComplexPlane *);

//Center
void complex_plane_set_center(ComplexPlane *, complex double);
void complex_plane_set_center_real(ComplexPlane *, double);
void complex_plane_set_center_imag(ComplexPlane *, double);
double complex_plane_get_center_real(ComplexPlane *);
double complex_plane_get_center_imag(ComplexPlane *);
complex double complex_plane_get_center(ComplexPlane *);

//Spans
void complex_plane_set_spanx(ComplexPlane *, double);
void complex_plane_set_spany(ComplexPlane *, double);
void complex_plane_set_default_spans(ComplexPlane *);
void complex_plane_adjust_span_ratio(ComplexPlane *);
double complex_plane_get_spanx(ComplexPlane *);
double complex_plane_get_spany(ComplexPlane *);
double complex_plane_get_spanx0(ComplexPlane *);
double complex_plane_get_spanx1(ComplexPlane *);
double complex_plane_get_spany0(ComplexPlane *);
double complex_plane_get_spany1(ComplexPlane *);

//Plot
void complex_plane_alloc_empty_plot(ComplexPlane *);
void complex_plane_alloc_drawn_plot(ComplexPlane *);
int complex_plane_free_plot(ComplexPlane *);        //returns 1 if freed, 0 else
int complex_plane_free_drawn_plot(ComplexPlane *);  //returns 1 if freed, 0 else
char *complex_plane_get_plot(ComplexPlane *);
int complex_plane_copy_plot(ComplexPlane *);  //-1 if error, 0 success

//Extra data
void complex_plane_set_plot_type(ComplexPlane *, char *);


//---Complex algebra
double complex_plane_get_norm(complex double z);



//---Complex plane blueprints
void complex_plane_set_mandelbrot_parameters(ComplexPlane *);


//---Complex plane image functions
void draw_sequence_lines(ComplexPlane *C, double point[2], int w, int h);
void draw_sequence_lines_polynomial(ComplexPlane *C, complex *polynomial, int order, double point[2], int w, int h);

#endif //COMPLEXPLANE_H
