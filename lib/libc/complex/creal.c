#include <complex.h>

#undef creal
#define creal(x) ((double)(x))

double (creal)(double complex z)
{
	return creal(z);
}
