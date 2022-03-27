__kernel void rec_f(__global unsigned char *m,
                    __global int *Np,
                    __global int *hp,
                    __global int *wp,
                    __global double *c,
                    __global double *Sx,
                    __global double *Sy) {
  double c_abs = native_sqrt(pow(c[0], 2) +  pow(c[1],2));
  double R = (c_abs > 2)? c_abs: 2;

  int h = *hp, w = *wp, N = *Np;


  const int y = get_global_id(0);
  const int x = get_global_id(1);

  //Map x and y to range between -R and R
  double newx = (double) x / (double) w;
  newx = newx * (Sx[1] - Sx[0]) + Sx[0];
  double newy = -(y - h);
  newy = (double) newy / (double) h;
  newy = newy * (Sy[1] - Sy[0]) + Sy[0];
  double z[2] = {newx, newy};
  double z_abs;
  double aux;

  for (int i = 0; i < N; i++){
    aux = (pow(z[0], 2) - pow(z[1], 2)) + c[0];
    z[1] = 2*z[0]*z[1] + c[1];
    z[0] = aux;
    z_abs = native_sqrt(pow(z[0], 2) + pow(z[1] , 2));

    if (z_abs > R){
      // // Glitch colors
      // double ni = log10((double) i);
      // double n = log10((double) N);
      // // m[(y*w + x)*3] = 255;
      // // m[(y*w + x)*3+1] = (int) floor( ((double) ni / (double) n) * 255);
      // m[(y*w + x)*3+0] = abs((int) floor(cos((double) i/200.0) * 255));
      // m[(y*w + x)*3+1] = abs((int) floor(sin((double) i/50.0) * 255));
      // m[(y*w + x)*3+2] = abs((int) floor(sin((double) i/100.0) * 255));

      // double ni = log10((double) i);
      // double n = log10((double) N);
      // m[(y*w + x)*3] = 255;
      // m[(y*w + x)*3+1] = (int) floor( ((double) ni / (double) n) * 255);
      m[(y*w + x)*3+0] = abs((int) floor(cos((double) i/200.0) * 255));
      m[(y*w + x)*3+1] = abs((int) floor(sin((double) i/50.0) * 255));
      m[(y*w + x)*3+2] = abs((int) floor(sin((double) i/200.0) * 255));

      ////Blue
      //m[(y*w + x)*3+2] = 255;
      //m[(y*w + x)*3+0] = (int) floor( ((double) ni / (double) n) * 255);
      //m[(y*w + x)*3+1] = i % 256;
      break;
    }
  }
  if (z_abs <= R){
      m[(y*w + x)*3+0] = 0;
      m[(y*w + x)*3+1] = 0;
      m[(y*w + x)*3+2] = 0;
  }
}

__kernel void parameter_space(__global unsigned char *m,
                    __global int *Np,
                    __global int *hp,
                    __global int *wp,
                    __global double *z0,
                    __global double *Sx,
                    __global double *Sy) {
  double z0_abs = native_sqrt(pow(z0[0], 2) +  pow(z0[1],2));
  double R = (z0_abs > 2)? z0_abs: 2;

  int h = *hp, w = *wp, N = *Np;


  const int y = get_global_id(0);
  const int x = get_global_id(1);

  //Map x and y to range between -R and R
  double newx = (double) x / (double) w;
  newx = newx * (Sx[1] - Sx[0]) + Sx[0];
  double newy = -(y - h);
  newy = (double) newy / (double) h;
  newy = newy * (Sy[1] - Sy[0]) + Sy[0];
  double c[2] = {newx, newy};
  double c_abs;
  double z[2];
  z[0] = z0[0];
  z[1] = z0[1];
  double z_abs;
  double aux;

  for (int i = 0; i < N; i++){
    aux = (pow(z[0], 2) - pow(z[1], 2)) + c[0];
    z[1] = 2*z[0]*z[1] + c[1];
    z[0] = aux;
    z_abs = native_sqrt(pow(z[0], 2) + pow(z[1] , 2));

    if (z_abs > R){
      // double ni = log10((double) i);
      // double n = log10((double) N);
      // m[(y*w + x)*3] = 255;
      // m[(y*w + x)*3+1] = (int) floor( ((double) ni / (double) n) * 255);
      m[(y*w + x)*3+0] = abs((int) floor(cos((double) i/200.0) * 255));
      m[(y*w + x)*3+1] = abs((int) floor(sin((double) i/50.0) * 255));
      m[(y*w + x)*3+2] = abs((int) floor(sin((double) i/200.0) * 255));
      break;
    }
  }
  if (z_abs <= R){
      m[(y*w + x)*3+0] = 0;
      m[(y*w + x)*3+1] = 0;
      m[(y*w + x)*3+2] = 0;
  }
}

void complex_mul(double a0, double b0, double a1, double b1, double *ar, double *br){
  *ar = (a0 * a1) - (b0 * b1);
  *br = (a0 * b1) + (b0 * a1);
}

__kernel void polynomial(__global unsigned char *m,
                         __global int *Np,
                         __global int *hp,
                         __global int *wp,
                         __global int *orderp,
                         __global double *polynomial_real,
                         __global double *polynomial_imag,
                         __global int *parameter_p,
                         __global double *Sx,
                         __global double *Sy) {
//  // double c_abs = native_sqrt(pow(c[0], 2) +  pow(c[1],2));
//  // double R = (c_abs > 2)? c_abs: 2;

  int parameter = *parameter_p;
  int h = *hp, w = *wp, N = *Np;
  int order = *orderp;


  const int y = get_global_id(0);
  const int x = get_global_id(1);

  //Map x and y to range between -R and R
  double newx = (double) x / (double) w;
  newx = newx * (Sx[1] - Sx[0]) + Sx[0];
  double newy = -(y - h);
  newy = (double) newy / (double) h;
  newy = newy * (Sy[1] - Sy[0]) + Sy[0];
  double param[2] = {newx, newy};
  double z[2] = {polynomial_real[order+1], polynomial_imag[order+1]};
  double z_abs;
  double aux;


  for (int i = 0; i < N; i++){
    double auxr = 0, auxi = 0, auxz0 = 0, auxz1 = 0, auxr2 = 0, auxi2 = 0;

    for (int j = order; j >= 0; j--){
      if (j == order){  // Add C
        if (parameter == order){  //c is my parameter (param[0])
          auxz0 = param[0];
          auxz1 = param[1];
        } else {    //param[0] is somewhere else
          auxz0 = polynomial_real[j];
          auxz1 = polynomial_imag[j];
        }
      } else if (j == order-1){ //param^1 -> add a*Z
        if (parameter == order + 1){  //param is my z
          if ((polynomial_real[j] != 0) || (polynomial_imag[j] != 0)){
            complex_mul(param[0], param[1], polynomial_real[j], polynomial_imag[j], &auxr2, &auxi2);
            auxz0 += auxr2;
            auxz1 += auxi2;
          }
        } else if (parameter == j){ //param is a
          complex_mul(param[0], param[1], z[0], z[1], &auxr2, &auxi2);
          auxz0 += auxr2;
          auxz1 += auxi2;
        } else {  //param is somewhere else
          if ((polynomial_real[j] != 0) || (polynomial_imag[j] != 0)){
            complex_mul(polynomial_real[j], polynomial_imag[j], z[0], z[1], &auxr2, &auxi2);
            auxz0 += auxr2;
            auxz1 += auxi2;
          }
        }
      } else {  //Exponentiate
        if (parameter == order + 1){  //param is z
          if ((polynomial_real[j] != 0) || (polynomial_imag[j] != 0)){
            auxr = param[0];
            auxi = param[1];
            for (int k = 0; k < order-j-1; k++){  //If order = 3; j = 0; order-j = 3; 0 < k < 2 -> multiply twice
              complex_mul(auxr, auxi, param[0], param[1], &auxr2, &auxi2);
              auxr = auxr2; auxi = auxi2;
            }
            complex_mul(auxr, auxi, polynomial_real[j], polynomial_imag[j], &auxr2, &auxi2);
            auxz0 += auxr2; auxz1 += auxi2;
          }
        } else if (parameter == j){ //param is a
          auxr = z[0];
          auxi = z[1];
          for (int k = 0; k < order-j-1; k++){  //If order = 3; j = 0; order-j = 3; 0 < k < 2 -> multiply twice
            complex_mul(auxr, auxi, z[0], z[1], &auxr2, &auxi2);
            auxr = auxr2; auxi = auxi2;
          }
          complex_mul(auxr, auxi, param[0], param[1], &auxr2, &auxi2);
          auxz0 += auxr2; auxz1 += auxi2;
        } else {  //param is somewhere else
          if ((polynomial_real[j] != 0) || (polynomial_imag[j] != 0)){
            auxr = z[0];
            auxi = z[1];
            for (int k = 0; k < order-j-1; k++){  //If order = 3; j = 0; order-j = 3; 0 < k < 2 -> multiply twice
              complex_mul(auxr, auxi, z[0], z[1], &auxr2, &auxi2);
              auxr = auxr2; auxi = auxi2;
            }
            complex_mul(auxr, auxi, polynomial_real[j], polynomial_imag[j], &auxr2, &auxi2);
            auxz0 += auxr2; auxz1 += auxi2;
          }
        }
      }
    }

    if (parameter == order + 1){  //z is param
      param[0] = auxz0;
      param[1] = auxz1;
      z_abs = native_sqrt(pow(param[0], 2) + pow(param[1], 2));
    } else {  //z is z
      z[0] = auxz0;
      z[1] = auxz1;
      z_abs = native_sqrt(pow(z[0], 2) + pow(z[1], 2));
    }
    if (z_abs > 10){
      // double ni = log10((double) i);
      // double n = log10((double) N);
      // m[(y*w + x)*3] = 255;
      // m[(y*w + x)*3+1] = (int) floor( ((double) ni / (double) n) * 255);
      m[(y*w + x)*3+0] = abs((int) floor(cos((double) i/200.0) * 255));
      m[(y*w + x)*3+1] = abs((int) floor(sin((double) i/50.0) * 255));
      m[(y*w + x)*3+2] = abs((int) floor(sin((double) i/200.0) * 255));
      break;
    }
  }
  if (z_abs <= 10){
      m[(y*w + x)*3+0] = 0;
      m[(y*w + x)*3+1] = 0;
      m[(y*w + x)*3+2] = 0;
  }
}

__kernel void clone(__global unsigned char *result,
                    __global unsigned char *source,
                    __global int *hp,
                    __global int *wp){
  int h = *hp;
  int w = *wp;
  const int y = get_global_id(0);
  const int x = get_global_id(1);

  result[(y*w + x)*3 + 0] = source[(y*w + x)*3 + 0];
  result[(y*w + x)*3 + 1] = source[(y*w + x)*3 + 1];
  result[(y*w + x)*3 + 2] = source[(y*w + x)*3 + 2];
}
