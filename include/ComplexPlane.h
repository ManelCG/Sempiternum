#ifndef COMPLEXPLANE_H
#define COMPLEXPLANE_H

#include <complex.h>
#include <stdbool.h>

#include <opencl_funcs.h>

typedef struct ComplexPlane ComplexPlane;

//---Complex plane setters and getters

//Generates new plane, returns it and saves it in argument
ComplexPlane *complex_plane_new(ComplexPlane **);

void complex_plane_set_id(ComplexPlane *, int);
int complex_plane_get_id(ComplexPlane *);

//Copies complex plane. Returns & saves in arg
ComplexPlane *complex_plane_copy(ComplexPlane **dest, ComplexPlane *src);

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
double *complex_plane_get_spanx_array(ComplexPlane *);
double *complex_plane_get_spany_array(ComplexPlane *);

//Zoom
void complex_plane_set_zoom_point1(ComplexPlane *, double, double);
void complex_plane_set_zoom_point2(ComplexPlane *, double, double);
double *complex_plane_get_zoom_point1(ComplexPlane *, double **);
double *complex_plane_get_zoom_point2(ComplexPlane *, double **);
void complex_plane_free_zoom_point1(ComplexPlane *);
void complex_plane_free_zoom_point2(ComplexPlane *);
_Bool complex_plane_zoom_point1_is_null(ComplexPlane *);
_Bool complex_plane_zoom_point2_is_null(ComplexPlane *);
double complex_plane_zoom_point_get_spanx(ComplexPlane *);
double complex_plane_zoom_point_get_spany(ComplexPlane *);
void complex_plane_zoom_points_normalize(ComplexPlane *);
void complex_plane_zoom_points_set_center(ComplexPlane *);
int complex_plane_zoom_point1_get_pixel_value_x(ComplexPlane *);
int complex_plane_zoom_point1_get_pixel_value_y(ComplexPlane *);
int complex_plane_zoom_point2_get_pixel_value_x(ComplexPlane *);
int complex_plane_zoom_point2_get_pixel_value_y(ComplexPlane *);

//Plot
void complex_plane_alloc_empty_plot(ComplexPlane *);
void complex_plane_alloc_drawn_plot(ComplexPlane *);
int complex_plane_free_plot(ComplexPlane *);        //returns 1 if freed, 0 else
int complex_plane_free_drawn_plot(ComplexPlane *);  //returns 1 if freed, 0 else
const unsigned char *complex_plane_get_plot(ComplexPlane *);
unsigned char *complex_plane_get_drawn_plot(ComplexPlane *);
int complex_plane_copy_plot(ComplexPlane *);  //-1 if error, 0 success

void complex_plane_gen_plot(ComplexPlane *);
// void complex_plane_gen_thumb(ComplexPlane *);

//Stop drawing
void complex_plane_set_drawing_active(ComplexPlane *, _Bool);
_Bool complex_plane_is_drawing_active(ComplexPlane *);
void complex_plane_set_drawing_lines_active(ComplexPlane *, _Bool);
_Bool complex_plane_is_drawing_lines_active(ComplexPlane *);

//Extra data
void complex_plane_set_plot_type(ComplexPlane *, char *);
char *complex_plane_get_plot_type(ComplexPlane *, char **);
void complex_plane_set_function_type(ComplexPlane *, int);
int complex_plane_get_function_type(ComplexPlane *);

//Polynomial
void complex_plane_set_polynomial_order(ComplexPlane *, int);
int complex_plane_get_polynomial_order(ComplexPlane *);
void complex_plane_set_polynomial_member(ComplexPlane *, complex v, int index);
complex complex_plane_get_polynomial_member(ComplexPlane *, int index);
void complex_plane_free_polynomial(ComplexPlane *);
int complex_plane_set_polynomial_parameter(ComplexPlane *, int);  //-1 if fail
int complex_plane_get_polynomial_parameter(ComplexPlane *);
_Bool complex_plane_polynomial_is_null(ComplexPlane *);
int complex_plane_copy_polynomial(ComplexPlane *dest, ComplexPlane *src); //-1 if fail
const complex double *complex_plane_get_polynomial(ComplexPlane *);


//---Complex algebra
double complex_plane_get_norm(complex double z);

double complex_plane_thumbnail_get_span(ComplexPlane *);



//---Complex plane blueprints
void complex_plane_set_mandelbrot_parameters(ComplexPlane *);


//---Complex plane image functions
void draw_sequence_lines(ComplexPlane *C, double point[2], int w, int h);
void draw_sequence_lines_polynomial(ComplexPlane *C, const complex double *polynomial, int order, double point[2], int w, int h);

#endif //COMPLEXPLANE_H
