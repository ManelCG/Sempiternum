#include <ComplexPlane.h>
#include <image_manipulation.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include <complex_function.h>

#include <stdio.h>

#include <draw_julia.h>

#include <opencl_funcs.h>

// #define DEBUG_CP

typedef struct ComplexPlane{
  int ID;
  int colorscheme;

  unsigned char *plot;
  unsigned char *drawn_plot;
  int pixel_stride;

  int function_type;
  int plot_type;
  complex double param;
  complex double center;

  _Bool is_drawing_active;
  _Bool is_lines_active;

  double *zoom_point1[2];
  double *zoom_point2[2];

  double Sx[2], Sy[2];
  double SpanX;
  double SpanY;
  int polynomial_order;
  int polynomial_parameter;
  complex double *polynomial;
  complex double *polynomial_derivative;
  complex double *second_polynomial_derivative;
  complex double *polynomial_parameters;
  complex double *polynomial_parameters_derivative;
  complex double *second_polynomial_parameters_derivative;
  complex double *polynomial_critical_point;

  complex double numerical_method_a;

  int nroots;
  RootArrayMember *roots;

  int w, h, a;
  int N, N_line;

  struct OpenCL_Program *cl;
} ComplexPlane;

void complex_plane_free(ComplexPlane *cp){
  complex_plane_free_plot(cp);
  complex_plane_free_drawn_plot(cp);


  complex_plane_free_polynomial(cp);
  complex_plane_free_polynomial_parameters(cp);

  // opencl_free(cp->cl);

  free(cp);
}

//---RootArrayMembers
int complex_plane_root_array_member_add(ComplexPlane *cp, RootArrayMember *ram){
  if (cp->roots == NULL){
    cp->roots = ram;
    cp->nroots ++;
    return 0;
  }

  int i = 0;
  RootArrayMember *r = cp->roots;
  RootArrayMember *p = NULL;
  for (i = 0; i < cp->nroots; i++){
    p = r;
    r = root_array_member_next(r);
  }

  root_array_member_set_next(p, ram);
  cp->nroots ++;

  return i;
}
_Bool complex_plane_root_array_member_remove_by_index(ComplexPlane *cp, int ram){
  if (cp->roots == NULL){
    return false;
  }

  int i = 0;
  RootArrayMember *r = cp->roots;
  RootArrayMember *p = NULL;

  if (ram == 0){
    cp->roots = root_array_member_next(r);
    cp->nroots --;
    root_array_member_destroy(r);
    return true;
  }

  for (i = 0; i < ram; i++){
    p = r;
    r = root_array_member_next(r);

    if (r == NULL){
      return false;
    }
  }

  root_array_member_set_next(p, root_array_member_next(r));
  root_array_member_destroy(r);
  cp->nroots --;
  return true;
}
RootArrayMember *complex_plane_root_array_member_get_by_index(ComplexPlane *cp, int ram){
  if (cp->roots == NULL){
    return NULL;
  }

  int i;
  RootArrayMember *r = cp->roots;
  for (i = 0; i < ram; i++){
    r = root_array_member_next(r);

    if (r == NULL){
      return NULL;
    }
  }
  return r;
}
int complex_plane_root_array_member_get_n(ComplexPlane *cp){
  return cp->nroots;
}


//---Setters and getters
ComplexPlane *complex_plane_new(ComplexPlane **cp){
  ComplexPlane *new = malloc(sizeof(ComplexPlane));

  new->plot = NULL;
  new->drawn_plot = NULL;
  new->pixel_stride = 3;

  new->ID = 0;

  new->polynomial = NULL;
  new->polynomial_derivative = NULL;
  new->polynomial_parameters = NULL;
  new->polynomial_parameters_derivative = NULL;
  new->second_polynomial_derivative = NULL;
  new->second_polynomial_parameters_derivative = NULL;
  new->polynomial_critical_point = NULL;

  complex_plane_set_function_type(new, 0);
  complex_plane_set_numerical_method_a(new, 0);

  new->is_drawing_active = true;
  new->is_lines_active = true;

  new->zoom_point1[0] = NULL;
  new->zoom_point1[1] = NULL;
  new->zoom_point2[0] = NULL;
  new->zoom_point2[1] = NULL;

  new->cl = get_opencl_info();
  new->cl->init = false;

  new->nroots = 0;
  new->roots = NULL;

  complex_plane_set_colorscheme(new, 0);

  if (cp != NULL){
    *cp = new;
  }
  return new;
}

ComplexPlane *complex_plane_copy(ComplexPlane **dest, ComplexPlane *src){
  ComplexPlane *new = complex_plane_new(NULL);

  new->ID = src->ID + 1;

  complex_plane_set_stride(new, complex_plane_get_stride(src));
  complex_plane_set_dimensions(new, src->w, src->h);

  complex_plane_set_iterations(new, complex_plane_get_iterations(src));
  complex_plane_set_line_iterations(new, complex_plane_get_line_iterations(src));

  complex_plane_set_center(new, complex_plane_get_center(src));
  complex_plane_set_spanx(new, complex_plane_get_spanx(src));
  complex_plane_set_spany(new, complex_plane_get_spany(src));

  complex_plane_set_plot_type(new, complex_plane_get_plot_type(src));
  complex_plane_set_quadratic_parameter(new, complex_plane_get_quadratic_parameter(src));

  complex_plane_set_function_type(new, complex_plane_get_function_type(src));

  complex_plane_set_drawing_active(new, complex_plane_is_drawing_active(src));
  complex_plane_set_drawing_lines_active(new, complex_plane_is_drawing_lines_active(src));

  complex_plane_set_polynomial_order(new, complex_plane_get_polynomial_order(src));
  complex_plane_set_polynomial_parameter(new, complex_plane_get_polynomial_parameter(src));
  complex_plane_copy_polynomial(new, src);
  copy_polynomial(src->polynomial, new->polynomial, complex_plane_get_polynomial_order(src));
  copy_polynomial(src->polynomial_derivative, new->polynomial_derivative, complex_plane_get_polynomial_order(src));
  copy_polynomial(src->second_polynomial_derivative, new->second_polynomial_derivative, complex_plane_get_polynomial_order(src));
  copy_polynomial(src->polynomial_parameters, new->polynomial_parameters, complex_plane_get_polynomial_order(src));
  copy_polynomial(src->polynomial_parameters_derivative, new->polynomial_parameters_derivative, complex_plane_get_polynomial_order(src));
  copy_polynomial(src->second_polynomial_parameters_derivative, new->second_polynomial_parameters_derivative, complex_plane_get_polynomial_order(src));
  copy_polynomial(src->polynomial_critical_point, new->polynomial_critical_point, complex_plane_get_polynomial_order(src));

  complex_plane_set_numerical_method_a(new, complex_plane_get_numerical_method_a(src));

  complex_plane_set_colorscheme(new, complex_plane_get_colorscheme(src));

  //TODO: Copy zoom_point1 & zoom_point2

  //TODO: Copy RAMs
  new->nroots = src->nroots;
  new->roots = root_array_copy(NULL, src->roots, src->nroots);

  if (dest != NULL){
    *dest = new;
  }
  return new;
}

void complex_plane_set_colorscheme(ComplexPlane *cp, int c){
  cp->colorscheme = c;
}
int complex_plane_get_colorscheme(ComplexPlane *cp){
  return cp->colorscheme;
}

void complex_plane_print(ComplexPlane *cp){
  printf("Printing Complex Plane:\n");
  printf("Complex plane   %d\n", cp->ID);
  printf("Dimensions:     %d x %d = %d\n", cp->w, cp->h, cp->a);
  printf("Size of plot:   %d x %d = %d\n", cp->a, cp->pixel_stride, cp->a * cp->pixel_stride);
  printf("Is plot alloc:  ");
  if (cp->plot == NULL){
    printf("No\n");
  } else {
    printf("Yes\n");
  }
  printf("Center:         %g %+gi\n", creal(cp->center), cimag(cp->center));
  printf("Spans:          %g, %g\n", cp->SpanX, cp->SpanY);

  printf("Numerical method a: %g %+gi\n", creal(cp->numerical_method_a), cimag(cp->numerical_method_a));


  complex_plane_print_all_polynomials(cp);
}

void complex_plane_print_all_polynomials(ComplexPlane *cp){
  if (cp->polynomial != NULL){
    printf("Polynomial     = ");
    complex_plane_format_polynomial(cp);
    printf("Derivative     = ");
    complex_plane_format_derivative(cp);
    printf("2nd Derivative = ");
    complex_plane_format_second_derivative(cp);
    printf("Critical Point = ");
    complex_plane_format_polynomial_critical_point(cp);
  }
}

void complex_plane_set_id(ComplexPlane *cp, int id){
  cp->ID = id;
}
int complex_plane_get_id(ComplexPlane *cp){
  return cp->ID;
}

//--- Dimensions
int complex_plane_set_dimensions(ComplexPlane *cp, int w, int h){
  #ifdef DEBUG_CP
  printf("CP %d: Set dimensions (%d x %d = %d)\n", cp->ID, w, h, w*h);
  #endif
  complex_plane_free_plot(cp);
  complex_plane_free_drawn_plot(cp);

  cp->w = w; cp->h = h; cp->a = w*h;
  complex_plane_alloc_empty_plot(cp);

  return cp->a;
}
int complex_plane_get_width(ComplexPlane *cp){
  return cp->w;
}
int complex_plane_get_height(ComplexPlane *cp){
  return cp->h;
}
int complex_plane_get_area(ComplexPlane *cp){
  return cp->a;
}
void complex_plane_set_stride(ComplexPlane *cp, int s){
  cp->pixel_stride = s;
}
int complex_plane_get_stride(ComplexPlane *cp){
  return cp->pixel_stride;
}
int complex_plane_get_size(ComplexPlane *cp){
  return (complex_plane_get_area(cp) * complex_plane_get_stride(cp));
}

void complex_plane_set_plot_type(ComplexPlane *cp, int plot_type){
  cp->plot_type = plot_type;
}
int complex_plane_get_plot_type(ComplexPlane *cp){
  return cp->plot_type;
}
void complex_plane_set_function_type(ComplexPlane *cp, int type){
  cp->function_type = type;
  // if (type == 0){
  complex_plane_set_polynomial_order(cp, -1);
  // }
}
int complex_plane_get_function_type(ComplexPlane *cp){
  return cp->function_type;
}

//---stop_drawing
void complex_plane_set_drawing_active(ComplexPlane *cp, _Bool b){
  cp->is_drawing_active = b;
}
_Bool complex_plane_is_drawing_active(ComplexPlane *cp){
  return cp->is_drawing_active;
}
void complex_plane_set_drawing_lines_active(ComplexPlane *cp, _Bool b){
  cp->is_lines_active = b;
}
_Bool complex_plane_is_drawing_lines_active(ComplexPlane *cp){
  return cp->is_lines_active;
}

//---polynomial
void complex_plane_set_polynomial_critical_point_member(ComplexPlane *cp, complex v, int index){
  if (index > cp->polynomial_order){
    return;
  }
  cp->polynomial_critical_point[index] = v;
}
void complex_plane_set_polynomial_derivative_member(ComplexPlane *cp, complex v, int index){
  if (index > cp->polynomial_order){
    return;
  }
  cp->polynomial_derivative[index] = v;
}
void complex_plane_set_second_polynomial_derivative_member(ComplexPlane *cp, complex v, int index){
  if (index > cp->polynomial_order){
    return;
  }
  cp->second_polynomial_derivative[index] = v;
}
void complex_plane_set_polynomial_parameters_derivative_member(ComplexPlane *cp, complex v, int index){
  if (index > cp->polynomial_order){
    return;
  }
  cp->polynomial_parameters_derivative[index] = v;
}
void complex_plane_set_second_polynomial_parameters_derivative_member(ComplexPlane *cp, complex v, int index){
  if (index > cp->polynomial_order){
    return;
  }
  cp->second_polynomial_parameters_derivative[index] = v;
}
void complex_plane_set_polynomial_order(ComplexPlane *cp, int o){
  complex_plane_free_polynomial(cp);

  cp->polynomial_order = o;
  cp->polynomial_parameter = -1;

  if (o > 0){
    cp->polynomial = calloc(sizeof(complex double) * (o+2), 1);
    cp->polynomial_parameters = calloc(sizeof(complex double) * (o+2), 1);
    cp->polynomial_derivative = calloc(sizeof(complex double) * (o+2), 1);
    cp->polynomial_parameters_derivative = calloc(sizeof(complex double) * (o+2), 1);
    cp->second_polynomial_derivative = calloc(sizeof(complex double) * (o+2), 1);
    cp->second_polynomial_parameters_derivative = calloc(sizeof(complex double) * (o+2), 1);
    cp->polynomial_critical_point = calloc(sizeof(complex double) * (o+2), 1);
    for (int i = 0; i <= o+1; i++){
      complex_plane_set_polynomial_member(cp, 0, i);
    }
    for (int i = 0; i < o+1; i++){
      complex_plane_set_polynomial_derivative_member(cp, 0, i);
    }
  }
}
int complex_plane_get_polynomial_order(ComplexPlane *cp){
  return cp->polynomial_order;
}
void complex_plane_set_polynomial_member(ComplexPlane *cp, complex v, int index){
  int order = cp->polynomial_order;
  if (index > order + 1){
    return;
  }
  cp->polynomial[index] = v;

  if (index < order){
    complex_plane_set_polynomial_derivative_member(cp, (order - index) * v, index + 1);
  }
  if (index < order -1){
    complex_plane_set_second_polynomial_derivative_member(cp, (order - index - 1)*(order - index) * v, index + 2);
  }


}
complex complex_plane_get_polynomial_member(ComplexPlane *cp, int index){
  return cp->polynomial[index];
}
complex complex_plane_get_polynomial_critical_point_member(ComplexPlane *cp, int index){
  return cp->polynomial_critical_point[index];
}
_Bool complex_plane_polynomial_is_null(ComplexPlane *cp){
  return (cp->polynomial == NULL);
}
void complex_plane_free_polynomial(ComplexPlane *cp){
  if (cp->polynomial != NULL){
    free(cp->polynomial);
    free(cp->polynomial_derivative);
    free(cp->second_polynomial_derivative);
    cp->polynomial = NULL;
  }
  complex_plane_free_polynomial_parameters(cp);
}
void complex_plane_free_polynomial_parameters(ComplexPlane *cp){
  if (cp->polynomial_parameters != NULL){
    free(cp->polynomial_parameters);
    free(cp->polynomial_parameters_derivative);
    free(cp->second_polynomial_parameters_derivative);
    free(cp->polynomial_critical_point);
    cp->polynomial_parameters = NULL;
  }
}
int complex_plane_set_polynomial_parameter(ComplexPlane *cp, int p){
  if (p > cp->polynomial_order+1){
    return -1;
  }
  // if (p == -1){
  //   complex_plane_free_polynomial_parameters(cp);
  // } else {
  //   cp->polynomial_parameters[p] = 1;
  // }
  cp->polynomial_parameter = p;
  return 0;
}
int complex_plane_get_polynomial_parameter(ComplexPlane *cp){
  return cp->polynomial_parameter;
}
int copy_polynomial(complex double *src, complex double *dest, int order){
  for (int i = 0; i <= order +1; i++){
    dest[i] = src[i];
  }
  return 0;
}
int complex_plane_copy_polynomial(ComplexPlane *d, ComplexPlane *s){
  int order = complex_plane_get_polynomial_order(s);
  int orderd = complex_plane_get_polynomial_order(d);
  if (order != orderd ||
      complex_plane_polynomial_is_null(d) ||
      complex_plane_polynomial_is_null(s)){
    return -1;
  }
  for (int i = 0; i <= order + 1; i++){
    complex v = complex_plane_get_polynomial_member(s, i);
    complex_plane_set_polynomial_member(d, v, i);
  }
  return 0;
}
const complex double *complex_plane_get_polynomial(ComplexPlane *cp){
  return cp->polynomial;
}
const complex double *complex_plane_get_critical(ComplexPlane *cp){
  return cp->polynomial_critical_point;
}
void complex_plane_print_polynomial(ComplexPlane *cp){
  if (cp->polynomial != NULL){
    for (int i = 0; i < cp->polynomial_order; i++){
      if (i == cp->polynomial_order){
        printf("(%g %+g)z ", creal(cp->polynomial[i]), cimag(cp->polynomial[i]));
      } else {
        printf("(%g %+g)z^%d ", creal(cp->polynomial[i]), cimag(cp->polynomial[i]), cp->polynomial_order - i);
      }
      printf("+ ");
    }
    printf("(%g %+g)", creal(cp->polynomial[cp->polynomial_order]), cimag(cp->polynomial[cp->polynomial_order]));
    printf("\n");
  } else {
    printf("(null)\n");
  }
}
void complex_plane_print_critical_point(ComplexPlane *cp){
  if (cp->polynomial_critical_point != NULL){
    for (int i = 0; i < cp->polynomial_order; i++){
      if (i == cp->polynomial_order){
        printf("(%g %+g)z ", creal(cp->polynomial_critical_point[i]), cimag(cp->polynomial_critical_point[i]));
      } else {
        printf("(%g %+g)z^%d ", creal(cp->polynomial_critical_point[i]), cimag(cp->polynomial_critical_point[i]), cp->polynomial_order - i);
      }
      printf("+ ");
    }
    printf("(%g %+g)", creal(cp->polynomial_critical_point[cp->polynomial_order]), cimag(cp->polynomial_critical_point[cp->polynomial_order]));
    printf("\n");
  } else {
    printf("(null)\n");
  }
}
void complex_plane_print_parameters(ComplexPlane *cp){
  for (int i = 0; i < cp->polynomial_order; i++){
    if (i == cp->polynomial_order){
      printf("(%g %+g)z ", creal(cp->polynomial_parameters[i]), cimag(cp->polynomial_parameters[i]));
    } else {
      printf("(%g %+g)z^%d ", creal(cp->polynomial_parameters[i]), cimag(cp->polynomial_parameters[i]), cp->polynomial_order - i);
    }
    printf("+ ");
  }
  printf("(%g %+g)", creal(cp->polynomial_parameters[cp->polynomial_order]), cimag(cp->polynomial_parameters[cp->polynomial_order]));
  printf("\n");
}
void complex_plane_format_polynomial(ComplexPlane *cp){
  complex_plane_format_arbitrary_polynomial(cp->polynomial, cp->polynomial_parameters, cp->polynomial_order, 'z', 'a');
}
void complex_plane_print_polynomial_derivative(ComplexPlane *cp){
  for (int i = 0; i < cp->polynomial_order; i++){
    if (i == cp->polynomial_order){
      printf("(%g %+g)z ", creal(cp->polynomial_derivative[i]), cimag(cp->polynomial_derivative[i]));
    } else {
      printf("(%g %+g)z^%d ", creal(cp->polynomial_derivative[i]), cimag(cp->polynomial_derivative[i]), cp->polynomial_order - i);
    }
    printf("+ ");
  }
  printf("(%g %+g)", creal(cp->polynomial_derivative[cp->polynomial_order]), cimag(cp->polynomial_derivative[cp->polynomial_order]));
  printf("\n");
}
void complex_plane_format_second_derivative(ComplexPlane *cp){
  complex_plane_format_arbitrary_polynomial(cp->second_polynomial_derivative, cp->second_polynomial_parameters_derivative, cp->polynomial_order, 'z', 'a');
}
void complex_plane_format_derivative(ComplexPlane *cp){
  complex_plane_format_arbitrary_polynomial(cp->polynomial_derivative, cp->polynomial_parameters_derivative, cp->polynomial_order, 'z', 'a');
}
void complex_plane_format_polynomial_critical_point(ComplexPlane *cp){
  complex_plane_format_arbitrary_polynomial(cp->polynomial_critical_point, NULL, cp->polynomial_order, 'a', '0');
}
void complex_plane_format_arbitrary_polynomial(const complex double *polynomial, const complex double *polynomial_parameters, int polynomial_order, char cvar, char cpar){
  _Bool printed = false;
  if (polynomial != NULL){
    for (int i = 0; i < polynomial_order; i++){
      if (i == polynomial_order - 1){
        if (polynomial[i] != 0){
          if (polynomial[i] == 1){
            if (printed == false){
              printf(" %c ", cvar);
            } else {
              printf("+ %c ", cvar);
            }
          } else if (polynomial[i] == -1){
            printf("- %c ", cvar);
          } else if (polynomial[i] == creal(polynomial[i])){
            if (printed == false){
              printf("%g%c ", creal(polynomial[i]), cvar);
            } else {
              printf("%+g%c ", creal(polynomial[i]), cvar);
            }
          } else if (polynomial[i] == cimag(polynomial[i])){
            if (printed == false){
              printf("%gi%c ", cimag(polynomial[i]), cvar);
            } else {
              printf("%+gi%c ", cimag(polynomial[i]), cvar);
            }
          } else {
            printf("(%+g %+gi)%c ", creal(polynomial[i]), cimag(polynomial[i]), cvar);
          }
          printed = true;
        }
        if (polynomial_parameters != NULL){
          if (polynomial_parameters[i] != 0){
            if (polynomial_parameters[i] == 1){
              if (printed == false){
                printf(" %c%c ", cpar, cvar);
              } else {
                printf("+ %c%c ", cpar, cvar);
              }
            } else if (polynomial_parameters[i] == -1){
              printf("- %c%c ", cpar, cvar);
            } else if (polynomial_parameters[i] == creal(polynomial_parameters[i])){
              if (printed == false){
                printf("%g%c%c ", creal(polynomial_parameters[i]), cpar, cvar);
              } else {
                printf("%+g%c%c ", creal(polynomial_parameters[i]), cpar, cvar);
              }
            } else if (polynomial_parameters[i] == cimag(polynomial_parameters[i])){
              if (printed == false){
                printf("%gi%c%c ", cimag(polynomial_parameters[i]), cpar, cvar);
              } else {
                printf("%+gi%c%c ", cimag(polynomial_parameters[i]), cpar, cvar);
              }
            } else {
              if (printed == false){
                printf("(%+g %+gi)%c%c ", creal(polynomial_parameters[i]), cimag(polynomial_parameters[i]), cpar, cvar);
              } else {
                printf("+(%+g %+gi)%c%c ", creal(polynomial_parameters[i]), cimag(polynomial_parameters[i]), cpar, cvar);
              }
            }
            printed = true;
          }
        }
      } else {
        if (polynomial[i] != 0){
          if (polynomial[i] == 1){
            if (printed == false){
              printf(" %c%s ", cvar, complex_function_get_exponent_str(polynomial_order-i));
            } else {
              printf("+ %c%s ", cvar, complex_function_get_exponent_str(polynomial_order-i));
            }
          } else if (polynomial[i] == -1){
            printf("- %c%s ", cvar, complex_function_get_exponent_str(polynomial_order-i));
          } else if (polynomial[i] == creal(polynomial[i])){
            if (printed == false){
              printf("%g%c%s ", creal(polynomial[i]), cvar, complex_function_get_exponent_str(polynomial_order - i));
            } else {
              printf("%+g%c%s ", creal(polynomial[i]), cvar, complex_function_get_exponent_str(polynomial_order - i));
            }
          } else if (polynomial[i] == cimag(polynomial[i])){
            if (printed == false){
              printf("%gi%c%s ", cimag(polynomial[i]), cvar, complex_function_get_exponent_str(polynomial_order - i));
            } else {
              printf("%+gi%c%s ", cimag(polynomial[i]), cvar, complex_function_get_exponent_str(polynomial_order - i));
            }
          } else {
            printf("(%+g %+gi)%c%s ", creal(polynomial[i]), cimag(polynomial[i]), cvar, complex_function_get_exponent_str(polynomial_order - i));
          }
          printed = true;
        }
        if (polynomial_parameters != NULL){
          if (polynomial_parameters[i] != 0){
            if (polynomial_parameters[i] == 1){
              if (printed == false){
                printf(" %c%c%s ", cpar, cvar, complex_function_get_exponent_str(polynomial_order-i));
              } else {
                printf("+ %c%c%s ", cpar, cvar, complex_function_get_exponent_str(polynomial_order-i));
              }
            } else if (polynomial_parameters[i] == -1){
              printf("- %c%c%s ", cpar, cvar, complex_function_get_exponent_str(polynomial_order-i));
            } else if (polynomial_parameters[i] == creal(polynomial_parameters[i])){
              if (printed == false){
                printf("%g%c%c%s ", creal(polynomial_parameters[i]), cpar, cvar, complex_function_get_exponent_str(polynomial_order - i));
              } else {
                printf("%+g%c%c%s ", creal(polynomial_parameters[i]), cpar, cvar, complex_function_get_exponent_str(polynomial_order - i));
              }
            } else if (polynomial_parameters[i] == cimag(polynomial_parameters[i])){
              if (printed == false){
                printf("%gi%c%c%s ", cimag(polynomial_parameters[i]), cpar, cvar, complex_function_get_exponent_str(polynomial_order - i));
              } else {
                printf("%+gi%c%c%s ", cimag(polynomial_parameters[i]), cpar, cvar, complex_function_get_exponent_str(polynomial_order - i));
              }
            } else {
              if (printed == false){
                printf("(%+g %+gi)%c%c%s ", creal(polynomial_parameters[i]), cimag(polynomial_parameters[i]), cpar, cvar, complex_function_get_exponent_str(polynomial_order - i));
              } else {
                printf("+(%+g %+gi)%c%c%s ", creal(polynomial_parameters[i]), cimag(polynomial_parameters[i]), cpar, cvar, complex_function_get_exponent_str(polynomial_order - i));
              }
            }
            printed = true;
          }
        }
      }
    }
    if (polynomial[polynomial_order] != 0){
      if (polynomial[polynomial_order] == creal(polynomial[polynomial_order])){
        if (printed == false){
          printf("%g ", creal(polynomial[polynomial_order]));
        } else {
          printf("%+g ", creal(polynomial[polynomial_order]));
        }
      } else if (polynomial[polynomial_order] == cimag(polynomial[polynomial_order])){
        if (printed == false){
          printf("%gi ", cimag(polynomial[polynomial_order]));
        } else {
          printf("%+gi ", cimag(polynomial[polynomial_order]));
        }
      } else {
        if (printed == false){
          printf("(%g %+gi) ", creal(polynomial[polynomial_order]), cimag(polynomial[polynomial_order]));
        } else {
          printf("+(%g %+gi) ", creal(polynomial[polynomial_order]), cimag(polynomial[polynomial_order]));
        }
      }
      printed = true;
    }
    if (polynomial_parameters != NULL){
      if (polynomial_parameters[polynomial_order] != 0){
        if (polynomial_parameters[polynomial_order] == creal(polynomial_parameters[polynomial_order])){
          if (printed == false){
            printf("%g%c ", creal(polynomial_parameters[polynomial_order]), cpar);
          } else {
            printf("%+g%c ", creal(polynomial_parameters[polynomial_order]), cpar);
          }
        } else if (polynomial_parameters[polynomial_order] == cimag(polynomial_parameters[polynomial_order])){
          if (printed == false){
            printf("%gi%c ", cimag(polynomial_parameters[polynomial_order]), cpar);
          } else {
            printf("%+gi%c ", cimag(polynomial_parameters[polynomial_order]), cpar);
          }
        } else {
          if (printed == false){
            printf("(%g %+gi)%c ", creal(polynomial_parameters[polynomial_order]), cimag(polynomial_parameters[polynomial_order]), cpar);
          } else {
            printf("+(%g %+gi)%c ", creal(polynomial_parameters[polynomial_order]), cimag(polynomial_parameters[polynomial_order]), cpar);
          }
        }
        printed = true;
      }
    }
    if (printed == false){
      printf("0");
    }
    printf("\n");
  } else {
    printf("(null)\n");
  }
}
char *complex_plane_polynomial_to_str(const complex double *polynomial, const complex double *polynomial_parameters, int polynomial_order, char cvar, char cpar){
  char *strret = calloc(sizeof(char) * 1024, 1);
  char buffer[64];
  _Bool printed = false;
  if (polynomial != NULL){
    for (int i = 0; i < polynomial_order; i++){
      if (i == polynomial_order - 1){
        if (polynomial[i] != 0){
          if (polynomial[i] == 1){
            if (printed == false){
              sprintf(buffer, " %c ", cvar);
            } else {
              sprintf(buffer, "+ %c ", cvar);
            }
            strcat(strret, buffer);
          } else if (polynomial[i] == -1){
            sprintf(buffer, "- %c ", cvar);
            strcat(strret, buffer);
          } else if (polynomial[i] == creal(polynomial[i])){
            if (printed == false){
              sprintf(buffer, "%g%c ", creal(polynomial[i]), cvar);
            } else {
              sprintf(buffer, "%+g%c ", creal(polynomial[i]), cvar);
            }
            strcat(strret, buffer);
          } else if (polynomial[i] == cimag(polynomial[i])){
            if (printed == false){
              sprintf(buffer, "%gi%c ", cimag(polynomial[i]), cvar);
            } else {
              sprintf(buffer, "%+gi%c ", cimag(polynomial[i]), cvar);
            }
            strcat(strret, buffer);
          } else {
            sprintf(buffer, "(%+g %+gi)%c ", creal(polynomial[i]), cimag(polynomial[i]), cvar);
            strcat(strret, buffer);
          }
          printed = true;
        }
        if (polynomial_parameters != NULL){
          if (polynomial_parameters[i] != 0){
            if (polynomial_parameters[i] == 1){
              if (printed == false){
                sprintf(buffer, " %c%c ", cpar, cvar);
              } else {
                sprintf(buffer, "+ %c%c ", cpar, cvar);
              }
              strcat(strret, buffer);
            } else if (polynomial_parameters[i] == -1){
              sprintf(buffer, "- %c%c ", cpar, cvar);
              strcat(strret, buffer);
            } else if (polynomial_parameters[i] == creal(polynomial_parameters[i])){
              if (printed == false){
                sprintf(buffer, "%g%c%c ", creal(polynomial_parameters[i]), cpar, cvar);
              } else {
                sprintf(buffer, "%+g%c%c ", creal(polynomial_parameters[i]), cpar, cvar);
              }
              strcat(strret, buffer);
            } else if (polynomial_parameters[i] == cimag(polynomial_parameters[i])){
              if (printed == false){
                sprintf(buffer, "%gi%c%c ", cimag(polynomial_parameters[i]), cpar, cvar);
              } else {
                sprintf(buffer, "%+gi%c%c ", cimag(polynomial_parameters[i]), cpar, cvar);
              }
              strcat(strret, buffer);
            } else {
              if (printed == false){
                sprintf(buffer, "(%+g %+gi)%c%c ", creal(polynomial_parameters[i]), cimag(polynomial_parameters[i]), cpar, cvar);
              } else {
                sprintf(buffer, "+(%+g %+gi)%c%c ", creal(polynomial_parameters[i]), cimag(polynomial_parameters[i]), cpar, cvar);
              }
              strcat(strret, buffer);
            }
            printed = true;
          }
        }
      } else {
        if (polynomial[i] != 0){
          if (polynomial[i] == 1){
            if (printed == false){
              sprintf(buffer, " %c%s ", cvar, complex_function_get_exponent_str(polynomial_order-i));
            } else {
              sprintf(buffer, "+ %c%s ", cvar, complex_function_get_exponent_str(polynomial_order-i));
            }
            strcat(strret, buffer);
          } else if (polynomial[i] == -1){
            sprintf(buffer, "- %c%s ", cvar, complex_function_get_exponent_str(polynomial_order-i));
            strcat(strret, buffer);
          } else if (polynomial[i] == creal(polynomial[i])){
            if (printed == false){
              sprintf(buffer, "%g%c%s ", creal(polynomial[i]), cvar, complex_function_get_exponent_str(polynomial_order - i));
            } else {
              sprintf(buffer, "%+g%c%s ", creal(polynomial[i]), cvar, complex_function_get_exponent_str(polynomial_order - i));
            }
            strcat(strret, buffer);
          } else if (polynomial[i] == cimag(polynomial[i])){
            if (printed == false){
              sprintf(buffer, "%gi%c%s ", cimag(polynomial[i]), cvar, complex_function_get_exponent_str(polynomial_order - i));
            } else {
              sprintf(buffer, "%+gi%c%s ", cimag(polynomial[i]), cvar, complex_function_get_exponent_str(polynomial_order - i));
            }
            strcat(strret, buffer);
          } else {
            sprintf(buffer, "(%+g %+gi)%c%s ", creal(polynomial[i]), cimag(polynomial[i]), cvar, complex_function_get_exponent_str(polynomial_order - i));
            strcat(strret, buffer);
          }
          printed = true;
        }
        if (polynomial_parameters != NULL){
          if (polynomial_parameters[i] != 0){
            if (polynomial_parameters[i] == 1){
              if (printed == false){
                sprintf(buffer, " %c%c%s ", cpar, cvar, complex_function_get_exponent_str(polynomial_order-i));
              } else {
                sprintf(buffer, "+ %c%c%s ", cpar, cvar, complex_function_get_exponent_str(polynomial_order-i));
              }
              strcat(strret, buffer);
            } else if (polynomial_parameters[i] == -1){
              sprintf(buffer, "- %c%c%s ", cpar, cvar, complex_function_get_exponent_str(polynomial_order-i));
              strcat(strret, buffer);
            } else if (polynomial_parameters[i] == creal(polynomial_parameters[i])){
              if (printed == false){
                sprintf(buffer, "%g%c%c%s ", creal(polynomial_parameters[i]), cpar, cvar, complex_function_get_exponent_str(polynomial_order - i));
              } else {
                sprintf(buffer, "%+g%c%c%s ", creal(polynomial_parameters[i]), cpar, cvar, complex_function_get_exponent_str(polynomial_order - i));
              }
              strcat(strret, buffer);
            } else if (polynomial_parameters[i] == cimag(polynomial_parameters[i])){
              if (printed == false){
                sprintf(buffer, "%gi%c%c%s ", cimag(polynomial_parameters[i]), cpar, cvar, complex_function_get_exponent_str(polynomial_order - i));
              } else {
                sprintf(buffer, "%+gi%c%c%s ", cimag(polynomial_parameters[i]), cpar, cvar, complex_function_get_exponent_str(polynomial_order - i));
              }
              strcat(strret, buffer);
            } else {
              if (printed == false){
                sprintf(buffer, "(%+g %+gi)%c%c%s ", creal(polynomial_parameters[i]), cimag(polynomial_parameters[i]), cpar, cvar, complex_function_get_exponent_str(polynomial_order - i));
              } else {
                sprintf(buffer, "+(%+g %+gi)%c%c%s ", creal(polynomial_parameters[i]), cimag(polynomial_parameters[i]), cpar, cvar, complex_function_get_exponent_str(polynomial_order - i));
              }
              strcat(strret, buffer);
            }
            printed = true;
          }
        }
      }
    }
    if (polynomial[polynomial_order] != 0){
      if (polynomial[polynomial_order] == creal(polynomial[polynomial_order])){
        if (printed == false){
          sprintf(buffer, "%g ", creal(polynomial[polynomial_order]));
        } else {
          sprintf(buffer, "%+g ", creal(polynomial[polynomial_order]));
        }
        strcat(strret, buffer);
      } else if (polynomial[polynomial_order] == cimag(polynomial[polynomial_order])){
        if (printed == false){
          sprintf(buffer, "%gi ", cimag(polynomial[polynomial_order]));
        } else {
          sprintf(buffer, "%+gi ", cimag(polynomial[polynomial_order]));
        }
        strcat(strret, buffer);
      } else {
        if (printed == false){
          sprintf(buffer, "(%g %+gi) ", creal(polynomial[polynomial_order]), cimag(polynomial[polynomial_order]));
        } else {
          sprintf(buffer, "+(%g %+gi) ", creal(polynomial[polynomial_order]), cimag(polynomial[polynomial_order]));
        }
        strcat(strret, buffer);
      }
      printed = true;
    }
    if (polynomial_parameters != NULL){
      if (polynomial_parameters[polynomial_order] != 0){
        if (polynomial_parameters[polynomial_order] == creal(polynomial_parameters[polynomial_order])){
          if (printed == false){
            sprintf(buffer, "%g%c ", creal(polynomial_parameters[polynomial_order]), cpar);
          } else {
            sprintf(buffer, "%+g%c ", creal(polynomial_parameters[polynomial_order]), cpar);
          }
          strcat(strret, buffer);
        } else if (polynomial_parameters[polynomial_order] == cimag(polynomial_parameters[polynomial_order])){
          if (printed == false){
            sprintf(buffer, "%gi%c ", cimag(polynomial_parameters[polynomial_order]), cpar);
          } else {
            sprintf(buffer, "%+gi%c ", cimag(polynomial_parameters[polynomial_order]), cpar);
          }
          strcat(strret, buffer);
        } else {
          if (printed == false){
            sprintf(buffer, "(%g %+gi)%c ", creal(polynomial_parameters[polynomial_order]), cimag(polynomial_parameters[polynomial_order]), cpar);
          } else {
            sprintf(buffer, "+(%g %+gi)%c ", creal(polynomial_parameters[polynomial_order]), cimag(polynomial_parameters[polynomial_order]), cpar);
          }
          strcat(strret, buffer);
        }
        printed = true;
      }
    }
    if (printed == false){
      strcpy(strret, "0");
    }
  } else {
    strcpy(strret, "(null)");
  }

  return strret;
}
void complex_plane_print_polynomial_parameters_derivative(ComplexPlane *cp){
  for (int i = 0; i < cp->polynomial_order; i++){
    if (i == cp->polynomial_order){
      printf("(%g %+g)z ", creal(cp->polynomial_parameters_derivative[i]), cimag(cp->polynomial_parameters_derivative[i]));
    } else {
      printf("(%g %+g)z^%d ", creal(cp->polynomial_parameters_derivative[i]), cimag(cp->polynomial_parameters_derivative[i]), cp->polynomial_order - i);
    }
    printf("+ ");
  }
  printf("(%g %+g)", creal(cp->polynomial_parameters_derivative[cp->polynomial_order]), cimag(cp->polynomial_parameters_derivative[cp->polynomial_order]));
  printf("\n");
}
void complex_plane_set_parameters_vector_member(ComplexPlane *cp, complex double v, int i){
  int order = cp->polynomial_order;
  cp->polynomial_parameters[i] = v;
  if (i < order){
    complex_plane_set_polynomial_parameters_derivative_member(cp, (order - i) * v, i + 1);
  }
  if (i < order -1){
    complex_plane_set_second_polynomial_parameters_derivative_member(cp, (order - i - 1)*(order - i) * v, i + 2);
  }
}
complex double complex_plane_get_parameters_vector_member(ComplexPlane *cp, int i){
  return cp->polynomial_parameters[i];
}


//---iterations
void complex_plane_set_iterations(ComplexPlane *cp, int N){
  cp->N = N;
}
void complex_plane_set_line_iterations(ComplexPlane *cp, int N){
  cp->N_line = N;
}
int complex_plane_get_line_iterations(ComplexPlane *cp){
  return cp->N_line;
}
int complex_plane_get_iterations(ComplexPlane *cp){
  return cp->N;
}

//---Parameters
void complex_plane_set_quadratic_parameter(ComplexPlane *cp, complex double p){
  cp->param = p;
}
void complex_plane_set_quadratic_parameter_real(ComplexPlane *cp, double x){
  cp->param = x + cimag(cp->param)*I;
}
void complex_plane_set_quadratic_parameter_imag(ComplexPlane *cp, double x){
  cp->param = creal(cp->param) + x*I;
}
complex double complex_plane_get_quadratic_parameter(ComplexPlane *cp){
  return(cp->param);
}
double complex_plane_get_quadratic_parameter_real(ComplexPlane *cp){
  return(creal(complex_plane_get_quadratic_parameter(cp)));
}
double complex_plane_get_quadratic_parameter_imag(ComplexPlane *cp){
  return(cimag(complex_plane_get_quadratic_parameter(cp)));
}
void complex_plane_set_numerical_method_a(ComplexPlane *cp, complex double a){
  cp->numerical_method_a = a;
}
complex double complex_plane_get_numerical_method_a(ComplexPlane *cp){
  return cp->numerical_method_a;
}

//---center
void complex_plane_set_center(ComplexPlane *cp, complex double p){
  cp->center = p;
  complex_plane_set_spanx(cp, complex_plane_get_spanx(cp));
  complex_plane_set_spany(cp, complex_plane_get_spany(cp));
}
void complex_plane_set_center_real(ComplexPlane *cp, double x){
  cp->center = x + cimag(cp->center)*I;
  complex_plane_set_spanx(cp, complex_plane_get_spanx(cp));
  complex_plane_set_spany(cp, complex_plane_get_spany(cp));
}
void complex_plane_set_center_imag(ComplexPlane *cp, double x){
  cp->center = creal(cp->center) + x*I;
  complex_plane_set_spanx(cp, complex_plane_get_spanx(cp));
  complex_plane_set_spany(cp, complex_plane_get_spany(cp));
}
complex double complex_plane_get_center(ComplexPlane *cp){
  return(cp->center);
}
double complex_plane_get_center_real(ComplexPlane *cp){
  return(creal(complex_plane_get_center(cp)));
}
double complex_plane_get_center_imag(ComplexPlane *cp){
  return(cimag(complex_plane_get_center(cp)));
}

//---spans
void complex_plane_set_spanx(ComplexPlane *cp, double s){
  cp->Sx[0] = (-s/2) + complex_plane_get_center_real(cp);
  cp->Sx[1] = (s/2)  + complex_plane_get_center_real(cp);
  cp->SpanX = s;
}
void complex_plane_set_spany(ComplexPlane *cp, double s){
  cp->Sy[0] = (-s/2) + complex_plane_get_center_imag(cp);
  cp->Sy[1] = (s/2)  + complex_plane_get_center_imag(cp);
  cp->SpanY = s;
}
void complex_plane_adjust_span_ratio(ComplexPlane *cp){
  int w = complex_plane_get_width(cp);
  int h = complex_plane_get_height(cp);
  if (w >= h){
    double ratio = (double) w / (double) h;
    complex_plane_set_spanx(cp, complex_plane_get_spany(cp)*ratio);
  } else {
    double ratio = (double) h / (double) w;
    complex_plane_set_spany(cp, complex_plane_get_spanx(cp)*ratio);
  }
}
void complex_plane_set_default_spans(ComplexPlane *cp){
  complex_plane_set_spanx(cp, 5.2);
  complex_plane_set_spany(cp, 3);
  complex_plane_adjust_span_ratio(cp);
}
double complex_plane_get_spanx(ComplexPlane *cp){
  return cp->SpanX;
}
double complex_plane_get_spany(ComplexPlane *cp){
  return cp->SpanY;
}
double complex_plane_get_spanx0(ComplexPlane *cp){
  return cp->Sx[0];
}
double complex_plane_get_spanx1(ComplexPlane *cp){
  return cp->Sx[1];
}
double complex_plane_get_spany0(ComplexPlane *cp){
  return cp->Sy[0];
}
double complex_plane_get_spany1(ComplexPlane *cp){
  return cp->Sy[1];
}
double *complex_plane_get_spanx_array(ComplexPlane *cp){
  double *Sx;
  Sx = malloc(sizeof *Sx *2);
  Sx[0] = cp->Sx[0]; Sx[1] = cp->Sx[1];
  return Sx;
} //TODO: Possible memory leak.
double *complex_plane_get_spany_array(ComplexPlane *cp){
  double *Sy;
  Sy = malloc(sizeof *Sy *2);
  Sy[0] = cp->Sy[0]; Sy[1] = cp->Sy[1];
  return Sy;
} //TODO: Possible memory leak.

//---drawing box
void complex_plane_set_zoom_point1(ComplexPlane *cp, double x, double y){
  complex_plane_free_zoom_point1(cp);
  cp->zoom_point1[0] = malloc(sizeof(double));
  cp->zoom_point1[1] = malloc(sizeof(double));
  *cp->zoom_point1[0] = x;
  *cp->zoom_point1[1] = y;
}
void complex_plane_set_zoom_point2(ComplexPlane *cp, double x, double y){
  complex_plane_free_zoom_point2(cp);
  cp->zoom_point2[0] = malloc(sizeof(double));
  cp->zoom_point2[1] = malloc(sizeof(double));
  *cp->zoom_point2[0] = x;
  *cp->zoom_point2[1] = y;
}
void complex_plane_free_zoom_point1(ComplexPlane *cp){
  if (cp->zoom_point1[0] != NULL){
  if (cp->zoom_point1[1] != NULL){
    free(cp->zoom_point1[0]);
    free(cp->zoom_point1[1]);
    cp->zoom_point1[0] = NULL;
    cp->zoom_point1[1] = NULL;
  }
  }
}
void complex_plane_free_zoom_point2(ComplexPlane *cp){
  if (cp->zoom_point2[0] != NULL){
  if (cp->zoom_point2[1] != NULL){
    free(cp->zoom_point2[0]);
    free(cp->zoom_point2[1]);
    cp->zoom_point2[0] = NULL;
    cp->zoom_point2[1] = NULL;
  }
  }
}
_Bool complex_plane_zoom_point1_is_null(ComplexPlane *cp){
  if (cp->zoom_point1[0] == NULL && cp->zoom_point1[1] == NULL){
    return true;
  }
  return false;
}
_Bool complex_plane_zoom_point2_is_null(ComplexPlane *cp){
  if (cp->zoom_point2[0] == NULL && cp->zoom_point2[1] == NULL){
    return true;
  }
  return false;
}
void complex_plane_zoom_points_normalize(ComplexPlane *cp){
  double aux;
  if (*cp->zoom_point1[0] > *cp->zoom_point2[0]){
      aux = *cp->zoom_point1[0];
      *cp->zoom_point1[0] = *cp->zoom_point2[0];
      *cp->zoom_point2[0] = aux;
  }
  if (*cp->zoom_point1[1] > *cp->zoom_point2[1]){
    aux = *cp->zoom_point1[1];
    *cp->zoom_point1[1] = *cp->zoom_point2[1];
    *cp->zoom_point2[1] = aux;
  }
}
double complex_plane_zoom_point_get_spanx(ComplexPlane *cp){
  return *cp->zoom_point2[0] - *cp->zoom_point1[0];
}
double complex_plane_zoom_point_get_spany(ComplexPlane *cp){
  return *cp->zoom_point2[1] - *cp->zoom_point1[1];
}
void complex_plane_zoom_points_set_center(ComplexPlane *cp){
    complex_plane_set_center(cp, ((*cp->zoom_point2[0] + *cp->zoom_point1[0])/2) + ((*cp->zoom_point2[1] + *cp->zoom_point1[1])/2)*I);
}
int complex_plane_zoom_point1_get_pixel_value_x(ComplexPlane *cp){
    int w = complex_plane_get_width(cp);
    double spanx = complex_plane_get_spanx(cp);

    return  (int) floor((*cp->zoom_point1[0] - complex_plane_get_spanx0(cp)) / spanx * w);
}
int complex_plane_zoom_point1_get_pixel_value_y(ComplexPlane *cp){
    int h = complex_plane_get_height(cp);
    double spany = complex_plane_get_spany(cp);
    return (int) floor((-(*cp->zoom_point1[1] - 2*complex_plane_get_center_imag(cp))
                          - complex_plane_get_spany0(cp)) / spany * h);
}
int complex_plane_zoom_point2_get_pixel_value_x(ComplexPlane *cp){
    int w = complex_plane_get_width(cp);
    double spanx = complex_plane_get_spanx(cp);

    return  (int) floor((*cp->zoom_point2[0] - complex_plane_get_spanx0(cp)) / spanx * w);
}
int complex_plane_zoom_point2_get_pixel_value_y(ComplexPlane *cp){
    int h = complex_plane_get_height(cp);
    double spany = complex_plane_get_spany(cp);
    return (int) floor((-(*cp->zoom_point2[1] - 2*complex_plane_get_center_imag(cp))
                          - complex_plane_get_spany0(cp)) / spany * h);
}





void complex_plane_set_mandelbrot_parameters(ComplexPlane *cp){
  complex_plane_set_iterations(cp, 500);
  complex_plane_set_line_iterations(cp, 25);
  complex_plane_set_plot_type(cp, COMPLEX_PLANE_PARAMETER_SPACE);
  complex_plane_set_quadratic_parameter(cp, 0);
  complex_plane_set_center(cp, 0);
  complex_plane_set_default_spans(cp);
  complex_plane_set_stride(cp, 3);
}

//---Plots
void complex_plane_gen_plot(ComplexPlane *cp){
  complex_plane_free_plot(cp);
  complex_plane_free_drawn_plot(cp);

  #ifdef DEBUG_CP
  printf("CP %d Spans: %f %f | %f %f\n", cp->ID, complex_plane_get_spanx0(cp),
                                                  complex_plane_get_spanx1(cp),
                                                  complex_plane_get_spany0(cp),
                                                  complex_plane_get_spany1(cp));

  #endif

  switch(cp->function_type){
    case 0:   //Simple quadratic family
      cp->plot = draw_julia(cp->N,
                            cp->h,
                            cp->w,
                            cp->param,
                            cp->Sx,
                            cp->Sy,
                            cp->plot_type,
                            cp->colorscheme,
                            &(cp->cl), !(cp->cl->init));
      break;
    case 1:   //Polynomial function
      if (complex_plane_get_polynomial_order(cp) != -1){
        cp->plot = draw_julia_polynomial(cp->N,
                                         cp->h,
                                         cp->w,
                                         cp->polynomial_order,
                                         cp->polynomial,
                                         cp->Sx,
                                         cp->Sy,
                                         cp->polynomial_parameter,
                                         cp->colorscheme,
                                         &(cp->cl), !cp->cl->init);
      } else {
        complex_plane_alloc_empty_plot(cp);
      }
      break;
    case 2:   //Newton's fractal
      if (complex_plane_get_polynomial_order(cp) != -1){
        cp->plot = draw_julia_polynomial_fraction(cp->N,
                                                  cp->h,
                                                  cp->w,
                                                  cp->polynomial_order,
                                                  cp->polynomial,
                                                  cp->polynomial_derivative,
                                                  cp->Sx,
                                                  cp->Sy,
                                                  cp->polynomial_parameter,
                                                  cp->colorscheme,
                                                  &(cp->cl), !cp->cl->init);
      }
      break;
    case 3:   //Newton's method
      if (complex_plane_get_polynomial_order(cp) != -1){
        cp->plot = draw_julia_numerical_method(cp->N,
                                               cp->h, cp->w,
                                               cp->polynomial_order,
                                               cp->plot_type,
                                               cp->polynomial,
                                               cp->polynomial_derivative,
                                               cp->second_polynomial_derivative,
                                               cp->polynomial_parameters,
                                               cp->polynomial_parameters_derivative,
                                               cp->second_polynomial_parameters_derivative,
                                               cp->polynomial_critical_point,
                                               cp->numerical_method_a,
                                               cp->nroots, cp->roots,
                                               cp->Sx, cp->Sy,
                                               cp->colorscheme,
                                               &(cp->cl), !cp->cl->init);
      }
      break;
    default:
      cp->plot = calloc(complex_plane_get_size(cp), 1);
      break;
  }
}


void complex_plane_alloc_empty_plot(ComplexPlane *cp){
  complex_plane_free_plot(cp);

  #ifdef DEBUG_CP
    printf("CP %d: Empty\n", cp->ID);
  #endif

  cp->plot = calloc(complex_plane_get_size(cp) * sizeof(unsigned char), 1);
}
void complex_plane_alloc_drawn_plot(ComplexPlane *cp){
  complex_plane_free_drawn_plot(cp);

  #ifdef DEBUG_CP
    printf("CP %d: Empty drawn\n", cp->ID);
  #endif

  cp->drawn_plot = calloc(complex_plane_get_size(cp) * sizeof(unsigned char), 1);
}
int complex_plane_free_plot(ComplexPlane *cp){
  if (cp->plot != NULL){
    #ifdef DEBUG_CP
      printf("CP %d: Freeing plot\n", cp->ID);
    #endif

    free(cp->plot);
    cp->plot = NULL;
    return 1;
  }
  return 0;
}
int complex_plane_free_drawn_plot(ComplexPlane *cp){
  if (cp->drawn_plot != NULL){
    #ifdef DEBUG_CP
      printf("CP %d: Freeing drawn\n", cp->ID);
    #endif
    free(cp->drawn_plot);
    cp->drawn_plot = NULL;
    return 1;
  }
  return 0;
}
const unsigned char *complex_plane_get_plot(ComplexPlane *cp){
  return cp->plot;
}
unsigned char *complex_plane_get_drawn_plot(ComplexPlane *cp){
  return cp->drawn_plot;
}
int complex_plane_copy_plot(ComplexPlane *cp){
  if (cp->drawn_plot == NULL || cp->plot == NULL){
    return -1;
  }
  complex_plane_free_drawn_plot(cp);
  #ifdef DEBUG_CP
    printf("CP %d: Copying plot to drawn\n", cp->ID);
  #endif
  cp->drawn_plot = image_manipulation_clone_image(cp->plot, cp->w, cp->h,
                                                &(cp->cl), !(cp->cl->init));
  return 0;
}

double complex_plane_get_norm(complex double z){
   return (pow(pow(creal(z), 2) + pow(cimag(z), 3), 0.5));
}
double complex_plane_thumbnail_get_span(ComplexPlane *cp){
  switch(cp->function_type){
    case 0:
      return fmax(complex_plane_get_norm(cp->param), 3);
      break;
    case 1:
      if (complex_plane_get_polynomial_order(cp) != -1 && !complex_plane_polynomial_is_null(cp)){
        return fmax(complex_plane_get_norm(cp->polynomial[cp->polynomial_order]), 2);
      }
      break;
    default:
      return 2;
  }
  return 2;

}

void draw_sequence_lines(struct ComplexPlane *C, double point[2], int w, int h){
  double c[2], p[2];
  int x, y, oldx, oldy;

  if (C->plot_type == COMPLEX_PLANE_PARAMETER_SPACE){
    p[0] = creal(C->param);
    p[1] = cimag(C->param);
    c[0] = point[0];
    c[1] = point[1];
    x = (int) floor((p[0]                              - C->Sx[0])/(C->Sx[1]-C->Sx[0]) * w);
    y = (int) floor((-(p[1]-complex_plane_get_center_imag(C) - ((C->Sy[0]+C->Sy[1])/2)) - C->Sy[0])/(C->Sy[1]-C->Sy[0]) * h);
  } else if (C->plot_type == COMPLEX_PLANE_DYNAMIC_PLANE){
    p[0] = point[0];
    p[1] = point[1];
    c[0] = creal(C->param);
    c[1] = cimag(C->param);
    x = (int) floor((p[0]                              - C->Sx[0])/(C->Sx[1]-C->Sx[0]) * w);
    y = (int) floor((-(p[1]-complex_plane_get_center_imag(C) - ((C->Sy[0]+C->Sy[1])/2)) - C->Sy[0])/(C->Sy[1]-C->Sy[0]) * h);
  } else {
    return;
  }

  for (int i = 0; i < C->N_line; i++){
    oldx = x;        oldy = y;

    double aux = (pow(p[0], 2) - pow(p[1], 2)) + c[0];
    p[1] = 2*p[0]*p[1] + c[1];
    p[0] = aux;

    x = (int) floor((p[0] - C->Sx[0])/(C->Sx[1]-C->Sx[0]) * w);
    y = (int) floor((-(p[1]-complex_plane_get_center_imag(C)-((C->Sy[0]+C->Sy[1])/2)) - C->Sy[0])/(C->Sy[1]-C->Sy[0]) * h);

    draw_line(C->drawn_plot, x, y, oldx, oldy, w, h);
    if (x >= C->w*2 || y >= C->h*2 ||
        x < -C->w   || y < -C->h   ){
      break;
    }
  }
}

complex complex_mul(complex a, complex b){
  return (((creal(a) * creal(b)) - (cimag(a) * cimag(b))) +
          ((creal(a) * cimag(b)) + (cimag(a) * creal(b)))*I);
}

complex complex_div(complex a, complex b){
  double den  = (pow(creal(b), 2) + pow(cimag(b), 2));
  double num1 = (creal(a)*creal(b) + cimag(a)*cimag(b));
  double num2 = (cimag(a)*creal(b) - creal(a)*cimag(b));

  return (num1 / den) + (num2 / den)*I;
}
complex complex_compute_polynomial(const complex double *polynomial,
                                  int order, complex param, complex z,
                                  int parameter){
  complex auxz = 0, aux = 0;

  for (int j = order; j >= 0; j--){
    if (j == order){            //Add c
      if (parameter == order){  //c is my param
        auxz = param;
      } else {                  //param is somewere else
        auxz = polynomial[j];
      }
    } else if (j == order-1){
      if (parameter == order + 1){  //z is param
        if (polynomial[j] != 0){
          auxz += complex_mul(param, polynomial[j]);
        }
      } else if (parameter == j){   //param is a
        auxz += complex_mul(param, z);
      } else {    //param is somewhere else
        if (polynomial[j] != 0){
          auxz += complex_mul(polynomial[j], z);
        }
      }
    } else {
      if (parameter == order + 1) { //param is z
        if (polynomial[j] != 0){
          aux = param;
          for (int k = 0; k < order-j-1; k++){
            aux = complex_mul(aux, param);
          }
          auxz += complex_mul(aux, polynomial[j]);
        }
      } else if (parameter == j){ //parameter is a
        aux = z;
        for (int k = 0; k < order-j-1; k++){
          aux = complex_mul(aux, z);
        }
        auxz += complex_mul(aux, param);
      } else {    //param is somewhere else
        if (polynomial[j] != 0){
          aux = z;
          for (int k = 0; k < order-j-1; k++){
            aux = complex_mul(aux, z);
          }
          auxz += complex_mul(aux, polynomial[j]);
        }
      }
    }
  }
  return auxz;
}
complex complex_compute_polynomial_p(const complex double *polynomial, complex double param, complex double z, int order){
  complex double auxz = 0, aux = 0;
  for (int j = order; j >= 0; j--){
    if (j == order){
      auxz = complex_mul(polynomial[j], param);
    } else if (j == order -1){
      if ((polynomial[j] != 0) && (param != 0)){
        aux = complex_mul(polynomial[j], z);
        auxz += complex_mul(aux, param);
      }
    } else {
      if (polynomial[j] != 0 && param != 0){
        aux = z;
        for (int k = 0; k < order-j-1; k++){
          aux = complex_mul(aux, z);
        }
        aux = complex_mul(aux, polynomial[j]);
        aux = complex_mul(aux, param);
        auxz += aux;
      }
    }
  }
  return auxz;
}

void complex_plane_reset_opencl(ComplexPlane *cp){
  cp->cl->init = false;
}


void draw_sequence_lines_polynomial(struct ComplexPlane *C,  double point[2], int w, int h){
  complex param, z;
  int x, y, oldx, oldy;

  const complex double *polynomial = C->polynomial;
  int order = C->polynomial_order;

  int parameter = C->polynomial_parameter;

  param = point[0] + point[1]*I;

  z = polynomial[order + 1];

  if (parameter == order + 1){
    x = (int) floor((creal(param)                              - C->Sx[0])/(C->Sx[1]-C->Sx[0]) * w);
    y = (int) floor((-(cimag(param)-complex_plane_get_center_imag(C) - ((C->Sy[0]+C->Sy[1])/2)) - C->Sy[0])/(C->Sy[1]-C->Sy[0]) * h);
  } else {
    x = (int) floor((creal(z)                              - C->Sx[0])/(C->Sx[1]-C->Sx[0]) * w);
    y = (int) floor((-(cimag(z)-complex_plane_get_center_imag(C) - ((C->Sy[0]+C->Sy[1])/2)) - C->Sy[0])/(C->Sy[1]-C->Sy[0]) * h);
  }

  for (int i = 0; i < C->N_line; i++){
    oldx = x; oldy = y;
    complex auxz = 0;

    auxz = complex_compute_polynomial(polynomial, order, param, z, parameter);

    if (parameter == order + 1){    //z is param
      param = auxz;
    } else {  //z is z
      z = auxz;
    }

    if (parameter == order + 1){
      x = (int) floor((creal(param) - C->Sx[0])/(C->Sx[1]-C->Sx[0]) * w);
      y = (int) floor((-(cimag(param)-complex_plane_get_center_imag(C)-((C->Sy[0]+C->Sy[1])/2)) - C->Sy[0])/(C->Sy[1]-C->Sy[0]) * h);
    } else {
      x = (int) floor((creal(z) - C->Sx[0])/(C->Sx[1]-C->Sx[0]) * w);
      y = (int) floor((-(cimag(z)-complex_plane_get_center_imag(C)-((C->Sy[0]+C->Sy[1])/2)) - C->Sy[0])/(C->Sy[1]-C->Sy[0]) * h);
    }

    draw_line(C->drawn_plot, x, y, oldx, oldy, w, h);
    if (x >= C->w*2 || y >= C->h*2 ||
        x < -C->w   || y < -C->h   ){
      break;
    }
  }
}

void draw_sequence_lines_newton(ComplexPlane *C, double p[2], int w, int h){
  complex param, z;
  int parameter = C->polynomial_parameter;
  int x, y, oldx, oldy;

  int order = C->polynomial_order;
  const complex double *numerator = C->polynomial;
  const complex double *denominator = C->polynomial_derivative;

  param = p[0] + p[1]*I;

  z = numerator[order + 1];

  if (parameter == order + 1){  //Dynamic plane
    x = (int) floor((creal(param)                              - C->Sx[0])/(C->Sx[1]-C->Sx[0]) * w);
    y = (int) floor((-(cimag(param)-complex_plane_get_center_imag(C) - ((C->Sy[0]+C->Sy[1])/2)) - C->Sy[0])/(C->Sy[1]-C->Sy[0]) * h);
  } else {    //Parameter space
    x = (int) floor((creal(z)                              - C->Sx[0])/(C->Sx[1]-C->Sx[0]) * w);
    y = (int) floor((-(cimag(z)-complex_plane_get_center_imag(C) - ((C->Sy[0]+C->Sy[1])/2)) - C->Sy[0])/(C->Sy[1]-C->Sy[0]) * h);
  }

  for (int i = 0; i < C->N_line; i++){
    oldx = x; oldy = y;
    complex auxn = 0, auxd = 0;
    complex auxz = 0;

    if (parameter == order){
      auxd = complex_compute_polynomial(denominator, order, param, z, -1);
    } else if (parameter == order + 1){
      auxd = complex_compute_polynomial(denominator, order, param, z, parameter);
    } else {
      auxd = complex_compute_polynomial(denominator, order, param, z, parameter + 1);
    }

    if (auxd == 0){
      break;
    }

    auxn = complex_compute_polynomial(numerator, order, param, z, parameter);

    auxz = complex_div(auxn, auxd);
    if (parameter == order + 1){    //z is param
      param = param - auxz;
    } else {  //z is z
      z = z - auxz;
    }
    if (parameter == order + 1){
      x = (int) floor((creal(param) - C->Sx[0])/(C->Sx[1]-C->Sx[0]) * w);
      y = (int) floor((-(cimag(param)-complex_plane_get_center_imag(C)-((C->Sy[0]+C->Sy[1])/2)) - C->Sy[0])/(C->Sy[1]-C->Sy[0]) * h);
    } else {
      x = (int) floor((creal(z) - C->Sx[0])/(C->Sx[1]-C->Sx[0]) * w);
      y = (int) floor((-(cimag(z)-complex_plane_get_center_imag(C)-((C->Sy[0]+C->Sy[1])/2)) - C->Sy[0])/(C->Sy[1]-C->Sy[0]) * h);
    }

    draw_line(C->drawn_plot, x, y, oldx, oldy, w, h);
  }
}


void draw_sequence_lines_numerical_method(ComplexPlane *cp, double p[2], int w, int h){
  complex double a, z;

  int plot_type = complex_plane_get_plot_type(cp);
  int order = cp->polynomial_order;
  int x, y, oldx, oldy;

  if (plot_type == COMPLEX_PLANE_PARAMETER_SPACE){
    a = p[0] + p[1] * I;
    z = complex_compute_polynomial_p(cp->polynomial_critical_point, 1, a, order); //Salgo del critico
  } else {
    a = complex_plane_get_numerical_method_a(cp);
    z = p[0] + p[1] * I;
  }

  x = (int) floor((creal(z) - cp->Sx[0])/(cp->Sx[1]-cp->Sx[0])*w);
  y = (int) floor((-(cimag(z)-complex_plane_get_center_imag(cp) - ((cp->Sy[0]+cp->Sy[1])/2)) - cp->Sy[0])/(cp->Sy[1]-cp->Sy[0]) * h);

  for (int i = 0; i < cp->N_line; i++){
    oldx = x; oldy = y;

    z = newton_method(cp->polynomial, cp->polynomial_derivative, cp->polynomial_parameters, cp->polynomial_parameters_derivative, z, a, order);

    if (z == INFINITY){
      #ifdef DEBUG_CP
        printf("Error: Newton's method returned infinity.\n");
      #endif

        return;
    }

    x = (int) floor((creal(z) - cp->Sx[0])/(cp->Sx[1]-cp->Sx[0])*w);
    y = (int) floor((-(cimag(z)-complex_plane_get_center_imag(cp) - ((cp->Sy[0]+cp->Sy[1])/2)) - cp->Sy[0])/(cp->Sy[1]-cp->Sy[0]) * h);

    draw_line(cp->drawn_plot, x, y, oldx, oldy, w, h);
  }
}

complex newton_method(const complex double *polynomial,
                      const complex double *polynomial_derivative,
                      const complex double *polynomial_param,
                      const complex double *polynomial_param_derivative,
                      complex double z, complex double a, int order){
  complex double den, num;

  den = complex_compute_polynomial_p(polynomial_derivative, 1, z, order);
  den += complex_compute_polynomial_p(polynomial_param_derivative, a, z, order);

  if (den == 0){
    return INFINITY;
  }

  num = complex_compute_polynomial_p(polynomial, 1, z, order);
  num += complex_compute_polynomial_p(polynomial_param, a, z, order);

  return z - complex_div(num, den);
}
