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
    x = (int) floor((p[0] - C->Sx[0])/(C->Sx[1]-C->Sx[0]) * w);
    y = (int) floor((-(p[1]-((C->Sy[0]+C->Sy[1])/2)) - C->Sy[0])/(C->Sy[1]-C->Sy[0]) * h);
  } else if (strcmp(C->plot_type, "rec_f") == 0){
    p[0] = point[0];
    p[1] = point[1];
    c[0] = C->z[0];
    c[1] = C->z[1];
    x = (int) floor((p[0] - C->Sx[0])/(C->Sx[1]-C->Sx[0]) * w);
    y = (int) floor((-(p[1]-((C->Sy[0]+C->Sy[1])/2)) - C->Sy[0])/(C->Sy[1]-C->Sy[0]) * h);
    C->drawn_plot[(y*C->w + x)*3 + 0] = 255;
    C->drawn_plot[(y*C->w + x)*3 + 1] = 255;
    C->drawn_plot[(y*C->w + x)*3 + 2] = 255;
  }

  for (int i = 0; i < C->N_line; i++){
    old_p[0] = p[0]; old_p[1] = p[1];
    oldx = x;        oldy = y;

    double aux = (pow(p[0], 2) - pow(p[1], 2)) + c[0];
    p[1] = 2*p[0]*p[1] + c[1];
    p[0] = aux;

    x = (int) floor((p[0] - C->Sx[0])/(C->Sx[1]-C->Sx[0]) * w);
    y = (int) floor((-(p[1]-((C->Sy[0]+C->Sy[1])/2)) - C->Sy[0])/(C->Sy[1]-C->Sy[0]) * h);

    draw_line(C->drawn_plot, x, y, oldx, oldy, w, h);
    if (x >= C->w || y >= C->h){
      break;
    }
  }
}
