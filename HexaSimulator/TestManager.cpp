#include "stdafx.h"
#include "TestManager.h"

#include <cstring>
#include <thread>

void TestManager_t::CreateTests()
{
	InputManager->GetTestContent();

	//Two testpieces are assembled from each test setup with different crack shape
	for (unsigned int t = 0; t < TestCount; t++)
		AssembleTestPiece(t);
}
void TestManager_t::StartTests()
{
	for (unsigned int t = 0; t < TestCount; t++)
	{
		printf("TestManager:Evaluating Test%u out of %u test(s)...", t, TestCount);
		// Rect specimens only create one TestPiece (at [t]); non-Rect creates two
		auto Piece = TestPiece[t + TestCount] ? TestPiece[t + TestCount] : TestPiece[t];
		auto ResultS = Piece->FiberLoadP(MaxCycle);
		Report[t + TestCount] = new FailureReport_t(ResultS);
		printf("Complete!\n");
	}
}
void TestManager_t::SaveResults()
{
	OutputManager->SaveBasicResults();
	OutputManager->SaveFractures();
}
void TestManager_t::AssembleTestPiece(unsigned int TestID)
{
	Init[TestID] = new TestPieceInit_t;
	auto& NowInit = Init[TestID];

	auto& NowTest = Test[TestID];
	auto& NowComposite = Composite[NowTest->CompositeID];
	auto& NowSpectra = Spectra[NowTest->SpectraID];

	NowInit->MaxLoad = NowTest->MaxLoad;
	NowInit->ICLCentral = NowTest->ICLCentral;
	NowInit->ICLSide = NowTest->ICLSide;
	NowInit->ICLRect = NowTest->ICLRect;
	NowInit->L = NowTest->L;
	NowInit->MaxLoadRatio = NowTest->MaxLoadRatio;
	NowInit->FiberOverloadFactor = NowTest->FiberOverloadFactor;
	NowInit->OrientDistributionModel = NowTest->OrientDistributionModel;
	NowInit->LoadSharingModel = NowTest->LoadSharingModel;

	NowInit->Vf = NowComposite->Vf;
	NowInit->OffsetK = NowComposite->OffsetK;
	NowInit->OffsetB = NowComposite->OffsetB;
	NowInit->SpecimenRadious = NowComposite->SpecimenRadious;
	NowInit->Rect = NowComposite->Rect;
	NowInit->X0 = NowComposite->X0;
	NowInit->YLayer = NowComposite->YLayer;
	NowInit->SlidingStrength0 = NowComposite->SlidingStrength0;
	NowInit->DeltaT = NowComposite->DeltaT;
	NowInit->dint = NowComposite->dint;
	NowInit->DegTauN = NowComposite->DegTauN; NowInit->DegTauD = NowComposite->DegTauD;
	NowInit->DegSigN = NowComposite->DegSigN; NowInit->DegSigD = NowComposite->DegSigD;
	NowInit->DegT = NowComposite->DegT;
	NowInit->SmallCrackCB = NowComposite->SmallCrackCB;
	NowInit->WakeC0 = NowTest->WakeC0;
	NowInit->TipSCF = NowTest->TipSCF;
	NowInit->TipZone = NowTest->TipZone;
	NowInit->LLSEff = NowTest->LLSEff;

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

	if (NowInit->Rect == false)
	{
		NowInit->CrackShape = CentralCrack;
		TestPiece[TestID] = new TestPiece_t(*NowInit);

		NowInit->CrackShape = SideCrack;
		//NowInit->CrackShape = SCAlatering;
		TestPiece[TestID + TestCount] = new TestPiece_t(*NowInit);
	}
	else
	{
		NowInit->CrackShape = RectSideCrack;
		TestPiece[TestID] = new TestPiece_t(*NowInit);
	}
	
	//printf("TestManager:FiberStrength:%.2lfMPa\n", NowInit->Sigma0 * EM6);
}
void TestManager_t::InputManager_t::GetTestContent()
{
	GetRawData();
	GetFibers();
	GetMatrixs();
	GetComposites();
	GetSpectras();
	GetTests();
}
void TestManager_t::InputManager_t::GetRawData()
{
	for (; RecordCount < MaxRecordCount; RecordCount++)
	{
		if (fscanf(FIn, "%s", RawData[RecordCount]) == EOF) break;
		//printf("Yes%d\n", RecordCount);
	}
}
void TestManager_t::InputManager_t::GetFibers()
{
	auto& FiberTypeCount = TestManager->FiberTypeCount;

	//Skip "FiberTypeCount" Row
	Record++; 

	sscanf(RawData[Record], "%d,", &FiberTypeCount);
	printf("TestManager:FiberType:%u\n", FiberTypeCount);

	//Skip "FiberID..." Row
	Record += 2;
	
	for (unsigned int Offset = 0; Offset < FiberTypeCount; Offset++)
	{
		TestManager->Fiber[Offset] = new Fiber_t;
		auto& NewFiber = TestManager->Fiber[Offset];
		NewFiber->FiberName = new char[MaxNameLen];
		memset(NewFiber->FiberName, 0, MaxNameLen);


		unsigned int StartPos = 0;
		GetIDandName(RawData[Record], NewFiber->ID, NewFiber->FiberName, StartPos);
		auto HeadlessData = &(RawData[Record][StartPos]);
		sscanf(HeadlessData, "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf", &NewFiber->Sigma0, &NewFiber->mf, &NewFiber->Lg, &NewFiber->Ef, &NewFiber->Niuf, &NewFiber->Alphaf, &NewFiber->R0, &NewFiber->StrengthDecay, &NewFiber->ScatterDecay);
		NewFiber->Sigma0 *= E6;
		NewFiber->Lg *= EM3;
		NewFiber->Ef *= E9;
		NewFiber->Alphaf *= EM6;
		NewFiber->R0 *= EM6;

		//printf("ID:%u,%lf\n",NewFiber->ID, NewFiber->Sigma0, NewFiber->mf);

		Record++;
	}
}
void TestManager_t::InputManager_t::GetMatrixs()
{
	auto& MatrixTypeCount = TestManager->MatrixTypeCount;

	Record++;

	sscanf(RawData[Record], "%d,", &MatrixTypeCount);
	printf("TestManager:MatrixType:%u\n", MatrixTypeCount);

	Record += 2;

	for (unsigned int Offset = 0; Offset < MatrixTypeCount; Offset++)
	{
		TestManager->Matrix[Offset] = new Matrix_t;
		auto& NewMatrix = TestManager->Matrix[Offset];
		NewMatrix->MatrixName = new char[MaxNameLen];
		memset(NewMatrix->MatrixName, 0, MaxNameLen);

		unsigned int StartPos = 0;
		GetIDandName(RawData[Record], NewMatrix->ID, NewMatrix->MatrixName, StartPos);
		auto HeadlessData = &(RawData[Record][StartPos]);

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
	printf("TestManager:CompositeType:%u\n", CompositeTypeCount);

	Record += 2;

	for (unsigned int Offset = 0; Offset < CompositeTypeCount; Offset++)
	{
		TestManager->Composite[Offset] = new Composite_t;
		auto& NewComposite = TestManager->Composite[Offset];
		NewComposite->CompositeName = new char[MaxNameLen];
		memset(NewComposite->CompositeName, 0, MaxNameLen);

		unsigned int StartPos = 0;
		GetIDandName(RawData[Record], NewComposite->ID, NewComposite->CompositeName, StartPos);
		auto HeadlessData = &(RawData[Record][StartPos]);

		unsigned int IsRect = 0;
		NewComposite->DegTauN = 0; NewComposite->DegTauD = 0;
		NewComposite->DegSigN = 0; NewComposite->DegSigD = 0; NewComposite->DegT = 0;
		NewComposite->SmallCrackCB = 1.0;
		sscanf(HeadlessData, "%lf,%lf,%lf,%lf,%u,%lf,%u,%u,%u,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf", &NewComposite->Vf, &NewComposite->OffsetK, &NewComposite->OffsetB, &NewComposite->SpecimenRadious, &IsRect, &NewComposite->X0, &NewComposite->YLayer, &NewComposite->FiberID, &NewComposite->MatrixID, &NewComposite->SlidingStrength0, &NewComposite->DeltaT, &NewComposite->dint, &NewComposite->DegTauN, &NewComposite->DegTauD, &NewComposite->DegSigN, &NewComposite->DegSigD, &NewComposite->DegT, &NewComposite->SmallCrackCB);

		//uint -> bool
		NewComposite->Rect = IsRect == 1 ? true : false;
		NewComposite->X0 *= EM6;
		NewComposite->YLayer;
		NewComposite->SpecimenRadious *= EM6;
		NewComposite->SlidingStrength0 *= E6;
		NewComposite->dint *= EM6;

		Record++;
	}
}
void TestManager_t::InputManager_t::GetSpectras()
{
	auto& SpectraCount = TestManager->SpectraCount;
	
	Record++;

	sscanf(RawData[Record], "%d,", &SpectraCount);
	printf("TestManager:SpectraType:%u\n", SpectraCount);

	Record += 2;

	for (unsigned int Offset = 0; Offset < SpectraCount; Offset++)
	{
		TestManager->Spectra[Offset] = new Spectra_t;
		auto& NewSpectra = TestManager->Spectra[Offset];
		NewSpectra->SpectraName = new char[MaxNameLen];
		memset(NewSpectra, 0, sizeof NewSpectra);

		unsigned int StartPos = 0;
		GetIDandName(RawData[Record], NewSpectra->ID, NewSpectra->SpectraName, StartPos);
		auto HeadlessData = &(RawData[Record][StartPos]);

		sscanf(HeadlessData, "%d,", &NewSpectra->LoadStepMax);

		NewSpectra->LoadPercent = new unsigned int[NewSpectra->LoadStepMax];
		GetNums(HeadlessData, NewSpectra->LoadPercent, true);

		Record++;
	}
}
void TestManager_t::InputManager_t::GetTests()
{
	auto& TestCount = TestManager->TestCount;

	Record++;

	sscanf(RawData[Record], "%d,", &TestCount);
	printf("TestManager:TestType:%u\n", TestCount);

	Record += 2;

	for (unsigned int Offset = 0; Offset < TestCount; Offset++)
	{
		TestManager->Test[Offset] = new Test_t;
		auto& NewTest = TestManager->Test[Offset];
		NewTest->TestName = new char[MaxNameLen];
		memset(NewTest->TestName, 0, MaxNameLen);

		unsigned int StartPos = 0;
		GetIDandName(RawData[Record], NewTest->ID, NewTest->TestName, StartPos);
		auto HeadlessData = &(RawData[Record][StartPos]);

		NewTest->LoadSharingModel = 0; //default LLS for backward compatibility
		NewTest->WakeC0 = 150.0; NewTest->TipSCF = 1.0; NewTest->TipZone = 0.0; NewTest->LLSEff = 0.30;
		sscanf(HeadlessData, "%u,%u,%lf,%u,%lf,%lf,%lf,%lf,%lf,%lf,%u,%u,%lf,%lf,%lf,%lf", &NewTest->CompositeID, &NewTest->SpectraID, &NewTest->MaxLoad, &NewTest->DesiredLife,&NewTest->ICLCentral,&NewTest->ICLSide,&NewTest->ICLRect,&NewTest->L,&NewTest->MaxLoadRatio,&NewTest->FiberOverloadFactor,&NewTest->OrientDistributionModel,&NewTest->LoadSharingModel,&NewTest->WakeC0,&NewTest->TipSCF,&NewTest->TipZone,&NewTest->LLSEff);
		NewTest->MaxLoad *= E6;
		NewTest->ICLCentral *= EM6;
		NewTest->ICLSide *= EM6;
		NewTest->ICLRect *= EM6;
		//printf("TestManager:ICLC:%.2lf,ICLS:%.2lf,ICLR:%.2lf\n", NewTest->ICLCentral*E6, NewTest->ICLSide*E6, NewTest->ICLRect*E6);
		NewTest->L *= EM6;
		//printf("TestManager:InitCrackLen=%.2lfum\n", NewTest->InitCrackRadious * E6);

		Record++;
	}
}
void TestManager_t::InputManager_t::GetIDandName(char* From, unsigned int& ID, char* Name, unsigned int& Offset)
{
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
void TestManager_t::OutputManager_t::SaveBasicResults()
{
	fprintf(FOut, "Name,PeakLoad,DesiredLife,Life_C,Life_S,Breaks_C,Breaks_S\n");
	for (unsigned int t = 0; t < TestManager->TestCount; t++)
	{
		auto CountBreaks = [](FailureReport_t* R) {
			unsigned int Count = 0;
			if (R == nullptr) return Count;
			for (unsigned int s = 0; s < SlicesMax; s++)
				Count += R->Fractures[s]->DebugGetSize();
			return Count;
		};
		auto RC = TestManager->Report[t];
		auto RS = TestManager->Report[t + TestManager->TestCount];
		fprintf(FOut, "%s,%.0lf,%u,%u,%u,%u,%u\n",
			TestManager->Test[t]->TestName,
			TestManager->Test[t]->MaxLoad * EM6,
			TestManager->Test[t]->DesiredLife,
			RC ? RC->Cycle : 0,
			RS ? RS->Cycle : 0,
			CountBreaks(RC),
			CountBreaks(RS));
	}
	fclose(FOut);
}
void TestManager_t::OutputManager_t::SaveFractures()
{
	for (unsigned int t = 0; t < TestManager->TestCount; t++)
	{
		fprintf(FracturesOut, "Test%d:,\n", t);

		//Central Crack Fractures
		//auto& Report = (TestManager->Report[t]);
		
		//Side Crack Fractures
		auto& Report = (TestManager->Report[t+TestManager->TestCount]);

		auto Count = 0;
		for (unsigned int s = 0; s < SlicesMax; s++)
			Count += Report->Fractures[s]->DebugGetSize();
		fprintf(FracturesOut, "%u,\n", Count);

		//Discard First Layer
		for (unsigned int s = 0; s < SlicesMax; s++)
		{
			//fprintf(FracturesOut, "Slice %d:,\nCrack fronts:,\n", s);
			auto& CrackFront = Report->CrackFronts[s];
			while (!CrackFront->Empty())
			{
				auto FrontNow = (CrackFront->Front());
				//fprintf(FracturesOut, "%lf,%lf\n", FrontNow->X, FrontNow->Y);
				CrackFront->Pop();
			}
			//fprintf(FracturesOut, "Fractures:,\n");
			auto& Fractures = (Report->Fractures[s]);
			while (!Fractures->Empty())
			{
				auto PosNow = (Fractures->Front());
				fprintf(FracturesOut, "%lf,%lf,%u,%lf,%lf,%lf\n", PosNow->X, PosNow->Y, s, PosNow->R, PosNow->R2, PosNow->Theta);
				Fractures->Pop();
			}
		}
	}

	fclose(FracturesOut);
}
TestManager_t::InputManager_t::InputManager_t(TestManager_t* TestManager)
{
	std::string DirS(Dir);
	std::string TestDataS(TestData);

	RawData = new char* [MaxRecordCount];
	for (unsigned int r = 0; r < MaxRecordCount; r++)
	{
		RawData[r] = new char[MaxRecordLen];
		memset(RawData[r], 0, MaxRecordLen);
	}

	printf("Input from %s\n", (DirS + TestDataS).c_str());

	FIn = fopen((DirS + TestDataS).c_str(), "r");
	if (FIn == NULL)
		printf("Warning No Input!\n");

	RecordCount = 0;
	Record = 0;

	this->TestManager = TestManager;
}
TestManager_t::OutputManager_t::OutputManager_t(TestManager_t* TestManager)
{
	std::string DirS(Dir);
	std::string GeneralResultS(GeneralResult);
	std::string FractureS(FractureReport);

	FOut = fopen((DirS + GeneralResultS).c_str(), "w");
	FracturesOut = fopen((DirS + FractureS).c_str(), "w");

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
	Fiber = new Fiber_t* [MaxFiberType];
	Report = new FailureReport_t* [MaxTestType](); //value-initialized to nullptr

	InputManager = new TestManager_t::InputManager_t(this);
	OutputManager = new TestManager_t::OutputManager_t(this);
}