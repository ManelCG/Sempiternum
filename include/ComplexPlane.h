#ifndef COMPLEXPLANE_H
#define COMPLEXPLANE_H

#define COMPLEX_PLANE_PARAMETER_SPACE 0
#define COMPLEX_PLANE_DYNAMIC_PLANE 1

#include <complex.h>
#include <stdbool.h>

#include <opencl_funcs.h>

#include <complex_function.h>

typedef struct ComplexPlane ComplexPlane;

//---Complex plane setters and getters

//Generates new plane, returns it and saves it in argument
ComplexPlane *complex_plane_new(ComplexPlane **);

void complex_plane_set_id(ComplexPlane *, int);
int complex_plane_get_id(ComplexPlane *);

void complex_plane_set_colorscheme(ComplexPlane *, int);
int complex_plane_get_colorscheme(ComplexPlane *);

//Copies complex plane. Returns & saves in arg
ComplexPlane *complex_plane_copy(ComplexPlane **dest, ComplexPlane *src);
void complex_plane_free(ComplexPlane *cp);

//IO
void complex_plane_print(ComplexPlane *cp);

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
void complex_plane_set_numerical_method_a(ComplexPlane *, complex double);
complex double complex_plane_get_numerical_method_a(ComplexPlane *);

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

//Stop drawing
void complex_plane_set_drawing_active(ComplexPlane *, _Bool);
_Bool complex_plane_is_drawing_active(ComplexPlane *);
void complex_plane_set_drawing_lines_active(ComplexPlane *, _Bool);
_Bool complex_plane_is_drawing_lines_active(ComplexPlane *);

//Extra data
void complex_plane_set_plot_type(ComplexPlane *, int);
int complex_plane_get_plot_type(ComplexPlane *);
void complex_plane_set_function_type(ComplexPlane *, int);
int complex_plane_get_function_type(ComplexPlane *);

//Polynomial
void complex_plane_set_polynomial_order(ComplexPlane *, int);
int complex_plane_get_polynomial_order(ComplexPlane *);
void complex_plane_set_polynomial_member(ComplexPlane *, complex v, int index);
void complex_plane_set_polynomial_derivative_member(ComplexPlane *, complex v, int index);
complex complex_plane_get_polynomial_member(ComplexPlane *, int index);
void complex_plane_free_polynomial(ComplexPlane *);
int complex_plane_set_polynomial_parameter(ComplexPlane *, int);  //-1 if fail
int complex_plane_get_polynomial_parameter(ComplexPlane *);
_Bool complex_plane_polynomial_is_null(ComplexPlane *);
int complex_plane_copy_polynomial(ComplexPlane *dest, ComplexPlane *src); //-1 if fail
int copy_polynomial(complex double *src, complex double *dest, int order);
const complex double *complex_plane_get_polynomial(ComplexPlane *);
const complex double *complex_plane_get_critical(ComplexPlane *cp);
void complex_plane_print_polynomial(ComplexPlane *);
void complex_plane_print_polynomial_derivative(ComplexPlane *);
void complex_plane_print_polynomial_parameters_derivative(ComplexPlane *cp);
void complex_plane_print_critical_point(ComplexPlane *cp);
void complex_plane_format_polynomial(ComplexPlane *cp);
void complex_plane_format_derivative(ComplexPlane *cp);
void complex_plane_format_second_derivative(ComplexPlane *cp);
void complex_plane_format_polynomial_critical_point(ComplexPlane *cp);
void complex_plane_format_arbitrary_polynomial(complex double *p, complex double *par, int order, char cvar, char cpar);
void complex_plane_print_all_polynomials(ComplexPlane *);
void complex_plane_set_polynomial_critical_point_member(ComplexPlane *cp, complex v, int index);


void complex_plane_free_polynomial_parameters(ComplexPlane *cp);
void complex_plane_set_parameters_vector_member(ComplexPlane *cp, complex double v, int i);
complex double complex_plane_get_parameters_vector_member(ComplexPlane *cp, int i);
void complex_plane_print_parameters(ComplexPlane *cp);


//---Complex algebra
double complex_plane_get_norm(complex double z);
complex complex_mul(complex a, complex b);
complex complex_div(complex a, complex b);
complex complex_compute_polynomial(const complex double *polynomial, int order, complex param, complex z, int parameter);
complex complex_compute_polynomial_p(const complex double *polynomial, complex double param, complex double z, int order);

double complex_plane_thumbnail_get_span(ComplexPlane *);

//---RootArrayMembers
int complex_plane_root_array_member_add(ComplexPlane *, RootArrayMember *);
_Bool complex_plane_root_array_member_remove_by_index(ComplexPlane *, int);
RootArrayMember *complex_plane_root_array_member_get_by_index(ComplexPlane *, int);


//OpenCL
void complex_plane_reset_opencl(ComplexPlane *cp);

//---Complex plane blueprints
void complex_plane_set_mandelbrot_parameters(ComplexPlane *);


//---Complex plane image functions
void draw_sequence_lines(ComplexPlane *C, double point[2], int w, int h);
void draw_sequence_lines_polynomial(ComplexPlane *C, double point[2], int w, int h);
void draw_sequence_lines_newton(ComplexPlane *cp, double point[2], int w, int h);
void draw_sequence_lines_numerical_method(ComplexPlane *cp, double point[2], int w, int h);

//---Numerical methods
complex newton_method(const complex double *polynomial,
                      const complex double *polynomial_derivative,
                      const complex double *polynomial_param,
                      const complex double *polynomial_param_derivative,
                      complex z, complex a, int order);

#endif //COMPLEXPLANE_H
