#pragma once
#include "stdafx.h"

#include "Hexa.h"
#include "TDataStructure.h"

// LLS i=1: only ring-1 neighbors (FEM: ring-2 SCF < 2%, negligible)
// Uniform load sharing among 6 ring-1 neighbors preserves force balance
// No artificial iteration limit — convergence is physically determined
constexpr unsigned int LLSIMax = 1;
constexpr double LLSFactor[LLSIMax + 1] = { 0, 1 };
// LLSEfficiency now per-test from CSV (TestPiece_t::LLSEff)
constexpr unsigned int LLSNeighborMax = ((2 - 1) + (LLSIMax)) * (LLSIMax) / 2 * 6;

class LLSNode_t
{
public:
	LLSNode_t(LLSNode_t& LLSNode):Node(LLSNode.Node),LLSIndex(LLSNode.LLSIndex){}
	LLSNode_t() :Node(nullptr), LLSIndex(0){}
	Node_t* Node;
	unsigned int LLSIndex;
	
};
class Slice_t
{
public:
	Slice_t(Hexa_t*, unsigned int);
	TVector_t<Node_t*>* Nodes;
	unsigned int NodeCount;
	int64_t Capablity;
	Crack_t* Crack;
	RectSideCrack_t* RectSideCrack;
	SideCrack_t* SideCrack;

	Hexa_t* Hexa;
	unsigned int Slice;

	int64_t* Visited;
	int64_t Color;

	TVector_t<Node_t*>* OverLoadNodes;
	TQueue_t<LLSNode_t>* BFSSeq;
	TVector_t<LLSNode_t>* AnsSeq;
	TQueue_t<Node_t*>* ToUpdate;

	unsigned int* Father;
	uint64_t* LoadtoTransfer;

	unsigned int FindAncestor(unsigned int);
};
class TestPieceInit_t
{
public:
	double R0, L, SpecimenRadious, X0;
	unsigned int YLayer;

	double Vf, Em, Ef;

	double Alpham, Alphaf, Nium, Niuf;
	double DeltaT, dint;
	double Sigmams;

	unsigned int LoadStepMax;
	unsigned int* LoadPercent;
	double MaxLoad;

	unsigned int CrackShape;

	double MaxLoadRatio;
	double FiberOverloadFactor;

	unsigned int LoadSharingModel; //0=LLS,1=GLS

	bool Rect;
	double mf, Sigma0, Lg;
	double StrengthDecay, ScatterDecay;
	double SlidingStrength0;

	double ICLCentral,ICLSide,ICLRect;
	double KLow, KIC;
	double* Cm;
	double* Mm;
	unsigned int OrientDistributionModel;
	double DegTauN, DegTauD, DegSigN, DegSigD, DegT;
	double SmallCrackCB;
	double WakeC0;
	double TipSCF, TipZone;
	double OffsetK, OffsetB;
	double LLSEff;
};

class TestPiece_t
{
	friend class FailureReport_t;
public:
	TestPiece_t(TestPieceInit_t Init);
	FailureReport_t FiberLoadP(unsigned int);

private:
	void SyncLoadDistibute(unsigned int, unsigned int);

	void FibersLoad(unsigned int, unsigned int);
	void FiberLoad(Slice_t*, unsigned int);

	void LoadDistribute(Slice_t*,unsigned int,unsigned int);
	void LoadRedistribute(Slice_t*,unsigned int,unsigned int);
	void FiberDamage(Slice_t*);

	void UpdateMaxStress();
	void UpdateDegradation(unsigned int Cycle);

	void CrackPlaneLoadDistribute(Slice_t*, unsigned int, unsigned int);
	void RectSideCrackPlaneLoadDistribute(Slice_t*, unsigned int, unsigned int);
	void SideCrackPlaneLoadDistribute(Slice_t*, unsigned int, unsigned int);
	void NonCrackLoadDistribute(Slice_t*, unsigned int, unsigned int);

	bool LLSFiberOverload(Slice_t*, unsigned int, unsigned int);
	bool GLSFiberOverload(Slice_t*, unsigned int, unsigned int);

	void SliceFiberOverload(Slice_t*, unsigned int,unsigned int, double);
	double FindFiberLoadBaseline(Slice_t*);
	void SetFiberMinimumStress(Slice_t*, Slice_t*);

	double FindFiberLoadBaselineWithCrack(Slice_t*);
	double FindFiberLoadBaselineWithRectSideCrack(Slice_t*);
	double FindFiberLoadBaselineWithSideCrack(Slice_t*);

	void FindNeighbors(Slice_t*,unsigned int,unsigned int);

	void ShowSideCrackFront(Slice_t*);

	void ParametersCopying(TestPieceInit_t Init);
	void HexaInitReady(TestPieceInit_t Init);

	Hexa_t* Hexa;
	Slice_t** Slice;

	unsigned int* DebugCounter;

	unsigned int Finish;
	std::mutex mtx;
	std::condition_variable cv,cvm;

	bool Ready;
	unsigned int SyncTotalStep;

	double R0, L, SpecimenRadious, X0;
	unsigned int YLayer;

	double Vm, Vf, Em, Ef;
	double FiberLoadFactor, MatrixLoadFactor;

	double SigmamMax;

	unsigned int LoadStepMax;
	unsigned int* LoadPercent;
	double MaxLoad;

	unsigned int CrackShape;

	double MaxLoadRatio; //0.53
	double FiberOverloadFactor; // by trial,100~1000, 500 is good

	unsigned int LoadSharingModel; //0=LLS,1=GLS
	double SlidingStrength0;
	double DegTauN, DegTauD, DegSigN, DegSigD, DegT;
	double WakeC0;
	double TipSCF, TipZone;
	double LLSEff;

	HexaInit_t* HexaInit;

	FailureReport_t** FailureReport;
	bool HasResult;
	FailureReport_t* Result;

};
class FailureReport_t
{
public:
	FailureReport_t(unsigned int, unsigned int, double, TestPiece_t*, unsigned int Mode = 0);
	FailureReport_t(FailureReport_t& From);
	~FailureReport_t();
	unsigned int Slice, Cycle;
	unsigned int Mode;   // 1=CrackMaxLen 2=RectSideCrackMaxLen 3=SideCrackMaxLen 4=LoadRatio 5=FiberOverload
	double CrackLen;
	TQueue_t<PlanePos_t*>** Fractures;
	TQueue_t<PlanePos_t*>** CrackFronts;
};