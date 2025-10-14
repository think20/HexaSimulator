#pragma once
#include "stdafx.h"

#include "Hexa.h"
#include "TDataStructure.h"

constexpr unsigned int LLSIMax = 3;
constexpr double LLSFactor[LLSIMax + 1] = { 0,3,2,1 };
constexpr unsigned int LLSNeighborMax = ((2 - 1) + (LLSIMax - 1)) * (LLSIMax - 1) / 2 * 6;

constexpr double MaxLoadRatio = 0.95;
constexpr double FiberOverloadFactor = 100; // by trial,100~1000

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
	SideCrack_t* SideCrack;

	Hexa_t* Hexa;
	unsigned int Slice;

	int64_t* Visited;
	int64_t Color;

	TQueue_t<Node_t*>* OverLoadNodes;
	TQueue_t<LLSNode_t>* BFSSeq;
	TVector_t<LLSNode_t>* AnsSeq;
	TQueue_t<Node_t*>* ToUpdate;
};
class TestPieceInit_t
{
public:
	double R0, L, SpecimenRadious;

	double Vm, Vf, Em, Ef;

	double Alpham, Alphaf, Nium, Niuf;
	double DeltaT, dint;
	double Sigmams;

	unsigned int LoadStepMax;
	unsigned int* LoadPercent;
	double MaxLoad;

	unsigned int CrackShape;

	double mf, Sigma0, Lg;
	double StrengthDecay, ScatterDecay;
	double SlidingStrength0;

	double InitCrackRadious;
	double KLow, KIC;
	double* Cm;
	double* Mm;
};
class FailureReport_t;
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
	void LoadRedistribute(Slice_t*);
	void FiberDamage(Slice_t*);

	void UpdateMaxStress();

	void CrackPlaneLoadDistribute(Slice_t*, unsigned int, unsigned int);
	void SideCrackPlaneLoadDistribute(Slice_t*, unsigned int, unsigned int);
	void NonCrackLoadDistribute(Slice_t*, unsigned int, unsigned int);
	void SliceFiberOverload(Slice_t*, unsigned int,unsigned int, double);
	double FindFiberLoadBaseline(Slice_t*);
	void SetFiberMinimumStress(Slice_t*, Slice_t*);

	double FindFiberLoadBaselineWithCrack(Slice_t*);
	double FindFiberLoadBaselineWithSideCrack(Slice_t*);

	void FindNeighbors(Slice_t*);

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

	double R0, L, SpecimenRadious;

	double Vm, Vf, Em, Ef;
	double FiberLoadFactor, MatrixLoadFactor;

	double SigmamMax;

	unsigned int LoadStepMax;
	unsigned int* LoadPercent;
	double MaxLoad;

	HexaInit_t* HexaInit;

	FailureReport_t** FailureReport;
	bool HasResult;
	FailureReport_t* Result;

	unsigned int CrackShape;
};
class PlanePos_t
{
public:
	PlanePos_t(PlanePos_t& From) : X(From.X), Y(From.Y) {};
	PlanePos_t(unsigned int X, unsigned int Y) : X(X), Y(Y) {};

	unsigned int X, Y;
};
class FailureReport_t
{
public:
	FailureReport_t(unsigned int, unsigned int, double, TestPiece_t*);
	FailureReport_t(FailureReport_t& From);
	~FailureReport_t();
	unsigned int Slice, Cycle;
	double CrackLen;
	unsigned int* FractureCount;
	TQueue_t<PlanePos_t*>** Fractures;
};