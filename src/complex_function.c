#include <complex.h>
#include <stdlib.h>
#include <stdio.h>

#include <complex_function.h>

typedef struct ComplexPolynomial{
  int order;

  complex double *polynomial;
  int *parameters;
  complex double z;
} ComplexPolynomial;


ComplexPolynomial *complex_polynomial_new(ComplexPolynomial **dest){
  ComplexPolynomial *new = malloc(sizeof(ComplexPolynomial));

  new->order = -1;
  new->parameters = NULL;
  new->polynomial = NULL;
  new->z = 0;

  if (dest != NULL){
    *dest = new;
  }
  return new;
}

//Memory management
void complex_polynomial_free_members(ComplexPolynomial *cp){
  if (cp->polynomial != NULL){
    free(cp->polynomial);
    cp->polynomial = NULL;
  }
  if (cp->parameters != NULL){
    free(cp->parameters);
    cp->parameters = NULL;
  }
}

//Configure polynomials
void complex_polynomial_set_order(ComplexPolynomial *cp, int o){
  complex_polynomial_free_members(cp);

  cp->order = o;
  if (o > 0){
    cp->polynomial = calloc(sizeof(complex double) * (o+1), 1);
    cp->parameters = calloc(sizeof(complex double) * (o+1), 1);

    for (int i = 0; i < o+1; i++){
      complex_polynomial_set_member(cp, 0, i);
    }
  }
}
int complex_polynomial_get_order(ComplexPolynomial *cp){
  return cp->order;
}

void complex_polynomial_set_member(ComplexPolynomial *cp, complex double x, int i){
  int order = cp->order;
  if (i >= order +1){
    return;
  }
  cp->polynomial[i] = x;
}
complex double complex_polynomial_get_member(ComplexPolynomial *cp, int i){
  return cp->polynomial[i];
}

void complex_polynomial_set_z(ComplexPolynomial *cp, complex double z){
  cp->z = z;
}
complex double complex_polynomial_get_z(ComplexPolynomial *cp){
  return cp->z;
}

void complex_polynomial_free_parameters(ComplexPolynomial *cp){
  if (cp->parameters != NULL){
    free(cp->parameters);
    cp->parameters = NULL;
  }
  // cp->n_parameters = 0;
}
void complex_polynomial_set_n_parameters(ComplexPolynomial *cp, int n){
  complex_polynomial_free_parameters(cp);
  // cp->n_parameters = n;
  cp->parameters = malloc(sizeof(int) * n);
}

//IO
void complex_polynomial_print(ComplexPolynomial *cp){
  for (int i = 0; i < cp->order; i++){
    if (i == cp->order){
      printf("(%g%+g)z ", creal(cp->polynomial[i]), cimag(cp->polynomial[i]));
    } else if (i == cp->order -1){
       printf("(%g%+g)z ", creal(cp->polynomial[i]), cimag(cp->polynomial[i]));
    } else {
       printf("(%g%+g)z^%d ", creal(cp->polynomial[i]), cimag(cp->polynomial[i]), cp->order - i);
    }
    printf("+ ");
  }

  printf("(%g%+g)", creal(cp->polynomial[cp->order]), cimag(cp->polynomial[cp->order]));
  printf("\n");
}
