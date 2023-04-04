#include "complex_impl.h"

#undef __CIMAG
#undef cimagf

#define __CIMAG(x, t) \
    (+(union { _Complex t __z; t __xy[2]; }){(_Complex t)(x)}.__xy[1])

#define cimagf(x) __CIMAG(x, float)

float (cimagf)(float complex z)
{
	return cimagf(z);
}
