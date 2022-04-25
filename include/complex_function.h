#ifndef COMPLEX_FUNCTION_H
#define COMPLEX_FUNCTION_h

typedef struct ComplexPolynomial ComplexPolynomial;

//Constructors
ComplexPolynomial *complex_polynomial_new(ComplexPolynomial **);

//Memory management
void complex_polynomial_free_members(ComplexPolynomial *);

//Configure polynomials
void complex_polynomial_set_order(ComplexPolynomial *cp, int);
int complex_polynomial_get_order(ComplexPolynomial *cp);
void complex_polynomial_set_member(ComplexPolynomial *cp, complex double, int);
complex double complex_polynomial_get_member(ComplexPolynomial *cp, int);
void complex_polynomial_set_z(ComplexPolynomial *cp, complex double z);
complex double complex_polynomial_get_z(ComplexPolynomial *cp);

//IO
void complex_polynomial_print(ComplexPolynomial *cp);

#endif //COMPLEX_FUNCTION_H
