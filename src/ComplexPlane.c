#include <ComplexPlane.h>
#include <image_manipulation.h>
#include <math.h>
#include <string.h>

void draw_sequence_lines(struct ComplexPlane *C, double point[2], int w, int h){
  double c[2], p[2], old_p[2];
  int x, y, oldx, oldy;

  if (strcmp(C->plot_type, "parameter_space") == 0){
    p[0] = C->z[0];
    p[1] = C->z[1];
    c[0] = point[0];
    c[1] = point[1];
    x = (int) floor((p[0]                              - C->Sx[0])/(C->Sx[1]-C->Sx[0]) * w);
    y = (int) floor((-(p[1]-C->center[1] - ((C->Sy[0]+C->Sy[1])/2)) - C->Sy[0])/(C->Sy[1]-C->Sy[0]) * h);
  } else if (strcmp(C->plot_type, "rec_f") == 0){
    p[0] = point[0];
    p[1] = point[1];
    c[0] = C->z[0];
    c[1] = C->z[1];
    x = (int) floor((p[0]                              - C->Sx[0])/(C->Sx[1]-C->Sx[0]) * w);
    y = (int) floor((-(p[1]-C->center[1] - ((C->Sy[0]+C->Sy[1])/2)) - C->Sy[0])/(C->Sy[1]-C->Sy[0]) * h);
  }

  for (int i = 0; i < C->N_line; i++){
    old_p[0] = p[0]; old_p[1] = p[1];
    oldx = x;        oldy = y;

    double aux = (pow(p[0], 2) - pow(p[1], 2)) + c[0];
    p[1] = 2*p[0]*p[1] + c[1];
    p[0] = aux;

    x = (int) floor((p[0] - C->Sx[0])/(C->Sx[1]-C->Sx[0]) * w);
    y = (int) floor((-(p[1]-C->center[1]-((C->Sy[0]+C->Sy[1])/2)) - C->Sy[0])/(C->Sy[1]-C->Sy[0]) * h);

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

void draw_sequence_lines_polynomial(struct ComplexPlane *C, complex *polynomial, int order, double point[2], int w, int h){
  complex param, old_p, c, z;
  int x, y, oldx, oldy;

  //TODO: parameter space
  int parameter = C->polynomial_parameter;

  param = point[0] + point[1]*I;

  z = polynomial[order + 1];
  c = polynomial[order];

  if (parameter == order + 1){
    x = (int) floor((creal(param)                              - C->Sx[0])/(C->Sx[1]-C->Sx[0]) * w);
    y = (int) floor((-(cimag(param)-C->center[1] - ((C->Sy[0]+C->Sy[1])/2)) - C->Sy[0])/(C->Sy[1]-C->Sy[0]) * h);
  } else {
    x = (int) floor((creal(z)                              - C->Sx[0])/(C->Sx[1]-C->Sx[0]) * w);
    y = (int) floor((-(cimag(z)-C->center[1] - ((C->Sy[0]+C->Sy[1])/2)) - C->Sy[0])/(C->Sy[1]-C->Sy[0]) * h);
  }

  for (int i = 0; i < C->N_line; i++){
    old_p = z;
    oldx = x; oldy = y;
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
    if (parameter == order + 1){    //z is param
      param = auxz;
    } else {  //z is z
      z = auxz;
    }

    if (parameter == order + 1){
      x = (int) floor((creal(param) - C->Sx[0])/(C->Sx[1]-C->Sx[0]) * w);
      y = (int) floor((-(cimag(param)-C->center[1]-((C->Sy[0]+C->Sy[1])/2)) - C->Sy[0])/(C->Sy[1]-C->Sy[0]) * h);
    } else {
      x = (int) floor((creal(z) - C->Sx[0])/(C->Sx[1]-C->Sx[0]) * w);
      y = (int) floor((-(cimag(z)-C->center[1]-((C->Sy[0]+C->Sy[1])/2)) - C->Sy[0])/(C->Sy[1]-C->Sy[0]) * h);
    }

    draw_line(C->drawn_plot, x, y, oldx, oldy, w, h);
    if (x >= C->w*2 || y >= C->h*2 ||
        x < -C->w   || y < -C->h   ){
      break;
    }
  }

}
