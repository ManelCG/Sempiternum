#ifndef DRAW_JULIA_H
#define DRAW_JULIA_H

#include <complex.h>
#include <opencl_funcs.h>

struct complexBI{
  double complex p;
  int j;
  double der;
  struct complexBI *n;
  struct complexBI *prev;
  _Bool disregard;
};

unsigned char *draw_julia(int N, int h, int w,
                          double c[2], double Sx[2], double Sy[2],
                          char *plot_type,
                          struct OpenCL_Program **cl_prog, _Bool init_new_cl);

unsigned char *draw_thumbnail(int N, int h, int w,
                              double c[2],
                              char *plot_type,
                              struct OpenCL_Program **cl_prog, _Bool init_new);

unsigned char *draw_julia_polynomial(int N, int h, int w,
                                     int order, complex double *polynomial,
                                     double Sx[2], double Sy[2],
                                     int parameter,
                                     struct OpenCL_Program **cl_prog, _Bool init_new_cl);

unsigned char *draw_thumbnail_polynomial(int N, int h, int w,
                                         int order, complex double *polynomial,
                                         int parameter,
                                         struct OpenCL_Program **cl_prog, _Bool init_new);

void draw_julia_zoom(int frames, int N, int h, int w,
                     double c[2], double p[2],
                     double zoom_ratio, char *plot_type);

unsigned char *draw_julia_backwards_iteration(int N, int h, int w, double c[2], long MAX_D, _Bool revisit);

unsigned char *merge_sets(unsigned char *full, unsigned char *empty, int h, int w);

#endif //DRAW_JULIA_H
