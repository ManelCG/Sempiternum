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

