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
  complex p, old_p, c;
  int x, y, oldx, oldy;

  //TODO: parameter space
  p = point[0] + point[1]*I;
  c = polynomial[order];

  x = (int) floor((creal(p)                              - C->Sx[0])/(C->Sx[1]-C->Sx[0]) * w);
  y = (int) floor((-(cimag(p)-C->center[1] - ((C->Sy[0]+C->Sy[1])/2)) - C->Sy[0])/(C->Sy[1]-C->Sy[0]) * h);

  for (int i = 0; i < C->N_line; i++){
    old_p = p;
    oldx = x; oldy = y;
    complex auxz = 0, aux = 0;

    for (int j = order; j >= 0; j--){
      if (j == order){
        auxz = c;
      } else if (j == order-1){
        auxz += complex_mul(polynomial[j], p);
      } else {
        aux = p;
        for (int k = 0; k < order-j-1; k++){
          aux = complex_mul(aux, p);
        }
        auxz += complex_mul(aux, polynomial[j]);
      }
    }
    p = auxz;

    x = (int) floor((creal(p) - C->Sx[0])/(C->Sx[1]-C->Sx[0]) * w);
    y = (int) floor((-(cimag(p)-C->center[1]-((C->Sy[0]+C->Sy[1])/2)) - C->Sy[0])/(C->Sy[1]-C->Sy[0]) * h);

    draw_line(C->drawn_plot, x, y, oldx, oldy, w, h);
    if (x >= C->w*2 || y >= C->h*2 ||
        x < -C->w   || y < -C->h   ){
      break;
    }
  }

}
