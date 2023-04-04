#include "complex_impl.h"

#undef cimag
#undef __CIMAG

#define __CIMAG(x, t) \
    (+(union { _Complex t __z; t __xy[2]; }){(_Complex t)(x)}.__xy[1])

#define cimag(x) __CIMAG(x, double)

double (cimag)(double complex z)
{
	return cimag(z);
}
