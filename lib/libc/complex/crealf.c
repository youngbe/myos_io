#include <complex.h>

#undef crealf
#define crealf(x) ((float)(x))

float (crealf)(float complex z)
{
	return crealf(z);
}
