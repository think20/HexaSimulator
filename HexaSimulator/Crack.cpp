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
		printf("Crack:Exceeding KIC\n");
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

double RectSideCrack_t::UpdateCrack(double Sigma)
{
	double Sigmab0 = Sigmab; //Last Sigmab

	Yb = Em * X / (a * Em * (Vf - 1) + X * (Em + Ef * Vf - Em * Vf));
	Sigmab = Yb * Sigma;

	if (Sigmab > SigmamMax)
	{
		Sigmab = SigmamMax;
		Yield = true;
	}
	else Yield = false;

	if (not(Sigmab > Sigmab0)) return a;

	double deltaKb = (Sigmab - Sigmab0) * sqrtl(pi * a) * EM6;// MPa/m^1/2
	double Kb = Sigmab * sqrtl(pi * a) * EM6;

	if (Kb > KIC)
	{
		printf("RectSideCrack:Exceeding KIC\n");
		a = MaxCrackLen + 1 * EM6;
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

	a += dadn;

	return a;
}
double RectSideCrack_t::Len()
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
		//Yc += cPara[Order] * powl((c / D), double(Order));
	}

	//printf("SideCrack:Yb %.2lf Yc %.2lf\n", Yb, Yc);
	Kb = Yb * Sigma * sqrt(pi * b) * EM6; //MPa
	Kc = Yc * Sigma * sqrt(pi * c) * EM6; //MPa

	//printf("kb:%.2lf MPa/m^1/2\n", Kb);

	if (Kb > KIC || Kc > KIC)
	{
		printf("FAIL|SideCrack_KIC|b=%.1f|c=%.1f|Kb=%.1f|Kc=%.1f|KIC=%.1f\n", b*E6, c*E6, Kb, Kc, KIC);
		b = MaxCrackLen + 1 * EM6;
		return b;
	}

	double DeltaKb = Kb - Kb0;
	double DeltaKc = Kc - Kc0;

	// Fiber bridging correction (Niu/Liu/Sun model): bridging stress
	// increases with crack area, reducing effective ΔK at crack tip
	// Quadratic form: negligible at small cracks, dominant at large cracks
	double ratio = b / D;
	double BridgeFactor = 1.0 - BridgingBeta * ratio * ratio;
	if (BridgeFactor < 0.10) BridgeFactor = 0.10;
	DeltaKb *= BridgeFactor;
	DeltaKc *= BridgeFactor;

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

	if (b / D < 0.1) c = b * SmallCrackCB;

	//printf("SideCrack: b%.2lf c%.2lf\n", b * E6,c*E6);

	//y=kx+b,Right half of the crack front
	k0 = (c * c - 2 * b * R * R) / (c * R * sqrt(4 - c * c / (R * R)));
	b0 = b - R;

	//Area of Cracked Composite
	S = (pi / 2 - acosl(c / (2 * R))) * R * R - 1 / 2 * (R - b) * c * sqrtl(1 - c * c / (4 * R * R));

	//equivalent crack length
	//printf("SideCrackLen:%.2lfum\n", b * E6);

	return b;
}

double SideCrack_t::DistanceToCrackFront(double R, double Theta)
{
	auto ThetaRel = Theta + Orient;
	auto x = R * cos(ThetaRel);
	auto y = R * sin(ThetaRel);

	//only the right half of the crack is modeled, and the crack is about the symmetry of the y-axis
	x = fabs(x);

	return ((y - k0 * x - b0) * sqrt(1 / (k0 * k0 + 1)));
}
double SideCrack_t::Len()
{
	return b;
}
void SideCrack_t::Front(PlanePos_t** Pos)
{
	auto& bPos = Pos[0];
	auto& cPos = Pos[1];
	auto& c2Pos = Pos[2];
	auto& r = this->R;
	auto d0 = r * 2;

	auto bx = (r - b) * cos(-pi / 2 - Orient) / d0;
	auto by = (r - b) * sin(-pi / 2 - Orient) / d0;
	bPos = new PlanePos_t(bx, by);

	auto ThetaC = 2 * asin(c / (2 * r));
	auto cx = r * cos(-pi / 2 + ThetaC - Orient) / d0;
	auto cy = r * sin(-pi / 2 + ThetaC - Orient) / d0;
	cPos = new PlanePos_t(cx, cy);

	auto c2x = r * cos(-pi / 2 - ThetaC - Orient) / d0;
	auto c2y = r * sin(-pi / 2 - ThetaC - Orient) / d0;
	c2Pos = new PlanePos_t(c2x, c2y);

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
	MaxCrackLen = MaxCrackRatio * R;
	Sigmab = 0;
	a = a0;
	Req = R;
}
void RectSideCrack_t::ParametersCopying(RectSideCrackInit_t Init)
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
RectSideCrack_t::RectSideCrack_t(double a0, double X0, RectSideCrackInit_t Init)
{
	ParametersCopying(Init);

	Yb = 0;
	Yield = false;
	MaxCrackLen = MaxCrackRatio * X0;
	Sigmab = 0;
	a = a0;
	X = X0;
}
void SideCrack_t::ParametersCopying(SideCrackInit_t Init)
{
	KLow = Init.KLow;
	KIC = Init.KIC;
	SmallCrackCB = Init.SmallCrackCB;
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
	MaxCrackLen = MaxCrackRatio * 2 * R;
	Yb = 0;
	Yc = 0;
	Kb = 0;
	Kc = 0;
	b = b0;
	c = c0;
	D = 2 * R;
	this->R = R;
	this->k0 = 0;
	this->b0 = 0;
	this->Orient = Orient;
}
void CrackManager_t::ParametersCopying(CrackManagerInit_t Init)
{
	ICLCentral = Init.ICLCentral;
	ICLSide = Init.ICLSide;
	ICLRect = Init.ICLRect;
	SpecimenRadious = Init.SpecimenRadious;
	OrientDistributionModel = Init.OrientDistributionModel;
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
void CrackManager_t::RectSideCrackInitReady(CrackManagerInit_t Init)
{
	RectSideCrackInit = new RectSideCrackInit_t;
	RectSideCrackInit->Em = Init.Em;
	RectSideCrackInit->Ef = Init.Ef;
	RectSideCrackInit->Vm = Init.Vm;
	RectSideCrackInit->Vf = Init.Vf;
	RectSideCrackInit->SigmamMax = Init.SigmamMax;
	RectSideCrackInit->KLow = Init.KLow;
	RectSideCrackInit->KIC = Init.KIC;
	RectSideCrackInit->Cm = new double[2];
	RectSideCrackInit->Mm = new double[2];
	for (unsigned int i = 0; i < 2; i++)
	{
		RectSideCrackInit->Cm[i] = Init.Cm[i];
		RectSideCrackInit->Mm[i] = Init.Mm[i];
	}
}
void CrackManager_t::SideCrackInitReady(CrackManagerInit_t Init)
{
	SideCrackInit = new SideCrackInit_t;
	SideCrackInit->KLow = Init.KLow;
	SideCrackInit->KIC = Init.KIC;
	SideCrackInit->SmallCrackCB = Init.SmallCrackCB;
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
	RectSideCrackInitReady(Init);
	SideCrackInitReady(Init);

	this->Crack = new Crack_t* [MaxCrackCount];
	this->RectSideCrack = new RectSideCrack_t* [MaxCrackCount];
	this->SideCrack = new SideCrack_t* [MaxSideCrackCount];

	for (auto s = 0; s < MaxCrackCount; s++)
	{
		Crack[s] = nullptr;
		RectSideCrack[s] = nullptr;
		SideCrack[s] = nullptr;
	}

	if (OrientDistributionModel == RandomOrient)
		this->OrientDist = new std::uniform_real_distribution<>(-pi, pi);
	else if (OrientDistributionModel == FanShapeOrient)
		this->OrientDist = new std::uniform_real_distribution<>(-pi / 2, pi / 2);
	else
		this->OrientDist = nullptr;
}
Crack_t* CrackManager_t::CreateCrack(double Req, unsigned int Slice)
{
	auto NewCrack = new Crack_t(ICLCentral, Req, *CrackInit);
	Crack[Slice] = NewCrack;
	//printf("CrackManager: Created Crack with InitRadious:%.2lfum\n", InitCrackRadious * E6);
	return NewCrack;
}
RectSideCrack_t* CrackManager_t::CreateRectSideCrack(double X, unsigned int Slice)
{
	auto NewRectSideCrack = new RectSideCrack_t(ICLRect, X, *RectSideCrackInit);
	RectSideCrack[Slice] = NewRectSideCrack;
	//printf("CrackManager: Created Crack with InitRadious:%.2lfum\n", InitCrackRadious * E6);
	return NewRectSideCrack;
}
SideCrack_t* CrackManager_t::CreateSideCrack(unsigned int Slice)
{
	double Orient = 0;

	if ((OrientDistributionModel == RandomOrient) or (OrientDistributionModel == FanShapeOrient))
		Orient = (*OrientDist)(RandDevice);
	else if (OrientDistributionModel == FixedOrient)
		Orient = 0;

	//printf("CrackManager:Orient:%.2lf\n", Orient);
	auto NewSideCrack = new SideCrack_t(ICLSide, ICLSide * SideCrackInit->SmallCrackCB, SpecimenRadious, Orient, *SideCrackInit);
	SideCrack[Slice] = NewSideCrack;
	return NewSideCrack;
}