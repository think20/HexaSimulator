#pragma once
#include "stdafx.h"
#include "Load.h"

#include <cstdio>

constexpr unsigned int MaxFiberType = 4;
constexpr unsigned int MaxMatrixType = 8;
constexpr unsigned int MaxCompositeType = 16;
constexpr unsigned int MaxSpectraType = 4;
constexpr unsigned int MaxTestType = 32;
constexpr unsigned int MaxTestPiece = MaxTestType;
constexpr unsigned int MaxRecordCount = 128;
constexpr unsigned int MaxRecordLen = 256;
constexpr unsigned int MaxNameLen = 32;

constexpr char Dir[] = "C:\\Users\\12155\\source\\repos\\HexaSimulator\\HexaSimulator\\";
constexpr char TestData[] = "TestDataC.csv";

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
};
class TestManager_t
{
public:
	TestManager_t();
	void CreateTests();
	void StartTests();
	void DisplayResults();

private:
	void AssembleTestPiece(unsigned int);

	class InputManager_t
	{
	public:
		InputManager_t(TestManager_t*);
		void GetRawData();
	private:
		void GetFibers();
		void GetMatrixs();
		void GetComposites();
		void GetSpectras();
		void GetTests();

		void GetIDandName(char*, unsigned int&, char*, char*);
		void GetNums(char*, unsigned int*, bool);

		char** RawData;
		unsigned int RecordCount;
		unsigned int Record;

		FILE* FIn;
		TestManager_t* TestManager;
	};

	InputManager_t* InputManager;

	TestPiece_t** TestPiece;
	TestPieceInit_t** Init;
	Test_t** Test;
	Spectra_t** Spectra;
	Composite_t** Composite;
	Matrix_t** Matrix;
	Fiber_t** Fiber;

	unsigned int TestCount, SpectraCount, CompositeTypeCount, MatrixTypeCount, FiberTypeCount;
};

