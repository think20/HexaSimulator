#include "stdafx.h"
#include "TestManager.h"
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
TestManager_t::TestManager_t()
{
	TestPiece = new TestPiece_t* [MaxTestPiece];
	Init = new TestPieceInit_t * [MaxTestPiece];
	Test = new Test_t* [MaxTestType];
	Spectra = new Spectra_t* [MaxSpectraType];
	Composite = new Composite_t* [MaxCompositeType];
	Matrix = new Matrix_t* [MaxMatrixType];
	Fiber = new Fiber_t * [MaxFiberType];

	InputManager = new TestManager_t::InputManager_t;
}