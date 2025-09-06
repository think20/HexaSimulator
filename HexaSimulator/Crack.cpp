#include "stdafx.h"
#include "Material.h"
#include "Crack.h"
#include <math.h>

double const& Crack_t::UpdateCrack(double Sigma)
{
	double Sigmab0 = Sigmab; //Last Sigmab

	//Yb = sqrtl(1.0 / cosl(pi * a * a / (2 * Req * Req)));
	Yb = Em * Req * Req / (a * a * Em * (Vf - 1) + Req * Req * (Em + Ef * Vf - Em * Vf));
	Sigmab = Yb * Sigma;

	if (Sigmab > SigmamMax)
	{
		Sigmab = SigmamMax;
		Yield = true;
	}
	else Yield = false;

	if (not(Sigmab > Sigmab0)) return a;

	double deltaKb = (Sigmab-Sigmab0) * sqrtl(pi * a) * EM6;// MPa/m^1/2
	double dadn = Cm * powl(deltaKb, Mm);//m
	
	a += dadn;

	return a;
}
double const& Crack_t::Len()
{
	return a;
}

double SideCrack_t::UpdateCrack(double Sigma)
{
	double Kb0 = Kb;
	double Kc0 = Kc;

	double Yb = 0;
	double Yc = 0;

	for (unsigned int Order = 0; Order < bTerm; Order++)
	{
		Yb += bPara[Order] * powl((b / D), double(Order));
		Yc += cPara[Order] * powl((b / D), double(Order)); //original paper says b/D rather than c/D
	}

	Kb = Yb * Sigma * sqrt(pi * b) * EM6; //MPa
	Kc = Yc * Sigma * sqrt(pi * c) * EM6; //MPa

	double DeltaKb = Kb - Kb0;
	double DeltaKc = Kc - Kc0;

	if (DeltaKb > 0) b += Cm * powl(DeltaKb, Mm) * dbFactor;
	if (DeltaKc > 0) c += Cm * powl(DeltaKc, Mm) * dcFactor;

	//y=kx+b,Right half of the crack front
	k0 = (c * c - 2 * b * R * R) / (c * R * sqrt(4 - c * c / (R * R)));
	b0 = b - R;

	//Area of Cracked Composite
	S = (pi / 2 - acosl(c / (2 * R))) * R * R - 1 / 2 * (R - b) * c * sqrtl(1 - c * c / (4 * R * R));

	//equivalent crack length
	return b;
}

double SideCrack_t::DistanceToCrackFront(double x, double y)
{
	//only the right half of the crack is modeled, and the crack is about the symmetry of the y-axis
	x = fabs(x);

	return ((y - k0 * x - b0) * sqrt(1 / (k0 * k0 + 1)));
}