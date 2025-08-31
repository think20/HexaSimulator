#include "stdafx.h"
#include "Material.h"
#include "Crack.h"
#include <math.h>


double Crack_t::dadN(double Sigma, double a)
{
	double R = 1500 * EM6;
	double Yb = sqrtl(1.0 / cosl(pi * a / (2 * R)));
	double deltaKb = Yb * Sigma * sqrtl(pi * a) * (1 - 0.1) * EM6;// MPa/m^1/2
	double dadn = Cm * powl(deltaKb, Mm); // m
	return dadn;
}