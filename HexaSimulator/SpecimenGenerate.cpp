#include "Load.h"

void TestPiece_t::ParametersCopying(TestPieceInit_t Init)
{
	R0 = Init.R0;
	L = Init.L;
	SpecimenRadious = Init.SpecimenRadious;
	X0 = Init.X0;
	YLayer = Init.YLayer;
	Vf = Init.Vf;
	Vm = 1 - Vf;
	Em = Init.Em;
	Ef = Init.Ef;
	FiberLoadFactor = Ef / (Ef * Vf + Em * Vm);
	MatrixLoadFactor = Em / (Ef * Vf + Em * Vm);

	double Alphac = (Init.Alpham * Vm * Em + Init.Alphaf * Vf * Ef) / (Vf * Ef + Vm * Em);
	double Gamma = Ef * Em / (Ef * (1 + Init.Nium) + Em * (1 - Init.Niuf));

	double Sigmam_heat_radical = Gamma * (Alphac - Init.Alpham) * Init.DeltaT;
	double Sigmam_heat_axial = Em * (Alphac - Init.Alpham) * Init.DeltaT;

	SigmamMax = (2.0 * Sigmam_heat_radical + sqrt(16.0 * Init.Sigmams * Init.Sigmams - 12.0 * Sigmam_heat_radical * Sigmam_heat_radical) - 4.0 * Sigmam_heat_axial) / 4.0;

	LoadStepMax = Init.LoadStepMax;
	LoadPercent = new unsigned int[LoadStepMax];
	for (auto s = 0; s < LoadStepMax; s++) LoadPercent[s] = Init.LoadPercent[s];
	MaxLoad = Init.MaxLoad;
	CrackShape = Init.CrackShape;
	MaxLoadRatio = Init.MaxLoadRatio;
	FiberOverloadFactor = Init.FiberOverloadFactor;
	LoadSharingModel = Init.LoadSharingModel;
	SlidingStrength0 = Init.SlidingStrength0;
	DegTauN = Init.DegTauN; DegTauD = Init.DegTauD;
	DegSigN = Init.DegSigN; DegSigD = Init.DegSigD;
	DegT = Init.DegT;
	WakeC0 = Init.WakeC0;
	TipSCF = Init.TipSCF;
	TipZone = Init.TipZone;
}
void TestPiece_t::HexaInitReady(TestPieceInit_t Init)
{
	HexaInit = new HexaInit_t;
	HexaInit->R0 = Init.R0;
	HexaInit->SpecimenRadious = Init.SpecimenRadious;
	HexaInit->Rect = Init.Rect;
	HexaInit->X0 = Init.X0;
	HexaInit->YLayer = Init.YLayer;
	HexaInit->Vf = Init.Vf;
	HexaInit->mf = Init.mf;
	HexaInit->Sigma0 = Init.Sigma0;
	//printf("TestPiece:FiberStrength:%.2lfMPa\n", Init.Sigma0 * EM6);
	HexaInit->L = Init.L;
	HexaInit->Lg = Init.Lg;
	HexaInit->StrengthDecay = Init.StrengthDecay;
	HexaInit->ScatterDecay = Init.ScatterDecay;
	HexaInit->SlidingStrength0 = Init.SlidingStrength0;
	HexaInit->ICLCentral = Init.ICLCentral;
	HexaInit->ICLSide = Init.ICLSide;
	HexaInit->ICLRect = Init.ICLRect;
	HexaInit->SpecimenRadious = Init.SpecimenRadious;
	HexaInit->Em = Init.Em;
	HexaInit->Ef = Init.Ef;
	HexaInit->Vm = this->Vm;
	HexaInit->SigmamMax = SigmamMax;
	HexaInit->KLow = Init.KLow;
	HexaInit->KIC = Init.KIC;
	HexaInit->Cm = new double[2];
	HexaInit->Mm = new double[2];
	for (unsigned int i = 0; i < 2; i++)
	{
		HexaInit->Cm[i] = Init.Cm[i];
		HexaInit->Mm[i] = Init.Mm[i];
	}
	HexaInit->CrackShape = Init.CrackShape;
	HexaInit->OrientDistributionModel = Init.OrientDistributionModel;
	HexaInit->DegTauN = Init.DegTauN; HexaInit->DegTauD = Init.DegTauD;
	HexaInit->DegSigN = Init.DegSigN; HexaInit->DegSigD = Init.DegSigD;
	HexaInit->DegT = Init.DegT;
	HexaInit->SmallCrackCB = Init.SmallCrackCB;
}
Slice_t::Slice_t(Hexa_t* Hexa, unsigned int Slice)
{
	this->Hexa = Hexa;
	this->Slice = Slice;
	this->NodeCount = Hexa->NodeCount[Slice];
	this->Nodes = new TVector_t<Node_t*>(this->NodeCount + 10);
	unsigned int SIndex = 0;
	for (unsigned int Index = 1; Index < NodeMax; Index++)
	{
		auto& Node = Hexa->Nodes[Slice][Index];
		if (!Node->OverBoarder)
		{
			Node->SIndex = SIndex++;
			this->Nodes->Push(Node);
			//if (Slice == 0)
				//printf("Slice:%.2lf,%.2lf,%.2lf,%.2lf\n", Node->x * E6, Node->y * E6, Node->r*E6, Node->theta*180/3.14);
		}
	}
	this->Capablity = Hexa->Capablity[Slice];
	this->Crack = Hexa->CrackManager->Crack[Slice];
	this->RectSideCrack = Hexa->CrackManager->RectSideCrack[Slice];
	this->SideCrack = Hexa->CrackManager->SideCrack[Slice];


	this->Visited = new int64_t[NodeMax];
	for (auto n = 0; n < NodeMax; n++)
		this->Visited[n] = 0;

	this->Color = 1; //if Color is 0,the fist overload node would have no neighbor to visit

	this->OverLoadNodes = new TVector_t<Node_t*>(this->NodeCount+10);
	this->BFSSeq = new TQueue_t<LLSNode_t>(this->NodeCount+10);
	this->AnsSeq = new TVector_t<LLSNode_t>(this->NodeCount+10);
	this->ToUpdate = new TQueue_t<Node_t*>(this->NodeCount+10);

	this->Father = new unsigned int[this->NodeCount+10];

	for (auto i = 0; i < this->NodeCount; i++)
		this->Father[i] = i;

	this->LoadtoTransfer = new uint64_t[this->NodeCount+10];
}
TestPiece_t::TestPiece_t(TestPieceInit_t Init)
{
	ParametersCopying(Init);
	HexaInitReady(Init);

	this->Hexa = new Hexa_t(*HexaInit);

	this->Slice = new Slice_t* [SlicesMax];
	for (unsigned int s = 0; s < SlicesMax; s++)
	{
		Slice[s] = new Slice_t(this->Hexa, s);
	}

	this->Ready = true;
	this->SyncTotalStep = 1;

	this->DebugCounter = new unsigned int[SlicesMax];
	for (unsigned int s = 0; s < SlicesMax; s++)
		DebugCounter[s] = 0;

	this->Finish = 0;


	//Slices calculated in order in a thread. One thread throw only one failure.
	this->FailureReport = new FailureReport_t * [ThreadMax];
	for (unsigned int t = 0; t < ThreadMax; t++)
		FailureReport[t] = nullptr;

	HasResult = false;
	Result = nullptr;
}