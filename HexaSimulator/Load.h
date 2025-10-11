#pragma once
#include "stdafx.h"

#include "Hexa.h"
#include "TDataStructure.h"

#ifdef vFiberF11
constexpr double MaxLoad = 800 * E6;
#define Spectra71
#endif

#ifdef vFiberF12
constexpr double MaxLoad = 600 * E6;
#define Spectra71
#endif

#ifdef vFiberF13
constexpr double MaxLoad = 400 * E6;
#define Spectra71
#endif

#ifdef vFiberF14
constexpr double MaxLoad = 300 * E6;
#define Spectra71
#endif

#ifdef F1
constexpr double MaxLoad = 1000 * E6;
#define Spectra71
#endif

#ifdef F2
constexpr double MaxLoad = 1250 * E6;
#define Spectra71
#endif

#ifdef F3
constexpr double MaxLoad = 1250 * E6;
#define Spectra71
#endif

#ifdef FiberT1
constexpr double MaxLoad = 3000 * E6;
#define IncLoad21
#endif

#ifdef LF1
constexpr double MaxLoad = 900 * E6;
#define Spectra71
#endif

#ifdef BLFRT1
constexpr double MaxLoad = 2100 * E6;
#define Spectra2
#endif

#ifdef BLFRT2
constexpr double MaxLoad = 1650 * E6;
#define Spectra2
#endif

#ifdef BLFRT3
constexpr double MaxLoad = 1200 * E6;
#define Spectra2
#endif

#ifdef BLFRT4
constexpr double MaxLoad = 1000 * E6;
#define Spectra2
#endif

#ifdef BLFRT5
constexpr double MaxLoad = 600 * E6;
#define Spectra2
#endif

#ifdef BLFRT6
constexpr double MaxLoad = 700 * E6;
#define Spectra2
#endif


#ifdef BLFHT1
constexpr double MaxLoad = 1600 * E6;
#define Spectra2
#endif

#ifdef BLFHT2
constexpr double MaxLoad = 1500 * E6;
#define Spectra2
#endif

#ifdef BLFHT3
constexpr double MaxLoad = 1350 * E6;
#define Spectra2
#endif

#ifdef BLFHT4
constexpr double MaxLoad = 1170 * E6;
#define Spectra2
#endif

#ifdef BLFHT5
constexpr double MaxLoad = 975 * E6;
#define Spectra2
#endif

#ifdef BLFHT6
constexpr double MaxLoad = 905 * E6;
#define Spectra2
#endif

#ifdef Spectra71
constexpr unsigned int LoadStepMax = 71;
constexpr unsigned int LoadPercent[LoadStepMax] = { 0,68,51,72,44,65,44,72,48,68,26,99,52,100,52,83,49,99,51,75,49,56,49,62,48,100,79,86,80,90,76,80,74,95,83,99,94,98,95,100,65,70,66,77,53,58,55,96,92,95,76,83,72,78,73,81,56,68,57,70,46,72,16,93,33,41,30,36,22,30,0 };
#endif

#ifdef IncLoad11
constexpr unsigned int LoadStepMax = 11;
constexpr unsigned int LoadPercent[LoadStepMax] = { 0,10,20,30,40,50,60,70,80,90,100 };
#endif


#ifdef IncLoad21
constexpr unsigned int LoadStepMax = 21;
constexpr unsigned int LoadPercent[LoadStepMax] = { 0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75,80,85,90,95,100 };
#endif

#ifdef Spectra4
constexpr unsigned int LoadStepMax = 4;
constexpr unsigned int LoadPercent[LoadStepMax] = { 10,50,100,50 };
#endif

#ifdef Spectra2
constexpr unsigned int LoadStepMax = 2;
constexpr unsigned int LoadPercent[LoadStepMax] = { 0,100 };
#endif

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

	double mf, Sigma0, Lg;
	double StrengthDecay, ScatterDecay;
	double SlidingStrength0;

	double InitCrackRadious;
	double KLow, KIC;
	double* Cm;
	double* Mm;
};
class TestPiece_t
{
public:
	TestPiece_t(TestPieceInit_t Init);
	void FiberLoadP(unsigned int);

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
};

