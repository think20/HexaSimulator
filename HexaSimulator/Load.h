#pragma once
#include "stdafx.h"
#include "Hexa.h"
#include "TDataStructure.h"

constexpr double dAdN = 6.0 * EM6 / 10000;
constexpr double LoadIncrease = 0.0001;

constexpr double FiberLoadFactor = 2.395; 
constexpr double MatrixLoadFactor = 0.258;

constexpr double MartixYieldStrength = 700 * E6;//to be looked up
constexpr double HeatResidualStrength = 100 * E6;//to be looked up

constexpr unsigned int LoadStepMax = 71;
constexpr unsigned int LoadPercent[LoadStepMax] = { 0,68,51,72,44,65,44,72,48,68,26,99,52,100,52,83,49,99,51,75,49,56,49,62,48,100,79,86,80,90,76,80,74,95,83,99,94,98,95,100,65,70,66,77,53,58,55,96,92,95,76,83,72,78,73,81,56,68,57,70,46,72,16,93,33,41,30,36,22,30,0 };

//constexpr unsigned int LoadStepMax = 11;
//constexpr unsigned int LoadPercent[LoadStepMax] = { 0,10,20,30,40,50,60,70,80,90,100 };

constexpr double MaxLoad = 1250 * E6;

constexpr unsigned int LLSIMax = 3;
constexpr double LLSFactor[LLSIMax + 1] = { 0,3,2,1 };
constexpr unsigned int LLSNeighborMax = ((2 - 1) + (LLSIMax - 1)) * (LLSIMax - 1) / 2 * 6;

constexpr double MaxLoadRatio = 0.95;

constexpr unsigned int StackOverload = 1;

class LLSNode_t
{
public:
	LLSNode_t(LLSNode_t& LLSNode):Node(LLSNode.Node),LLSIndex(LLSNode.LLSIndex){}
	LLSNode_t() :Node(nullptr), LLSIndex(0){}
	Node_t* Node;
	unsigned int LLSIndex;
	
};

class TestPiece_t
{
public:
	TestPiece_t();
	void FiberLoadP(unsigned int);

private:
	void FibersLoad(unsigned int, unsigned int);
	void FiberLoad(unsigned int, unsigned int);

	void LoadDistribute(unsigned int,unsigned int);
	void LoadRedistribute(unsigned int);
	void FiberDamage(unsigned int);

	void UpdateMaxStress();

	void FindNeighbors(unsigned int);

	Hexa_t* Hexa;

	uint64_t** Visited;
	uint64_t* Color;

	TQueue_t<Node_t*>** OverLoadNodes;
	TQueue_t<LLSNode_t>** BFSSeq;
	TVector_t<LLSNode_t>** AnsSeq;
	TQueue_t<Node_t*>** ToUpdate;

	unsigned int* DebugCounter;

	unsigned int Finish;
	std::mutex mtx;
	std::condition_variable cv,cvm;

	bool Ready;
	unsigned int SyncTotalStep;

};

