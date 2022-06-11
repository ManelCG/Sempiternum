#ifndef COMPLEX_FUNCTION_H
#define COMPLEX_FUNCTION_h

#include <stdbool.h>

typedef struct ComplexPolynomial ComplexPolynomial;

typedef struct RootArrayMember RootArrayMember;

//Constructors
ComplexPolynomial *complex_polynomial_new(ComplexPolynomial **, int order);
RootArrayMember *root_array_member_new(RootArrayMember **, ComplexPolynomial *, int order);

//RootArrayMember
const ComplexPolynomial *root_array_member_get_root(RootArrayMember *);
void root_array_member_set_root(RootArrayMember *, ComplexPolynomial *);
const ComplexPolynomial *root_array_member_new_root(RootArrayMember *, int order);
RootArrayMember *root_array_member_next(RootArrayMember *);
void root_array_member_set_next(RootArrayMember *, RootArrayMember *);

//Memory management
_Bool complex_polynomial_free_members(ComplexPolynomial *);
_Bool root_array_member_destroy(RootArrayMember *);

//Configure polynomials
void complex_polynomial_set_order(ComplexPolynomial *cp, int);
int complex_polynomial_get_order(ComplexPolynomial *cp);
void complex_polynomial_set_member(ComplexPolynomial *cp, complex double, int);
complex double complex_polynomial_get_member(ComplexPolynomial *cp, int);

const complex double *complex_polynomial_get_polynomial(ComplexPolynomial *cp);

//IO
void complex_polynomial_print(ComplexPolynomial *cp);

#endif //COMPLEX_FUNCTION_H
