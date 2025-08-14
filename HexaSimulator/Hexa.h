#pragma once
#include "stdafx.h"
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <random>

constexpr unsigned int NeighberMax = 6;
constexpr unsigned int LayerMax = 15;
constexpr unsigned int StacksMax = 16;
constexpr unsigned int StackPerThread = StacksMax / ThreadMax;

constexpr unsigned int NodeMax = ((2 - 1) + (LayerMax - 1)) * (LayerMax - 1) / 2 * 6 + 1;

constexpr double R0 = 102 * EM6 * 0.5;
constexpr double SpecimenRadious = 1500 * EM6;

constexpr double Sigma0 = 3160 * E6;
constexpr double mf = 14.89;
constexpr double Lg = 25.4 * EM3; //25.4mm Gauge Length

constexpr double L = 100 * EM6; //100um segment
const double SigmaL = Sigma0 * powf(L, -1.0 / mf) * powf(Lg, 1.0 / mf);



//0.45vf = 361 fibers = F2(365)
//0.43vf = 337 fibers = F3(338)
//vf is estimated, fiber count is true

#ifdef FiberF2
constexpr double Vf = 0.45;
#endif

#ifdef FiberF3
constexpr double Vf = 0.43;
#endif

const double R1 = sqrt(pi / (2 * sqrt(3) * Vf)) * R0;


#ifdef FiberF2
constexpr double OffsetK = 0.2529;
constexpr double OffsetB = -0.6441;
#endif

#ifdef FiberF3
constexpr double OffsetK = 0.3363;
constexpr double OffsetB = 2.2076;
#endif

constexpr double ThetaThresh = 7; //above 7 degree of deviation cause compression damage
constexpr double OffsetFactor = 60;// 60% per deg

constexpr int FiberHealthMax = 3000;
constexpr unsigned int SlidingStrength0 = 100 * E6;

class Node_t
{
public:
	Node_t();

	unsigned int Stack;

	unsigned int Layer;
	unsigned int Order;
	unsigned int Index;

	Node_t* Neighbors[NeighberMax];
	unsigned int NeighborCount;

	double LoadFactor;
	double OffsetAngle;
	double OffsetDamageFactor;

	int FiberHealth;
	int64_t Strength;
	int64_t MaxStress;
	int64_t Load;
	int64_t SlidingStrength;

	double x, y, r;

	bool Broken;

	bool OverBoarder;
};

class Hexa_t
{
public:
	Hexa_t();	
	Node_t*** Nodes;
	unsigned int* NodeCount;
	int64_t* FiberBroken;
private:
	void HexaGenerateP();
	void HexasGenerate(unsigned int);
	void HexaGenerate(unsigned int);
	void GenerateLayer(unsigned int, unsigned int);
	void AddNeighbor(Node_t**, unsigned int, unsigned int);

	static void Convert23(const unsigned int&, const unsigned int&, unsigned int&, unsigned int&);
	static unsigned int Convert31(const unsigned int&, const unsigned int&, const unsigned int&);
	static unsigned int Convert21(const unsigned int&, const unsigned int&);

	unsigned int Finish;
	std::mutex mtx;
	std::condition_variable cv;



};


const double Orient[6][2] = { 1.0,0.0,1.0 / 2.0,sqrt(3.0) / 2.0,-1.0 / 2.0,sqrt(3.0) / 2.0,-1.0,0.0,-1.0 / 2.0,-sqrt(3.0) / 2.0,1.0 / 2.0,-sqrt(3.0) / 2.0 };