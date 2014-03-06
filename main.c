#include <stdio.h>

#include "integrator.h"

long double f(long double x)
{
	return x * x;
}

int main(int argc, char **argv)
{
	long double res = integrator(4, 0, 5, 0.0000001, f);
	printf("%Lf\n", res);
	return 0;
}