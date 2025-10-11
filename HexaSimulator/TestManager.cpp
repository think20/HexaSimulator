#include "stdafx.h"
#include "TestManager.h"

#include <cstring>

void TestManager_t::CreateTests()
{
	for (unsigned int t = 0; t < TestCount; t++)
		AssembleTestPiece(t);
}
void TestManager_t::AssembleTestPiece(unsigned int TestID)
{
	Init[TestID] = new TestPieceInit_t;
	auto& NowInit = Init[TestID];

	auto& NowTest = Test[TestID];
	auto& NowComposite = Composite[NowTest->CompositeID];
	auto& NowSpectra = Spectra[NowTest->SpectraID];

	NowInit->MaxLoad = NowTest->MaxLoad;

	NowInit->Vf = NowComposite->Vf;
	//OffsetK and B not simulated yet;
	NowInit->SpecimenRadious = NowComposite->SpecimenRadious;
	NowInit->SlidingStrength0 = NowComposite->SlidingStrength0;
	NowInit->DeltaT = NowComposite->DeltaT;
	NowInit->dint = NowComposite->dint;

	NowInit->LoadStepMax = NowSpectra->LoadStepMax;
	NowInit->LoadPercent = new unsigned int[NowInit->LoadStepMax];
	for (unsigned int s = 0; s < NowInit->LoadStepMax; s++)
		NowInit->LoadPercent[s] = NowSpectra->LoadPercent[s];

	auto& NewFiber = Fiber[NowComposite->FiberID];
	auto& NowMatrix = Matrix[NowComposite->MatrixID];

	NowInit->Sigma0 = NewFiber->Sigma0;
	NowInit->mf = NewFiber->mf;
	NowInit->Lg = NewFiber->Lg;
	NowInit->Ef = NewFiber->Ef;
	NowInit->Niuf = NewFiber->Niuf;
	NowInit->Alphaf = NewFiber->Alphaf;
	NowInit->R0 = NewFiber->R0;
	NowInit->StrengthDecay = NewFiber->StrengthDecay;
	NowInit->ScatterDecay = NewFiber->ScatterDecay;
	
	NowInit->Sigmams = NowMatrix->Sigmams;
	NowInit->Em = NowMatrix->Em;
	NowInit->Nium = NowMatrix->Nium;
	NowInit->Alpham = NowMatrix->Alpham;
	NowInit->Cm = new double[2];
	NowInit->Mm = new double[2];
	for (unsigned int i = 0; i < 2; i++)
	{
		NowInit->Cm[i] = NowMatrix->Cm[i];
		NowInit->Mm[i] = NowMatrix->Mm[i];
	}
	NowInit->KLow = NowMatrix->KLow;
	NowInit->KIC = NowMatrix->KIC;
}
void TestManager_t::InputManager_t::GetRawData()
{
	for (; RecordCount < MaxRecordCount; RecordCount++)
	{
		if (fscanf(FIn, "%s\n", RawData[RecordCount]) == EOF) break;
	}
}
void TestManager_t::InputManager_t::GetFibers()
{
	auto& FiberTypeCount = TestManager->FiberTypeCount;

	//Skip "FiberTypeCount" Row
	Record++; 

	sscanf(RawData[Record], "%d,", &FiberTypeCount);

	//Skip "FiberID..." Row
	Record += 2;
	
	for (unsigned int Offset = 0; Offset < FiberTypeCount; Offset++)
	{
		TestManager->Fiber[Offset] = new Fiber_t;
		auto& NewFiber = TestManager->Fiber[Offset];
		NewFiber->FiberName = new char[MaxNameLen];
		memset(NewFiber->FiberName, 0, sizeof NewFiber->FiberName);

		char* HeadlessData;
		GetIDandName(RawData[Record], NewFiber->ID, NewFiber->FiberName, HeadlessData);
		sscanf(HeadlessData, "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf", &NewFiber->Sigma0, &NewFiber->mf, &NewFiber->Lg, &NewFiber->Ef, &NewFiber->Niuf, &NewFiber->Alphaf, &NewFiber->R0, &NewFiber->StrengthDecay, &NewFiber->ScatterDecay);
		NewFiber->Sigma0 *= E6;
		NewFiber->Lg *= EM3;
		NewFiber->Ef *= E9;
		NewFiber->Alphaf *= EM6;
		NewFiber->R0 *= EM6;

		Record++;
	}
}
void TestManager_t::InputManager_t::GetMatrixs()
{
	auto& MatrixTypeCount = TestManager->MatrixTypeCount;

	Record++;

	sscanf(RawData[Record], "%d,", &MatrixTypeCount);

	Record += 2;

	for (unsigned int Offset = 0; Offset < MatrixTypeCount; Offset++)
	{
		TestManager->Matrix[Offset] = new Matrix_t;
		auto& NewMatrix = TestManager->Matrix[Offset];
		NewMatrix->MatrixName = new char[MaxNameLen];
		memset(NewMatrix->MatrixName, 0, sizeof NewMatrix->MatrixName);

		char* HeadlessData;
		GetIDandName(RawData[Record], NewMatrix->ID, NewMatrix->MatrixName, HeadlessData);

		NewMatrix->Cm = new double[2];
		NewMatrix->Mm = new double[2];
		for (unsigned int i = 0; i < 2; i++)
		{
			NewMatrix->Cm[i] = 0;
			NewMatrix->Mm[i] = 0;
		}

		sscanf(HeadlessData, "%lf,0,0,%lf,%lf,%lf,0,0,0,%lf,%lf,%lf,%lf,%lf,%lf", &NewMatrix->Sigmams, &NewMatrix->Em, &NewMatrix->Nium, &NewMatrix->Alpham, &NewMatrix->Cm[0], &NewMatrix->Cm[1], &NewMatrix->Mm[0], &NewMatrix->Mm[1], &NewMatrix->KLow, &NewMatrix->KIC);
		NewMatrix->Sigmams *= E6;
		NewMatrix->Em *= E9;
		NewMatrix->Alpham *= EM6;
		NewMatrix->Cm[0] *= EM11;
		NewMatrix->Cm[1] *= EM11;

		Record++;
	}
}
void TestManager_t::InputManager_t::GetComposites()
{
	auto& CompositeTypeCount = TestManager->CompositeTypeCount;

	Record++;

	sscanf(RawData[Record], "%d,", &CompositeTypeCount);

	Record += 2;

	for (unsigned int Offset = 0; Offset < CompositeTypeCount; Offset++)
	{
		TestManager->Composite[Offset] = new Composite_t;
		auto& NewComposite = TestManager->Composite[Offset];
		NewComposite->CompositeName = new char[MaxNameLen];
		memset(NewComposite->CompositeName, 0, sizeof NewComposite->CompositeName);

		char* HeadlessData;
		GetIDandName(RawData[Record], NewComposite->ID, NewComposite->CompositeName, HeadlessData);

		sscanf(HeadlessData, "%lf,%lf,%lf,%lf,%u,%u,%lf,%lf,%lf", &NewComposite->Vf, &NewComposite->OffsetK, &NewComposite->OffsetB, &NewComposite->SpecimenRadious, &NewComposite->FiberID, &NewComposite->MatrixID, &NewComposite->SlidingStrength0, &NewComposite->DeltaT, &NewComposite->dint);
		NewComposite->SpecimenRadious *= EM6;
		NewComposite->SlidingStrength0 *= E6;
		NewComposite->dint *= EM6;

		Record++;
	}
}
void TestManager_t::InputManager_t::GetTests()
{

}
void TestManager_t::InputManager_t::GetSpectras()
{
	auto& SpectraCount = TestManager->SpectraCount;
	
	Record++;

	sscanf(RawData[Record], "%d,", &SpectraCount);

	Record += 2;

	for (unsigned int Offset = 0; Offset < SpectraCount; Offset++)
	{
		TestManager->Spectra[Offset] = new Spectra_t;
		auto& NewSpectra = TestManager->Spectra[Offset];
		NewSpectra->SpectraName = new char[MaxNameLen];
		memset(NewSpectra, 0, sizeof NewSpectra);

		char* HeadlessData;
		GetIDandName(RawData[Record], NewSpectra->ID, NewSpectra->SpectraName, HeadlessData);

		sscanf(HeadlessData, "%d,", &NewSpectra->LoadStepMax);

		NewSpectra->LoadPercent = new unsigned int[NewSpectra->LoadStepMax];
		GetNums(HeadlessData, NewSpectra->LoadPercent, true);

		Record++;
	}
}
void TestManager_t::InputManager_t::GetIDandName(char* From, unsigned int& ID, char* Name, char* NewHead)
{
	unsigned int Offset = 0;
	for (; Offset < MaxRecordLen; Offset++)
	{
		auto& c = From[Offset];

		if (c == ',') break;

		if (c >= '0' && c <= '9')
		{
			ID *= 10;
			ID += c - '0';
		}
	}

	//Skip the first ','
	Offset++;

	unsigned int NameIndex = 0;
	for (; Offset < MaxRecordLen; Offset++)
	{
		auto& c = From[Offset];

		if (c == ',') break;

		Name[NameIndex] = c;
		NameIndex++;
	}

	//Skip the second ','
	Offset++;

	NewHead = &(From[Offset]);
}
void TestManager_t::InputManager_t::GetNums(char* From, unsigned int* Dest, bool IgnoreFirst)
{
	unsigned int Index = 0;
	unsigned int Num = 0;

	//Subsequent ','s means the end
	bool EndFlag = false;

	for (unsigned int Offset = 0; Offset < MaxRecordLen; Offset++)
	{
		auto& c = From[Offset];

		if (c == ',')
		{
			//two ','s together
			if (EndFlag) break;

			EndFlag = true;

			if (IgnoreFirst && Index == 0)
			{
				//Prevent ignoring all numbers;
				IgnoreFirst = false;
				Num = 0;
			}
			else
			{
				Dest[Index] = Num;
				Index++;
				Num = 0;
			}
		}
		else if (c >= '0' && c <= '9')
		{
			EndFlag = false;

			Num *= 10;
			Num += c - '0';
		}

	}
}

TestManager_t::InputManager_t::InputManager_t(TestManager_t* TestManager)
{
	std::string DirS(Dir);
	std::string TestDataS(TestData);

	RawData = new char* [MaxRecordCount];
	for (unsigned int r = 0; r < MaxRecordCount; r++)
	{
		RawData[r] = new char[MaxRecordLen];
		memset(RawData[r], 0, sizeof RawData[r]);
	}

	FIn = fopen((DirS + TestDataS).c_str(), "r");

	RecordCount = 0;
	Record = 0;

	this->TestManager = TestManager;
}
TestManager_t::TestManager_t()
{
	TestPiece = new TestPiece_t* [MaxTestPiece];
	Init = new TestPieceInit_t * [MaxTestPiece];
	Test = new Test_t* [MaxTestType];
	Spectra = new Spectra_t* [MaxSpectraType];
	Composite = new Composite_t* [MaxCompositeType];
	Matrix = new Matrix_t* [MaxMatrixType];
	Fiber = new Fiber_t * [MaxFiberType];

	InputManager = new TestManager_t::InputManager_t(this);
}