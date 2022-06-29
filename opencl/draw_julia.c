#define COMPLEX_PLANE_PARAMETER_SPACE 0
#define COMPLEX_PLANE_DYNAMIC_PLANE 1

#define COLORSCHEME_ITERATIONS_DEFAULT 0
#define COLORSCHEME_ITERATIONS_BLUE 1
#define COLORSCHEME_ITERATIONS_GREEN 2
#define COLORSCHEME_ITERATIONS_BLOODRED 3
#define COLORSCHEME_ITERATIONS_LEAFAGE 4
#define COLORSCHEME_ITERATIONS_SEABLUE 5
#define COLORSCHEME_ITERATIONS_GLITCH 6
#define COLORSCHEME_ITERATIONS_SUPER 7
#define COLORSCHEME_ITERATIONS_MEGA 8
#define COLORSCHEME_ITERATIONS_GIGA 9
#define COLORSCHEME_ITERATIONS_INFINITY 10
#define COLORSCHEME_ITERATIONS_PRECISSION 11
#define COLORSCHEME_ITERATIONS_BLACK 12
#define COLORSCHEME_ITERATIONS_WHITE 13
#define COLORSCHEME_ITERATIONS_TRANSCENDENTAL 14


#ifdef cl_khr_fp64
  #pragma OPENCL EXTENSION cl_khr_fp64 : enable
#elif defined(cl_amd_fp64)
  #pragma OPENCL EXTENSION cl_amd_fp64 : enable
#else
  #error "Double precision floating point not supported by OpenCL implementation."
#endif



//  KERNEL DECLARATIONS #############

//Quadratic formula, dynamic and parameter space
__kernel void rec_f(__global unsigned char *m,
                    __global int *Np,
                    __global int *hp,
                    __global int *wp,
                    __global double *c,
                    __global double *Sx,
                    __global double *Sy,
                    __global int *color);

__kernel void parameter_space(__global unsigned char *m,
                              __global int *Np,
                              __global int *hp,
                              __global int *wp,
                              __global double *z0,
                              __global double *Sx,
                              __global double *Sy,
                              __global int *color);

//Polynomial iteration
__kernel void polynomial(__global unsigned char *m,
                         __global int *Np,
                         __global int *hp,
                         __global int *wp,
                         __global int *orderp,
                         __global double *polynomial_real,
                         __global double *polynomial_imag,
                         __global int *parameter_p,
                         __global double *Sx,
                         __global double *Sy,
                         __global int *color);

//Prebuilt newton method
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
                                  __global double *Sy,
                                  __global int *color);

//Numerical method
__kernel void numerical_method(__global unsigned char *m,
                               __global int *Np,
                               __global int *hp,
                               __global int *wp,
                               __global int *orderp,
                               __global int *func_type_P,
                               __global double *param_a,
                               __global double *polynomial_real,
                               __global double *polynomial_imag,
                               __global double *polynomial_derivative_real,
                               __global double *polynomial_derivative_imag,
                               __global double *polynomial_second_derivative_real,
                               __global double *polynomial_second_derivative_imag,
                               __global double *polynomial_parameters_real,
                               __global double *polynomial_parameters_imag,
                               __global double *polynomial_parameters_derivative_real,
                               __global double *polynomial_parameters_derivative_imag,
                               __global double *polynomial_parameters_second_derivative_real,
                               __global double *polynomial_parameters_second_derivative_imag,
                               __global double *critical_real,   __global double *critical_imag,
                               __global double *Sx,              __global double *Sy,
                               __global int *color,
                               __global int *nrootsp,
                               __global double *roots_real,
                               __global double *roots_imag);

//Utils
__kernel void clone(__global unsigned char *result,
                    __global unsigned char *source,
                    __global int *hp,
                    __global int *wp);


// NUMERICAL METHODS ############
int newton_method(double *result_real, double *result_imag, double zr, double zi,
                  __global double *polynm_real, __global double *polynm_imag,
                  __global double *polynp_real, __global double *polynp_imag,
                  __global double *param_real,  __global double *param_imag,
                  __global double *paramp_real, __global double *paramp_imag,
                  double ar, double ai, int order);


// COMPLEX ALGEBRA ###############
void complex_mul(double a0, double b0, double a1, double b1, double *ar, double *br);
void complex_div(double a0, double b0, double a1, double b1, double *ar, double *br);
double complex_norm(double a, double b);
void cartesian_to_polar(double a, double b, double *r, double *t);

// POLYNOMIAL COMPUTATIONS #############
void compute_polynomial_p(double *result_real,     double *result_imag,
                          __global double *polynomial_real, __global double *polynomial_imag,
                          double param_real,       double param_imag,
                          double z_ini_real,       double z_ini_imag,
                          int order);

void compute_polynomial(__global double *polynomial_real, __global double *polynomial_imag,
                      int order, double *result_real, double *result_imag,
                      double *param, double *z, int parameter);


// COLORS ################
void color_with_iterations(int i, int N,
                           int colorscheme,
                           __global unsigned char *r,
                           __global unsigned char *g,
                           __global unsigned char *b);

void color_matrix_radial(__global unsigned char *m,
                         unsigned x, unsigned y, unsigned w,
                         double zreal, double zimag,
                         int colorscheme, float factor);





void color_with_iterations(int i, int N,
                           int colorscheme,
                           __global unsigned char *r,
                           __global unsigned char *g,
                           __global unsigned char *b){
  const double PI = 3.141592;
  const double E =  2.718281;
  const double PHI= 1.618033;
  double ni, n;

  float trig_ratio_1 = 200, trig_ratio_2 = 50, trig_ratio_3 = 200;

  if (i >= N){
    switch(colorscheme){
      case COLORSCHEME_ITERATIONS_WHITE:
        *r = 255;
        *g = 255;
        *b = 255;
        break;
      default:
        *r = 0;
        *g = 0;
        *b = 0;
        break;
    }
    return;
  }

  switch(colorscheme){
    case COLORSCHEME_ITERATIONS_DEFAULT:
    case COLORSCHEME_ITERATIONS_BLUE:
    case COLORSCHEME_ITERATIONS_GREEN:
      trig_ratio_1 = 200;
      trig_ratio_2 = 50;
      trig_ratio_3 = 200;
      break;
    case COLORSCHEME_ITERATIONS_GLITCH:
      trig_ratio_1 = 200;
      trig_ratio_2 = 50;
      trig_ratio_3 = 100;
      break;
    case COLORSCHEME_ITERATIONS_SUPER:
      trig_ratio_1 = 100;
      trig_ratio_2 = 25;
      trig_ratio_3 = 100;
      break;
    case COLORSCHEME_ITERATIONS_MEGA:
      trig_ratio_1 = 40;
      trig_ratio_2 = 10;
      trig_ratio_3 = 40;
      break;
    case COLORSCHEME_ITERATIONS_GIGA:
      trig_ratio_1 = 20;
      trig_ratio_2 = 5;
      trig_ratio_3 = 20;
      break;
    case COLORSCHEME_ITERATIONS_INFINITY:
      trig_ratio_1 = 4;
      trig_ratio_2 = 1;
      trig_ratio_3 = 4;
      break;
    case COLORSCHEME_ITERATIONS_PRECISSION:
      trig_ratio_1 = 500;
      trig_ratio_2 = 125;
      trig_ratio_3 = 500;
      break;
    case COLORSCHEME_ITERATIONS_TRANSCENDENTAL:
      trig_ratio_1 = PHI * 100;   //R
      trig_ratio_2 = E   * 20;    //G
      trig_ratio_3 = PI  * 250 ;  //B
      break;
  }

  switch (colorscheme){
    case COLORSCHEME_ITERATIONS_DEFAULT:
    case COLORSCHEME_ITERATIONS_SUPER:
    case COLORSCHEME_ITERATIONS_MEGA:
    case COLORSCHEME_ITERATIONS_GIGA:
    case COLORSCHEME_ITERATIONS_INFINITY:
    case COLORSCHEME_ITERATIONS_PRECISSION:
      *r = abs((int) floor(cos((double) i/trig_ratio_1) * 255));
      *g = abs((int) floor(sin((double) i/trig_ratio_2) * 255));
      *b = abs((int) floor(sin((double) i/trig_ratio_3) * 255));
      break;
    case COLORSCHEME_ITERATIONS_BLUE:
      *r = abs((int) floor(sin((double) i/trig_ratio_1) * 255));
      *g = abs((int) floor(sin((double) i/trig_ratio_2) * 255));
      *b = abs((int) floor(cos((double) i/trig_ratio_3) * 255));
      break;
    case COLORSCHEME_ITERATIONS_GREEN:
      *r = abs((int) floor(sin((double) i/trig_ratio_1) * 255));
      *g = abs((int) floor(cos((double) i/trig_ratio_2) * 255));
      *b = abs((int) floor(sin((double) i/trig_ratio_3) * 255));
      break;
    case COLORSCHEME_ITERATIONS_GLITCH:
      *r = abs((int) floor(cos((double) i/200.0) * 255));
      *g = abs((int) floor(sin((double) i/50.0) * 255));
      *b = abs((int) floor(sin((double) i/100.0) * 255));
      break;
    case COLORSCHEME_ITERATIONS_BLOODRED:
      *r = (int) (((1.0-exp(-(((double) N/25)*((double)i/N))))/(1.0-exp(-2*PI*((double) N/25))))*255);
      *g = 0;
      *b = 0;
      break;
    case COLORSCHEME_ITERATIONS_LEAFAGE:
      *r = (int) (((1.0-exp(-(((double) N/1000)*((double)i/N))))/(1.0-exp(-2*PI*((double) N/1000))))*255);
      *g = (int) (((1.0-exp(-(((double) N/25)*((double)i/N))))/(1.0-exp(-2*PI*((double) N/25))))*255);
      *b = (int) (((1.0-exp(-(((double) N/1000)*((double)i/N))))/(1.0-exp(-2*PI*((double) N/1000))))*255);
      break;
    case COLORSCHEME_ITERATIONS_SEABLUE:
      *r = (int) (((1.0-exp(-(((double) N/500)*((double)i/N))))/(1.0-exp(-2*PI*((double) N/500))))*128);
      *g = (int) (((1.0-exp(-(((double) N/500)*((double)i/N))))/(1.0-exp(-2*PI*((double) N/500))))*128);
      *b = (int) (((1.0-exp(-(((double) N/15)*((double)i/N))))/(1.0-exp(-2*PI*((double) N/15))))*255);
      break;
    case COLORSCHEME_ITERATIONS_BLACK:
      *r = 255;
      *g = 0;
      *b = 0;
      break;
    case COLORSCHEME_ITERATIONS_WHITE:
      *r = 0;
      *g = 0;
      *b = 0;
      break;
    case COLORSCHEME_ITERATIONS_TRANSCENDENTAL:
      *r = abs((int) floor(sin((double) i/trig_ratio_1) * 255));
      *g = abs((int) floor(cos((double) i/trig_ratio_2) * 255));
      *b = abs((int) floor(cos((double) i/trig_ratio_3) * 255));
      break;
    default:
      *r = abs((int) floor(cos((double) i/200.0) * 255));
      *g = abs((int) floor(sin((double) i/50.0) * 255));
      *b = abs((int) floor(sin((double) i/200.0) * 255));
      break;
  }
}

void color_matrix_radial(__global unsigned char *m, unsigned x, unsigned y, unsigned w, double zreal, double zimag, int colorscheme, float factor){
  const double PI = 3.141592;
  const double tol = 0.0000001;
  if (fabs(zreal) < tol && fabs(zimag) < tol){
    m[(y*w + x)*3+0] = 128;
    m[(y*w + x)*3+1] = 128;
    m[(y*w + x)*3+2] = 128;
    return;
  }

  double r, theta;

  double zr[2] = {zreal, zimag};

  cartesian_to_polar(zr[0], zr[1], &r, &theta);
  theta = fmod(theta, 2*PI);

  //Theta en [0, 2PI]
  double hp;
  double C;
  double X;

  int Xi;
  int Ci;

  // hp = theta / (PI/3);
  // C = 1;

  // X = C * (1 - fabs(fmod(hp, 2) - 1));
  // X = floor(X*255);
  // C = floor(C*255);

  // Xi = (int) (X * factor);
  // Ci = (int) (C * factor);

  switch(colorscheme){
    case COLORSCHEME_ITERATIONS_BLUE:
      m[(y*w + x)*3+0] = (int) floor(fabs(sin(theta/4))*255);
      m[(y*w + x)*3+1] = 64;
      // m[(y*w + x)*3+2] = (unsigned char) 255*factor;
      m[(y*w + x)*3+2] = (int) floor(fabs(cos(theta/4))*255);
      break;
    case COLORSCHEME_ITERATIONS_DEFAULT:
    default:
      hp = theta / (PI/3);
      C = 1;

      X = C * (1 - fabs(fmod(hp, 2) - 1));
      X = floor(X*255);
      C = floor(C*255);

      Xi = (int) (X * factor);
      Ci = (int) (C * factor);

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
      } else if (5 <= hp && hp <= 6){
        m[(y*w + x)*3+0] = Ci;
        m[(y*w + x)*3+1] = 0;
        m[(y*w + x)*3+2] = Xi;
      }
      break;
  }
}


__kernel void rec_f(__global unsigned char *m,
                    __global int *Np,
                    __global int *hp,
                    __global int *wp,
                    __global double *c,
                    __global double *Sx,
                    __global double *Sy,
                    __global int *color) {
  double c_abs = native_sqrt(pow(c[0], 2) +  pow(c[1],2));
  double R = (c_abs > 2)? c_abs: 2;

  const int death_tol = 50;
  double tol = 0.000001;

  int colorscheme = *color;

  int h = *hp, w = *wp, N = *Np;


  const int y = get_global_id(0);
  const int x = get_global_id(1);

  int death_counter = 0;

  //Map x and y to range between -R and R
  double newx = (double) x / (double) w;
  newx = newx * (Sx[1] - Sx[0]) + Sx[0];
  double newy = -(y - h);
  newy = (double) newy / (double) h;
  newy = newy * (Sy[1] - Sy[0]) + Sy[0];
  double z[2] = {newx, newy};
  double z_abs = native_sqrt(pow(z[0], 2) + pow(z[1] , 2));
  double old_z_abs;
  double z_abs_diff = 0;
  double old_z_abs_diff;
  double aux;

  for (int i = 0; i < N; i++){
    aux = (pow(z[0], 2) - pow(z[1], 2)) + c[0];
    z[1] = 2*z[0]*z[1] + c[1];
    z[0] = aux;
    old_z_abs = z_abs;
    z_abs = native_sqrt(pow(z[0], 2) + pow(z[1] , 2));

    //Diverges
    if (z_abs > R){
      color_with_iterations(i, N, colorscheme, &m[(y*w + x)*3+0], &m[(y*w + x)*3+1], &m[(y*w + x)*3+2]);
      return;
    }

    //Converges
    old_z_abs_diff = z_abs_diff;
    z_abs_diff = fabs(z_abs - old_z_abs);
    if (z_abs_diff < tol && old_z_abs_diff >= z_abs_diff){
      death_counter ++;
    } else {
      death_counter = 0;
    }
    if (death_counter >= death_tol){
      color_with_iterations(N, N, colorscheme, &m[(y*w + x)*3+0], &m[(y*w + x)*3+1], &m[(y*w + x)*3+2]);
      return;
    }
  }
  if (z_abs <= R){
      color_with_iterations(N, N, colorscheme, &m[(y*w + x)*3+0], &m[(y*w + x)*3+1], &m[(y*w + x)*3+2]);
      return;
  }
}

__kernel void parameter_space(__global unsigned char *m,
                    __global int *Np,
                    __global int *hp,
                    __global int *wp,
                    __global double *z0,
                    __global double *Sx,
                    __global double *Sy,
                    __global int *color) {
  double z0_abs = native_sqrt(pow(z0[0], 2) +  pow(z0[1],2));
  double R = (z0_abs > 2)? z0_abs: 2;

  const int death_tol = 50;
  double tol = 0.0000000001;

  int h = *hp, w = *wp, N = *Np;

  int colorscheme = *color;


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
  double z_abs = native_sqrt(pow(z[0], 2) + pow(z[1] , 2));
  double old_z_abs;
  double z_abs_diff = 0;
  double old_z_abs_diff;
  double aux;
  int death_counter = 0;

  for (int i = 0; i < N; i++){
    aux = (pow(z[0], 2) - pow(z[1], 2)) + c[0];
    z[1] = 2*z[0]*z[1] + c[1];
    z[0] = aux;
    old_z_abs = z_abs;
    z_abs = native_sqrt(pow(z[0], 2) + pow(z[1] , 2));

    //Diverges
    if (z_abs > R){
      color_with_iterations(i, N, colorscheme, &m[(y*w + x)*3+0], &m[(y*w + x)*3+1], &m[(y*w + x)*3+2]);
      break;
    }

    //Converges
    old_z_abs_diff = z_abs_diff;
    z_abs_diff = fabs(z_abs - old_z_abs);
    if (z_abs_diff < tol && old_z_abs_diff >= z_abs_diff){
      death_counter ++;
    } else {
      death_counter = 0;
    }
    if (death_counter >= death_tol){
      color_with_iterations(N, N, colorscheme, &m[(y*w + x)*3+0], &m[(y*w + x)*3+1], &m[(y*w + x)*3+2]);
      return;
    }
  }
  if (z_abs <= R){
      color_with_iterations(N, N, colorscheme, &m[(y*w + x)*3+0], &m[(y*w + x)*3+1], &m[(y*w + x)*3+2]);
      return;
  }
}

void complex_mul(double a0, double b0, double a1, double b1, double *ar, double *br){
  double auxa = (a0 * a1) - (b0 * b1);
  double auxb = (a0 * b1) + (b0 * a1);
  *ar = auxa;
  *br = auxb;
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

void cartesian_to_polar(double a, double b, double *r, double *t){
  const double PI = 3.141592;
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

void compute_polynomial_p(double *result_real,     double *result_imag,
                          __global double *polynomial_real, __global double *polynomial_imag,
                          double param_real,       double param_imag,
                          double z_ini_real,       double z_ini_imag,
                          int order){

  double auxz0 = 0, auxz1 = 0, auxr2 = 0, auxi2 = 0, auxr = 0, auxi = 0;
  double z[2] = {z_ini_real, z_ini_imag};

  for (int j = order; j >= 0; j--){
    if (j == order){    //Add C
      // auxz0 = polynomial_real[j];
      // auxz1 = polynomial_imag[j];
      complex_mul(polynomial_real[j], polynomial_imag[j], param_real, param_imag, &auxz0, &auxz1);
    } else if (j == order -1){  //add a*Z
      if ((polynomial_real[j] != 0 || polynomial_imag[j] != 0) && (param_real != 0 || param_imag != 0)){
        complex_mul(polynomial_real[j], polynomial_imag[j], z[0], z[1], &auxr2, &auxi2);
        complex_mul(auxr2, auxi2, param_real, param_imag, &auxr2, &auxi2);
        auxz0 += auxr2; auxz1 += auxi2;
      }
    } else {  //Exponientiate
      if ((polynomial_real[j] != 0 || polynomial_imag[j] != 0) && (param_real != 0 || param_imag != 0)){
        auxr = z[0]; auxi = z[1];
        for (int k = 0; k < order-j-1; k++){
          complex_mul(auxr, auxi, z[0], z[1], &auxr, &auxi);
        }
        complex_mul(auxr, auxi, polynomial_real[j], polynomial_imag[j], &auxr, &auxi);
        complex_mul(auxr, auxi, param_real, param_imag, &auxr, &auxi);
        auxz0 += auxr, auxz1 += auxi;
      }
    }
  }
  *result_real = auxz0; *result_imag = auxz1;
}

void compute_polynomial(__global double *polynomial_real, __global double *polynomial_imag,
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
                         __global double *Sy,
                         __global int *color) {

  int parameter = *parameter_p;
  int h = *hp, w = *wp, N = *Np;
  int order = *orderp;

  int colorscheme = *color;

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
      color_with_iterations(i, N, colorscheme, &m[(y*w + x)*3+0], &m[(y*w + x)*3+1], &m[(y*w + x)*3+2]);
      break;
    }
  }
  if (z_abs <= 10){
      color_with_iterations(N, N, colorscheme, &m[(y*w + x)*3+0], &m[(y*w + x)*3+1], &m[(y*w + x)*3+2]);
  }
}

int newton_method(double *result_real, double *result_imag, double zr, double zi,
                  __global double *polynm_real, __global double *polynm_imag, __global double *polynp_real, __global double *polynp_imag,
                  __global double *param_real,  __global double *param_imag,  __global double *paramp_real, __global double *paramp_imag,
                  double ar, double ai, int order){

  double auxr, auxi;
  double denr, deni;
  double numr, numi;

  compute_polynomial_p(&denr, &deni, polynp_real, polynp_imag, 1, 0, zr, zi, order);
  compute_polynomial_p(&auxr, &auxi, paramp_real, paramp_imag, ar, ai, zr, zi, order);
  denr += auxr; deni += auxi;

  if (denr == 0 && deni == 0){
    return -1;
  }

  compute_polynomial_p(&numr, &numi, polynm_real, polynm_imag, 1, 0, zr, zi, order);
  compute_polynomial_p(&auxr, &auxi, param_real, param_imag, ar, ai, zr, zi, order);
  numr += auxr; numi += auxi;

  complex_div(numr, numi, denr, deni, &auxr, &auxi);

  *result_real = zr - auxr;
  *result_imag = zi - auxi;

  return 0;
}

__kernel void numerical_method(__global unsigned char *m,
                               __global int *Np,
                               __global int *hp,
                               __global int *wp,
                               __global int *orderp,
                               __global int *func_type_P,
                               __global double *param_a,
                               __global double *polynomial_real,
                               __global double *polynomial_imag,
                               __global double *polynomial_derivative_real,
                               __global double *polynomial_derivative_imag,
                               __global double *polynomial_second_derivative_real,
                               __global double *polynomial_second_derivative_imag,
                               __global double *polynomial_parameters_real,
                               __global double *polynomial_parameters_imag,
                               __global double *polynomial_parameters_derivative_real,
                               __global double *polynomial_parameters_derivative_imag,
                               __global double *polynomial_parameters_second_derivative_real,
                               __global double *polynomial_parameters_second_derivative_imag,
                               __global double *critical_real,   __global double *critical_imag,
                               __global double *Sx,              __global double *Sy,
                               __global int *color,
                               __global int *nrootsp,
                               __global double *roots_real,
                               __global double *roots_imag){

  const double PI = 3.141592;
  int func_type = *func_type_P;

  const int w = *wp; const int h = *hp;
  const int order = *orderp;
  const int N = *Np;

  const int y = get_global_id(0);
  const int x = get_global_id(1);

  int colorscheme = *color;
  int nroots = *nrootsp;

  //Map x and y to range between -R and R
  double newx = (double) x / (double) w;
  newx = newx * (Sx[1] - Sx[0]) + Sx[0];
  double newy = -(y - h);
  newy = (double) newy / (double) h;
  newy = newy * (Sy[1] - Sy[0]) + Sy[0];

  // double tol = 0.0000000001;
  double tol = 0.00000001;
  double far_tol = 100;
  int death_counter = 0;
  int death_tolerance = 5;

  double a[2];  //If parameterspace -> pixel. If dynamicplane -> cursor
  double z[2];
  double zr[2];
  double oldz[2];

  double norm;
  double old_norm;

  int ret;



  if (func_type == COMPLEX_PLANE_PARAMETER_SPACE){
    a[0] = newx; a[1] = newy;
    compute_polynomial_p(&z[0], &z[1], critical_real, critical_imag, 1, 0, a[0], a[1], order);
  } else if (func_type == COMPLEX_PLANE_DYNAMIC_PLANE){
    a[0] = param_a[0]; a[1] = param_a[1];
    z[0] = newx; z[1] = newy;
  }


  zr[0] = z[0]; zr[1] = z[1];
  norm = complex_norm(zr[0], zr[1]);
  for (int i = 0; i < N; i++){
    oldz[0] = zr[0]; oldz[1] = zr[1];
    old_norm = norm;

    ret = newton_method(&zr[0], &zr[1], zr[0], zr[1], polynomial_real, polynomial_imag,
                        polynomial_derivative_real, polynomial_derivative_imag,
                        polynomial_parameters_real, polynomial_parameters_imag,
                        polynomial_parameters_derivative_real, polynomial_parameters_derivative_imag,
                        a[0], a[1], order);

    norm = complex_norm(zr[0], zr[1]);
    if (norm > far_tol && norm > old_norm){
      death_counter++;
    } else {
      death_counter = 0;
    }


    if (ret == -1 || death_counter >= death_tolerance){                 //Divide by 0 or diverge
      m[(y*w + x)*3+0] = 255;
      m[(y*w + x)*3+1] = 255;
      m[(y*w + x)*3+2] = 255;
      // m[(y*w + x)*3+0] = abs((int) floor(cos((double) i/200.0) * 255));
      // m[(y*w + x)*3+1] = abs((int) floor(sin((double) i/50.0) * 255));
      // m[(y*w + x)*3+2] = abs((int) floor(sin((double) i/200.0) * 255));
      return;
    }


    // if (fabs(norm - old_norm) <= tol){ //Converges!
    if (fabs(oldz[0] - zr[0]) < tol && fabs(oldz[1] - zr[1]) < tol){
      int colored = 0;

      for (int root = 0; root < nroots; root++){
        double root1, root2;
        compute_polynomial_p(&root1, &root2, &roots_real[root*(order+1)], &roots_imag[root*(order+1)], 1, 0, a[0], a[1], order);

        if (fabs(root1 - zr[0]) < tol && fabs(root2 - zr[1]) < tol){
          float deg = ((float) root / (float) nroots) * 2*PI;

          double cord1 = cos(deg);
          double cord2 = sin(deg);

          color_matrix_radial(m, x, y, w, cord1, cord2, colorscheme, 0.66);

          colored = 1;
          return;
        }
      }

      if (colored == 0){
        color_matrix_radial(m, x, y, w, zr[0], zr[1], colorscheme, 1);
        return;
      }
    }

  }


  //Doesnt converge nor diverge!
  m[(y*w + x)*3+0] = 0;
  m[(y*w + x)*3+1] = 0;
  m[(y*w + x)*3+2] = 0;

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
                                  __global double *Sy,
                                  __global int *color) {
  const double PI = 3.141592;

  int parameter = *parameter_p;
  int w = *wp, h = *hp;
  int N = *Np;
  int order = *orderp;

  int colorscheme = *color;

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
    color_matrix_radial(m, x, y, w, z[0], z[1], colorscheme, 1);
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
