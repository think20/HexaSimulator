#include "stdafx.h"
#include "Material.h"
#include "Crack.h"
#include <math.h>

double const& Crack_t::UpdateCrack(double Sigma)
{
	Yb = sqrtl(1.0 / cosl(pi * a / (2 * Req)));

	double Sigmab0 = Sigmab;
	Sigmab = Yb * Sigma;

	if (not(Sigmab > Sigmab0)) return a;

	double deltaKb = (Sigmab-Sigmab0) * sqrtl(pi * a) * EM6;// MPa/m^1/2
	double dadn = Cm * powl(deltaKb, Mm);
	
	a += dadn;

	return a;
}