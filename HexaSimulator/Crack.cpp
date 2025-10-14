#include "stdafx.h"

#include "Crack.h"
#include <math.h>
#include <cstdio>

double const& Crack_t::UpdateCrack(double Sigma)
{
	double Sigmab0 = Sigmab; //Last Sigmab

	//Yb = sqrtl(1.0 / cosl(pi * a * a / (2 * Req * Req)));
	Yb = Em * Req * Req / (a * a * Em * (Vf - 1) + Req * Req * (Em + Ef * Vf - Em * Vf));
	Sigmab = Yb * Sigma;

	//printf("Crack:Yb:%.2lf,Sigma:%.2lf\n", Yb, Sigma);

	if (Sigmab > SigmamMax)
	{
		Sigmab = SigmamMax;
		Yield = true;
	}
	else Yield = false;

	//printf("Crack:Sigmab%lf,Sigmab0%lf\n");
	//printf("Crack:a %lf\n", a);

	if (not(Sigmab > Sigmab0)) return a;
	
	//printf("Crack:Propgated!\n");

	double deltaKb = (Sigmab-Sigmab0) * sqrtl(pi * a) * EM6;// MPa/m^1/2
	double Kb = Sigmab * sqrtl(pi * a) * EM6;

	if (Kb > KIC)
	{
		a = MaxCrackLen+1*EM6;
		return a;
	}

	double Cb = Cm[0];
	double Mb = Mm[0];

	if (deltaKb > KLow)
	{
		Cb = Cm[1];
		Mb = Mm[1];
	}
	double dadn = Cb * powl(deltaKb, Mb);//m

	//printf("Crack:dadn %lf\n", dadn);
	
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

	//printf("kb:%.2lf MPa/m^1/2\n", Kb);

	if (Kb > KIC)
	{
		b = MaxCrackLen+1*EM6;
		return b;
	}

	double DeltaKb = Kb - Kb0;
	double DeltaKc = Kc - Kc0;

	double Cb = Cm[0];
	double Cc = Cm[0];

	double Mb = Mm[0];
	double Mc = Mm[0];

	if (DeltaKb>KLow)
	{
		Cb = Cm[1];
		Mb = Mm[1];
	}
	if (DeltaKc>KLow)
	{
		Cc = Cm[1];
		Mc = Mm[1];
	}
	//printf("%.2lf\n", DeltaKb);

	if (DeltaKb > 0) b += Cb * powl(DeltaKb, Mb) * dbFactor;
	if (DeltaKc > 0) c += Cc * powl(DeltaKc, Mc) * dcFactor;

	//printf("%.2lf\n", b * E6);

	//y=kx+b,Right half of the crack front
	k0 = (c * c - 2 * b * R * R) / (c * R * sqrt(4 - c * c / (R * R)));
	b0 = b - R;

	//Area of Cracked Composite
	S = (pi / 2 - acosl(c / (2 * R))) * R * R - 1 / 2 * (R - b) * c * sqrtl(1 - c * c / (4 * R * R));

	//equivalent crack length
	//printf("SideCrackLen:%.2lfum\n", b * E6);

	return b;
}

double SideCrack_t::DistanceToCrackFront(double x, double y)
{
	x = x * cosf(Orient);
	y = y * sinf(Orient);

	//only the right half of the crack is modeled, and the crack is about the symmetry of the y-axis
	x = fabs(x);

	return ((y - k0 * x - b0) * sqrt(1 / (k0 * k0 + 1)));
}
double SideCrack_t::Len()
{
	return b;
}
void Crack_t::ParametersCopying(CrackInit_t Init)
{
	Em = Init.Em;
	Ef = Init.Ef;
	Vm = Init.Vm;
	Vf = Init.Vf;
	SigmamMax = Init.SigmamMax;
	KLow = Init.KLow;
	KIC = Init.KIC;
	Cm = new double[2];
	Mm = new double[2];
	for (unsigned int i = 0; i < 2; i++)
	{
		Cm[i] = Init.Cm[i];
		Mm[i] = Init.Mm[i];
	}
}
Crack_t::Crack_t(double a0, double R, CrackInit_t Init)
{
	ParametersCopying(Init);

	Yb = 0;
	Yield = false;
	MaxCrackLen = 0.99 * R;
	Sigmab = 0;
	a = a0;
	Req = R;
}
void SideCrack_t::ParametersCopying(SideCrackInit_t Init)
{
	KLow = Init.KLow;
	KIC = Init.KIC;
	Cm = new double[2];
	Mm = new double[2];
	for (unsigned int i = 0; i < 2; i++)
	{
		Cm[i] = Init.Cm[i];
		Mm[i] = Init.Mm[i];
	}
}
SideCrack_t::SideCrack_t(double b0, double c0, double R, double Orient, SideCrackInit_t Init)
{
	ParametersCopying(Init);
	S = 0;
	MaxCrackLen = 0.99 * 2 * R;
	Yb = 0;
	Yc = 0;
	Kb = 0;
	Kc = 0;
	b = b0;
	c = c0;
	D = 2 * R;
	this->R = R;
	k0 = 0;
	b0 = 0;
	Orient = Orient;
}
void CrackManager_t::ParametersCopying(CrackManagerInit_t Init)
{
	InitCrackRadious = Init.InitCrackRadious;
	SpecimenRadious = Init.SpecimenRadious;
}
void CrackManager_t::CrackInitReady(CrackManagerInit_t Init)
{
	CrackInit = new CrackInit_t;
	CrackInit->Em = Init.Em;
	CrackInit->Ef = Init.Ef;
	CrackInit->Vm = Init.Vm;
	CrackInit->Vf = Init.Vf;
	CrackInit->SigmamMax = Init.SigmamMax;
	CrackInit->KLow = Init.KLow;
	CrackInit->KIC = Init.KIC;
	CrackInit->Cm = new double[2];
	CrackInit->Mm = new double[2];
	for (unsigned int i = 0; i < 2; i++)
	{
		CrackInit->Cm[i] = Init.Cm[i];
		CrackInit->Mm[i] = Init.Mm[i];
	}
}
void CrackManager_t::SideCrackInitReady(CrackManagerInit_t Init)
{
	SideCrackInit = new SideCrackInit_t;
	SideCrackInit->KLow = Init.KLow;
	SideCrackInit->KIC = Init.KIC;
	SideCrackInit->Cm = new double[2];
	SideCrackInit->Mm = new double[2];
	for (unsigned int i = 0; i < 2; i++)
	{
		SideCrackInit->Cm[i] = Init.Cm[i];
		SideCrackInit->Mm[i] = Init.Mm[i];
	}
}
CrackManager_t::CrackManager_t(CrackManagerInit_t Init)
{
	ParametersCopying(Init);
	CrackInitReady(Init);
	SideCrackInitReady(Init);

	this->Crack = new Crack_t* [MaxCrackCount];
	this->SideCrack = new SideCrack_t* [MaxSideCrackCount];

	for (auto s = 0; s < MaxCrackCount; s++)
	{
		Crack[s] = nullptr;
		SideCrack[s] = nullptr;
	}

	this->OrientDist = new std::uniform_real_distribution<>(0.0, 2.0 * pi);

}
Crack_t* CrackManager_t::CreateCrack(double Req, unsigned int Slice)
{
	auto NewCrack = new Crack_t(InitCrackRadious, Req, *CrackInit);
	Crack[Slice] = NewCrack;
	//printf("CrackManager: Created Crack with InitRadious:%.2lfum\n", InitCrackRadious * E6);
	return NewCrack;
}
SideCrack_t* CrackManager_t::CreateSideCrack(unsigned int Slice)
{
	double Orient = (*OrientDist)(RandDevice);
	auto NewSideCrack = new SideCrack_t(InitCrackRadious, InitCrackRadious, SpecimenRadious, Orient, *SideCrackInit);
	SideCrack[Slice] = NewSideCrack;
	return NewSideCrack;
}