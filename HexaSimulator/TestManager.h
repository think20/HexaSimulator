#pragma once
#include "stdafx.h"
#include "Load.h"
class Fiber_t
{
public:
	unsigned int ID;
	char* FiberName;
	double Sigma0, mf, Lg, Ef, Niuf, Alphaf, R0, StrengthDecay, ScatterDecay;
};
class Matrix_t
{
public:
	unsigned int ID;
	char* MatrixName;
	double Sigmams, Em, Nium, Alpham;
	double* Cm;
	double* Mm;
	double KLow,KIC;
};
class Composite_t
{
public:
	unsigned int ID;
	char* CompositeName;
	double Vf, OffsetK, OffsetB, SpecimenRadious;
	unsigned int FiberID, MatrixID;
	double SlidingStrength0;
	double DeltaT, dint;
};
class Spectra_t
{
	unsigned int ID;
	char* SpectraName;
	unsigned int LoadStepMax;
	unsigned int* LoadPercent;
};
class Test_t
{
	unsigned int ID;
	char* TestName;
	unsigned int CompositeID;
	unsigned int SpectraID;
	double MaxLoad;
	unsigned int DesiredLife;
};
class InputManager_t
{
public:
	InputManager_t();
private:
	void GetFibers();

};

class TestManager_t
{
public:
	TestManager_t();
	void CreateTests();
	void StartTests();
	void DisplayResults();

private:


};