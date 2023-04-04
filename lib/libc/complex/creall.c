#include <complex.h>

#undef creall
#define creall(x) ((long double)(x))

long double (creall)(long double complex z)
{
	return creall(z);
}
