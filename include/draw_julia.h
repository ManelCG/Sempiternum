#ifndef DRAW_JULIA_H
#define DRAW_JULIA_H

#include <complex.h>
#include <opencl_funcs.h>
#include <complex_function.h>

struct complexBI{
  double complex p;
  int j;
  double der;
  struct complexBI *n;
  struct complexBI *prev;
  _Bool disregard;
};

int draw_julia_load_opencl_src(struct OpenCL_Program *cl_prog);

unsigned char *draw_julia(int N, int h, int w,
                          complex double c, double Sx[2], double Sy[2],
                          int plot_type, int color,
                          struct OpenCL_Program **cl_prog, _Bool init_new_cl);

unsigned char *draw_julia_polynomial(int N, int h, int w,
                                     int order, complex double *polynomial,
                                     double Sx[2], double Sy[2],
                                     int parameter, int color,
                                     struct OpenCL_Program **cl_prog, _Bool init_new_cl);

unsigned char *draw_julia_polynomial_fraction(int N, int h, int w,
                                              int order,
                                              complex double *numerator,
                                              complex double *denominator,
                                              double Sx[2], double Sy[2],
                                              int parameter, int color,
                                              struct OpenCL_Program **cl_prog, _Bool init_new_cl);

unsigned char *draw_julia_numerical_method(int N, int h, int w,
                                           int order,
                                           int plot_type, //Dynamic / parameter
                                           complex double *polynomial,
                                           complex double *polynomial_derivative,
                                           complex double *polynomial_second_derivative,
                                           complex double *polynomial_parameters,
                                           complex double *polynomial_parameters_derivative,
                                           complex double *polynomial_parameters_second_derivative,
                                           complex double *polynomial_critical_point,
                                           complex double numerical_method_a,
                                           int nroots, RootArrayMember *roots,
                                           double Sx[2], double Sy[2], int color,
                                           struct OpenCL_Program **cl_prog, _Bool init_new_cl);

unsigned char *draw_julia_custom_function(int N, int h, int w,
                                          complex double c,
                                          double Sx[2], double Sy[2],
                                          int plot_type, int color,
                                          const char *custom_function,
                                          struct OpenCL_Program **cl_prog, _Bool init_new_cl);

void draw_julia_zoom(int frames, int N, int h, int w,
                     complex double c, complex double p,
                     double zoom_ratio, int plot_type);

unsigned char *draw_julia_backwards_iteration(int N, int h, int w, double c[2], long MAX_D, _Bool revisit);

unsigned char *merge_sets(unsigned char *full, unsigned char *empty, int h, int w);

#endif //DRAW_JULIA_H
