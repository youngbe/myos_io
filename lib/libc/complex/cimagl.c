#include "complex_impl.h"

#undef __CIMAG
#undef cimagl

#define __CIMAG(x, t) \
    (+(union { _Complex t __z; t __xy[2]; }){(_Complex t)(x)}.__xy[1])

#define cimagl(x) __CIMAG(x, long double)

long double (cimagl)(long double complex z)
{
	return cimagl(z);
}
