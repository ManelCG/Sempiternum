#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

#include <math.h>
#include <complex.h>
#include <stdbool.h>

#include <lodepng.h>
#include <file_io.h>
#include <file_io.h>
#include <draw_julia.h>
#include <opencl_funcs.h>

#include <complex_function.h>

int main(int argc, char *argv[]){
  ComplexPolynomial *f1 = complex_polynomial_new(NULL);
  ComplexPolynomial *f2 = complex_polynomial_new(NULL);
  ComplexPolynomial *f3 = complex_polynomial_new(NULL);

  complex_polynomial_set_order(f1, 1);
  complex_polynomial_set_order(f2, 1);
  complex_polynomial_set_order(f3, 1);

  complex_polynomial_set_member(f1, 1, 0);
  complex_polynomial_set_member(f2, 1, 0);
  complex_polynomial_set_member(f2, -1, 1);
  complex_polynomial_set_member(f3, 1, 0);

  // complex_polynomial_set_n_parameters(f3, 1);

  complex_polynomial_print(f1);
  complex_polynomial_print(f2);
  complex_polynomial_print(f3);

  free(f1);
  free(f2);
  free(f3);
}
