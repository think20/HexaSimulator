#pragma once
#include "stdafx.h"
#include "Load.h"

#include <cstdio>


constexpr unsigned int MaxFiberType = 4;
constexpr unsigned int MaxMatrixType = 32;
constexpr unsigned int MaxCompositeType = 32;
constexpr unsigned int MaxSpectraType = 4;
constexpr unsigned int MaxTestType = 128;
constexpr unsigned int MaxTestPiece = MaxTestType;
constexpr unsigned int MaxRecordCount = 256;
constexpr unsigned int MaxRecordLen = 256;
constexpr unsigned int MaxNameLen = 32;

//For PC
constexpr char Dir[] = "C:\\Users\\12155\\Source\\repos\\think20\\HexaSimulator\\HexaSimulator\\";

//For Laptop
//constexpr char Dir[] = "C:\\Users\\12155\\source\\repos\\HexaSimulator\\HexaSimulator\\";

constexpr char TestData[] = "TestDataC.csv";
constexpr char GeneralResult[] = "Result.csv";
constexpr char FractureReport[] = "Fractures.csv";

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
	bool Rect;
	double Vf, OffsetK, OffsetB, SpecimenRadious, X0;
	unsigned int YLayer;
	unsigned int FiberID, MatrixID;
	double SlidingStrength0;
	double DeltaT, dint;
	double DegTauN, DegTauD, DegSigN, DegSigD, DegT; //degradation: N_tau, D_tau, N_sig, D_sig, T(K), 0=disabled
};
class Spectra_t
{
public:
	unsigned int ID;
	char* SpectraName;
	unsigned int LoadStepMax;
	unsigned int* LoadPercent;
};
class Test_t
{
public:
	unsigned int ID;
	char* TestName;
	unsigned int CompositeID;
	unsigned int SpectraID;
	double MaxLoad;
	unsigned int DesiredLife;
	double ICLCentral, ICLSide, ICLRect;
	double L;
	double MaxLoadRatio; 
	double FiberOverloadFactor;
	unsigned int OrientDistributionModel;
	unsigned int LoadSharingModel; //0=LLS,1=GLS
};
class TestManager_t
{
public:
	TestManager_t();

	void CreateTests();
	void StartTests();
	void SaveResults();

private:
	void AssembleTestPiece(unsigned int);

	class InputManager_t
	{
	public:
		InputManager_t(TestManager_t*);
		void GetTestContent();
	private:
		void GetRawData();
		void GetFibers();
		void GetMatrixs();
		void GetComposites();
		void GetSpectras();
		void GetTests();

		void GetIDandName(char*, unsigned int&, char*, unsigned int&);
		void GetNums(char*, unsigned int*, bool);

		char** RawData;
		unsigned int RecordCount;
		unsigned int Record;

		FILE* FIn;

		TestManager_t* TestManager;
	};
	InputManager_t* InputManager;
	class OutputManager_t
	{
	public:
		OutputManager_t(TestManager_t*);
		void SaveBasicResults();
		void SaveFractures();
	private:

		FILE* FOut;
		FILE* FracturesOut;
		TestManager_t* TestManager;
	};
	OutputManager_t* OutputManager;

	TestPiece_t** TestPiece;
	TestPieceInit_t** Init;
	Test_t** Test;
	Spectra_t** Spectra;
	Composite_t** Composite;
	Matrix_t** Matrix;
	Fiber_t** Fiber;
	FailureReport_t** Report;

	unsigned int TestCount, SpectraCount, CompositeTypeCount, MatrixTypeCount, FiberTypeCount;
};

