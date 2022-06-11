#include <complex.h>
#include <stdlib.h>
#include <stdio.h>

#include <complex_function.h>

typedef struct ComplexPolynomial{
  int order;
  complex double *polynomial;
} ComplexPolynomial;

typedef struct RootArrayMember{
  ComplexPolynomial *root;
  RootArrayMember *next;
} RootArrayMember;


//If root is null, creates new with specified order. If root is not null, ignores order.
RootArrayMember *root_array_member_new(RootArrayMember **dest, ComplexPolynomial *root, int order){
  RootArrayMember *new = malloc(sizeof(RootArrayMember));
  if (root == NULL){
    new->root = complex_polynomial_new(NULL, order);
  } else {
    new->root = root;
  }

  if (dest != NULL){
    *dest = new;
  }

  return new;
}

const ComplexPolynomial *root_array_member_get_root(RootArrayMember *ram){
  return ram->root;
}
void root_array_member_set_root(RootArrayMember *ram, ComplexPolynomial *cp){
  ram->root = cp;
}
const ComplexPolynomial *root_array_member_new_root(RootArrayMember *ram, int order){
  ComplexPolynomial *cp = complex_polynomial_new(NULL, order);
  ram->root = cp;
  return cp;
}
RootArrayMember *root_array_member_next(RootArrayMember *ram){
  return ram->next;
}
void root_array_member_set_next(RootArrayMember *ram, RootArrayMember *dest){
  ram->next = dest;
}

ComplexPolynomial *complex_polynomial_new(ComplexPolynomial **dest, int order){
  ComplexPolynomial *new = malloc(sizeof(ComplexPolynomial));

  new->polynomial = NULL;

  complex_polynomial_set_order(new, order);

  if (dest != NULL){
    *dest = new;
  }
  return new;
}

//Memory management
_Bool complex_polynomial_free_members(ComplexPolynomial *cp){
  if (cp->polynomial != NULL){
    free(cp->polynomial);
    cp->polynomial = NULL;
    return true;
  }
  return false;
}
_Bool root_array_member_destroy(RootArrayMember *ram){
  if (ram->root == NULL){
    free(ram);
    return false;
  }
  if (! complex_polynomial_free_members(ram->root)){
    free(ram);
    return false;
  } else {
    free(ram);
    return true;
  }
}

//Configure polynomials
void complex_polynomial_set_order(ComplexPolynomial *cp, int o){
  complex_polynomial_free_members(cp);

  cp->order = o;
  if (o > 0){
    cp->polynomial = calloc(sizeof(complex double) * (o+1), 1);

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

const complex double *complex_polynomial_get_polynomial(ComplexPolynomial *cp){
  return cp->polynomial;
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
