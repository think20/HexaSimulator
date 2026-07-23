#pragma once
#include "stdafx.h"
#include "TDataStructure.h"

#include <random>

constexpr unsigned int MaxCrackCount = SlicesMax;
constexpr unsigned int MaxSideCrackCount = SlicesMax;
constexpr double MaxCrackRatio = 0.50; //0.50 for HT,0.99 for RT

class PlanePos_t
{
public:
	PlanePos_t(PlanePos_t& From) : X(From.X), Y(From.Y), R(From.R), R2(From.R2), Theta(From.Theta) {};
	PlanePos_t(double X, double Y) : X(X), Y(Y), R(sqrt(X* X + Y * Y)), R2(R* R), Theta(atan2(Y, X) * 180 / 3.14 + 180) {};

	double X, Y, R, R2, Theta;
};

class CrackInit_t
{
public:
	double Em, Ef, Vm, Vf;
	double SigmamMax;
	double KLow, KIC;
	double* Cm;
	double* Mm;

};

class Crack_t
{
public: 
	Crack_t(double, double, CrackInit_t Init);
	double const& UpdateCrack(double);
	double const& Len();
	double Yb;
	bool Yield;
	double MaxCrackLen;

private:
	void ParametersCopying(CrackInit_t Init);
	double Sigmab;
	double a;
	double Req;

	double Em, Ef, Vm, Vf;
	double SigmamMax;
	double KLow, KIC;
	double* Cm;
	double* Mm;
};

class RectSideCrackInit_t
{
public:
	double Em, Ef, Vm, Vf;
	double SigmamMax;
	double KLow, KIC;
	double* Cm;
	double* Mm;

};

class RectSideCrack_t
{

public:
	RectSideCrack_t(double, double, RectSideCrackInit_t Init);
	double UpdateCrack(double);
	double Len();
	double Yb;
	bool Yield;
	double MaxCrackLen;
private:
	void ParametersCopying(RectSideCrackInit_t Init);
	double Sigmab;
	double a;
	double X;

	double Em, Ef, Vm, Vf;
	double SigmamMax;
	double KLow, KIC;
	double* Cm;
	double* Mm;
};

constexpr unsigned int bTerm = 5;
constexpr unsigned int cTerm = 5;
constexpr double bPara[bTerm] = { 0.906,-1.23,0.599,12,-11.2 };
constexpr double cPara[cTerm] = { 1.12,-2.61,7.34,1.41,-4.28 };
constexpr double dbFactor = 1.6;
constexpr double dcFactor = 1;

class SideCrackInit_t
{
public:
	double KLow, KIC;
	double* Cm;
	double* Mm;
	double SmallCrackCB;
};

class FailureReport_t;
class SideCrack_t
{
	//friend class FailureReport_t;
public:
	SideCrack_t(double, double, double, double, SideCrackInit_t);
	double UpdateCrack(double);
	double DistanceToCrackFront(double, double);
	double Len();
	void Front(PlanePos_t**);
	double S;
	double MaxCrackLen;
private:
	void ParametersCopying(SideCrackInit_t Init);
	double Yb, Yc;
	double Kb, Kc;
	double b, c, D, R;
	double k0, b0;

	//Orient means the degree fibers rotate, or the degree the crack rotates counter-clockwise
	double Orient;

	double KLow, KIC;
	double* Cm;
	double* Mm;
	double SmallCrackCB;
};
class CrackManagerInit_t
{
public:
	double ICLCentral, ICLSide, ICLRect, SpecimenRadious;

	double Em, Ef, Vm, Vf;
	double SigmamMax;
	double KLow, KIC;
	double* Cm;
	double* Mm;
	unsigned int OrientDistributionModel;
	double SmallCrackCB;
};
class CrackManager_t
{
public:
	CrackManager_t(CrackManagerInit_t Init);
	Crack_t* CreateCrack(double, unsigned int);
	RectSideCrack_t* CreateRectSideCrack(double, unsigned int);
	SideCrack_t* CreateSideCrack(unsigned int);

	Crack_t** Crack;
	RectSideCrack_t** RectSideCrack;
	SideCrack_t** SideCrack;
private:
	void ParametersCopying(CrackManagerInit_t Init);
	void CrackInitReady(CrackManagerInit_t Init);
	void RectSideCrackInitReady(CrackManagerInit_t Init);
	void SideCrackInitReady(CrackManagerInit_t Init);
	std::random_device RandDevice;
	std::uniform_real_distribution<>* OrientDist;
	unsigned int Seed;

	double  ICLCentral, ICLSide, ICLRect, SpecimenRadious;
	unsigned int OrientDistributionModel;
	CrackInit_t* CrackInit;
	RectSideCrackInit_t* RectSideCrackInit;
	SideCrackInit_t* SideCrackInit;
};