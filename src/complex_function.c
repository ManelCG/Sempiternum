#include <complex.h>
#include <stdlib.h>
#include <stdio.h>

#include <string.h>

#include <math.h>

#include <complex_function.h>

typedef struct ComplexPolynomial{
  int order;
  complex double *polynomial;
} ComplexPolynomial;

typedef struct RootArrayMember{
  ComplexPolynomial *root;
  RootArrayMember *next;
} RootArrayMember;


ComplexPolynomial *complex_polynomial_copy(ComplexPolynomial **dest, ComplexPolynomial *src){
  if (src != NULL){
    int order = src->order;
    ComplexPolynomial *new = complex_polynomial_new(NULL, order);
    for (int i = 0; i <= order; i++){
      new->polynomial[i] = src->polynomial[i];
    }

    if (dest != NULL){
      *dest = new;
    }
    return new;
  } else {
    return NULL;
  }
}

RootArrayMember *root_array_member_copy(RootArrayMember **dest, RootArrayMember *src){
  if (src != NULL){
    ComplexPolynomial *cpnew = complex_polynomial_copy(NULL, src->root);
    RootArrayMember *new = root_array_member_new(NULL, cpnew, src->root->order);
    new->next = src->next;

    if (dest != NULL){
      *dest = new;
    }
    return new;
  } else {
    return NULL;
  }
}

RootArrayMember *root_array_copy(RootArrayMember **dest, RootArrayMember *src, int nroots){
  if (src != NULL){
    RootArrayMember *r = src;

    RootArrayMember *new = root_array_member_copy(NULL, r);
    RootArrayMember *ptr = new;

    for (int root = 0; root < nroots; root++){
      r = r->next;

      RootArrayMember *newr = root_array_member_copy(NULL, r);
      ptr->next = newr;

      ptr = ptr->next;
    }

    if (dest != NULL){
      *dest = new;
    }
    return new;
  } else {
    return NULL;
  }
}


//If root is null, creates new with specified order. If root is not null, ignores order.
RootArrayMember *root_array_member_new(RootArrayMember **dest, ComplexPolynomial *root, int order){
  RootArrayMember *new = malloc(sizeof(RootArrayMember));
  if (root == NULL){
    new->root = complex_polynomial_new(NULL, order);
  } else {
    new->root = root;
  }

  new->next = NULL;

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
  if (i > order){
    return;
  }
  cp->polynomial[i] = x;
}
complex double complex_polynomial_get_member(const ComplexPolynomial *cp, int i){
  return cp->polynomial[i];
}

const complex double *complex_polynomial_get_polynomial(const ComplexPolynomial *cp){
  return cp->polynomial;
}

const char *complex_function_get_exponent_str(int exp){
  int digits = floor(log10(exp) + 1);
  char str[digits+1];
  char *ret = calloc(sizeof(char) * (2*digits +2), 1);

  sprintf(str, "%d", exp);

  for (int i = 0; i < digits; i++){
    switch(str[i]){
      case '0':
        strcat(ret,  "⁰");
        break;
      case '1':
        strcat(ret,  "¹");
        break;
      case '2':
        strcat(ret,  "²");
        break;
      case '3':
        strcat(ret,  "³");
        break;
      case '4':
        strcat(ret,  "⁴");
        break;
      case '5':
        strcat(ret,  "⁵");
        break;
      case '6':
        strcat(ret,  "⁶");
        break;
      case '7':
        strcat(ret,  "⁷");
        break;
      case '8':
        strcat(ret,  "⁸");
        break;
      case '9':
        strcat(ret,  "⁹");
        break;
    }
  }

  return ret;
}


//IO
void complex_polynomial_print(ComplexPolynomial *cp, char var){
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
