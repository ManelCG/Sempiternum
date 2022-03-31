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

void complex_div(double a0, double b0, double a1, double b1, double *ar, double *br){
  double den  = (a1*a1 + b1*b1);
  double num1 = (a0*a1 + b0*b1);
  double num2 = (b0*a1 - a0*b1);

  *ar = num1/den;
  *br = num2/den;
}

double complex_norm(double a, double b){
  return native_sqrt(pow(a, 2) + pow(b, 2));
}

const double PI = 3.141592;
void cartesian_to_polar(double a, double b, double *r, double *t){
  double theta;

  theta = atan(b/a);
  if (a < 0){
    theta += PI;
  } else if (b < 0){
    theta += 2*PI;
  }

  *r = complex_norm(a, b);
  *t = theta;
}

void compute_polynomial(double *polynomial_real, double *polynomial_imag,
                      int order, double *result_real, double *result_imag,
                      double *param, double *z, int parameter){

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

  *result_real = auxz0;
  *result_imag = auxz1;

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


  for (int i = 0; i < N; i++){
    double auxz0 = 0, auxz1 = 0;

    compute_polynomial(polynomial_real, polynomial_imag,
                       order, &auxz0, &auxz1, param, z, parameter);

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

__kernel void polynomial_fraction(__global unsigned char *m,
                                  __global int *Np,
                                  __global int *hp,
                                  __global int *wp,
                                  __global int *orderp,
                                  __global double *numerator_real,
                                  __global double *numerator_imag,
                                  __global double *denominator_real,
                                  __global double *denominator_imag,
                                  __global int *parameter_p,
                                  __global double *Sx,
                                  __global double *Sy) {
  const double PI = 3.141592;

  int parameter = *parameter_p;
  int w = *wp, h = *hp;
  int N = *Np;
  int order = *orderp;

  double tol = 0.00000001;

  const int y = get_global_id(0);
  const int x = get_global_id(1);

  //Map x and y to range between -R and R
  double newx = (double) x / (double) w;
  newx = newx * (Sx[1] - Sx[0]) + Sx[0];
  double newy = -(y - h);
  newy = (double) newy / (double) h;
  newy = newy * (Sy[1] - Sy[0]) + Sy[0];
  double param[2] = {newx, newy};
  double z[2] = {numerator_real[order+1], numerator_imag[order+1]};

  int converged = 0;

  double oldz[2];
  for (int i = 0; i < N; i++){
    double num0 = 0, num1 = 0;
    double den0 = 0, den1 = 0;
    double auxz0 = 0, auxz1 = 0;
    double diff[2] = {0, 0};
    double diff_abs = 0;

    if (parameter == order + 1){
      oldz[0] = param[0];
      oldz[1] = param[1];
    } else {
      oldz[0] = z[0];
      oldz[1] = z[1];
    }

    if (parameter == order){              //Parameter is c -> deriv doesnt have parameter
      compute_polynomial(denominator_real, denominator_imag,
                         order, &den0, &den1, param, z, -1);
    } else if (parameter == order + 1){   //Parameter is z -> it should still be z
      compute_polynomial(denominator_real, denominator_imag,
                         order, &den0, &den1, param, z, parameter);
    } else {                              //Parameter is one of the a*z^n
      compute_polynomial(denominator_real, denominator_imag,
                         order, &den0, &den1, param, z, parameter + 1);

    }

    if (den0 == 0 && den1 == 0){
      break;
    }

    compute_polynomial(numerator_real, numerator_imag,
                       order, &num0, &num1, param, z, parameter);

    complex_div(num0, num1, den0, den1, &auxz0, &auxz1);

    if (parameter == order + 1){  //z is param
      param[0] = param[0] - auxz0;
      param[1] = param[1] - auxz1;

      diff[0] = oldz[0] - param[0];
      diff[1] = oldz[1] - param[1];
    } else {  //z is z
      z[0] = z[0] - auxz0;
      z[1] = z[1] - auxz1;

      diff[0] = oldz[0] - z[0];
      diff[1] = oldz[1] - z[1];
    }

    diff_abs = complex_norm(diff[0], diff[1]);

    if (diff_abs < tol){
      converged = 1;
      break;
    }

  } //Main loop finished

  if (parameter == order + 1){  //z is param
    z[0] = param[0];
    z[1] = param[1];
  }

  if (converged == 1){ //Color points
    double r, theta;
    cartesian_to_polar(z[0], z[1], &r, &theta);
    theta = fmod(theta, 2*PI);

    //Theta en [0, 2PI]
    double hp = theta / (PI/3);
    double C = 1;

    double X = C * (1 - fabs(fmod(hp, 2) - 1));
    X = floor(X*255);
    C = floor(C*255);

    int Xi = (int) X;
    int Ci = (int) C;

    if (0 <= hp && hp < 1){
      m[(y*w + x)*3+0] = Ci;
      m[(y*w + x)*3+1] = Xi;
      m[(y*w + x)*3+2] = 0;
    } else if (1 <= hp && hp < 2){
      m[(y*w + x)*3+0] = Xi;
      m[(y*w + x)*3+1] = Ci;
      m[(y*w + x)*3+2] = 0;
    } else if (2 <= hp && hp < 3){
      m[(y*w + x)*3+0] = 0;
      m[(y*w + x)*3+1] = Ci;
      m[(y*w + x)*3+2] = Xi;
    } else if (3 <= hp && hp < 4){
      m[(y*w + x)*3+0] = 0;
      m[(y*w + x)*3+1] = Xi;
      m[(y*w + x)*3+2] = Ci;
    } else if (4 <= hp && hp < 5){
      m[(y*w + x)*3+0] = Xi;
      m[(y*w + x)*3+1] = 0;
      m[(y*w + x)*3+2] = Ci;
    } else if (5 <= hp && hp < 6){
      m[(y*w + x)*3+0] = Ci;
      m[(y*w + x)*3+1] = 0;
      m[(y*w + x)*3+2] = Xi;
    } else {
    }


  } else {
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
