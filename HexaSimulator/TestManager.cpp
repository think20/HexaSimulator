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

	auto& NowFiber = Fiber[NowComposite->FiberID];
	auto& NowMatrix = Matrix[NowComposite->MatrixID];

	NowInit->Sigma0 = NowFiber->Sigma0;
	NowInit->mf = NowFiber->mf;
	NowInit->Lg = NowFiber->Lg;
	NowInit->Ef = NowFiber->Ef;
	NowInit->Niuf = NowFiber->Niuf;
	NowInit->Alphaf = NowFiber->Alphaf;
	NowInit->R0 = NowFiber->R0;
	NowInit->StrengthDecay = NowFiber->StrengthDecay;
	NowInit->ScatterDecay = NowFiber->ScatterDecay;
	
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

	sscanf(RawData[Record], "%d", &FiberTypeCount);

	//Skip "FiberID..." Row
	Record++;
	
	for (unsigned int Offset = 0; Offset < FiberTypeCount; Offset++)
	{
		TestManager->Fiber[Offset] = new Fiber_t;
		auto& Fiber = TestManager->Fiber[Offset];
		Fiber->FiberName = new char[MaxNameLen];
		memset(Fiber->FiberName, 0, sizeof Fiber->FiberName);


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