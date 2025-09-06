#pragma once
#include "stdafx.h"
#include "Material.h"
#include "Crack.h"
#include "TDataStructure.h"

#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <random>



constexpr unsigned int NeighberMax = 6;
constexpr unsigned int LayerMax = 15;
constexpr unsigned int SlicesMax = 16;
constexpr unsigned int StackPerThread = SlicesMax / ThreadMax;

constexpr unsigned int NodeMax = ((2 - 1) + (LayerMax - 1)) * (LayerMax - 1) / 2 * 6 + 1;

class NodeLocalDamage_t
{
	double MatrixDamagePercent;
	double LocalVf, LocalVm, LocalVa;
	double LocalMatrixLoadFactor, LocalFiberLoadFactor;

};

class Node_t
{
public:
	Node_t();

	unsigned int Slice;

	unsigned int Layer;
	unsigned int Order;
	unsigned int Index;

	Node_t* Neighbors[NeighberMax];
	unsigned int NeighborCount;

	double LoadFactor;
	double OffsetAngle;
	double OffsetDamageFactor;

	int64_t Strength;
	int64_t MaxStress;
	int64_t MinStress;
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
	int64_t* Capablity;
	Crack_t** Crack;
	SideCrack_t** SideCrack;

private:
	void HexaGenerateP();
	void HexasGenerate(unsigned int);
	void HexaGenerate(unsigned int);
	void GenerateLayer(unsigned int, unsigned int);
	void InitStrength(unsigned int,unsigned int);

	void AddNeighbor(Node_t**, unsigned int, unsigned int);

	static void Convert23(const unsigned int&, const unsigned int&, unsigned int&, unsigned int&);
	static unsigned int Convert31(const unsigned int&, const unsigned int&, const unsigned int&);
	static unsigned int Convert21(const unsigned int&, const unsigned int&);

	unsigned int Finish;
	std::mutex mtx;
	std::condition_variable cv;

};


const double Orient[6][2] = { 1.0,0.0,1.0 / 2.0,sqrt(3.0) / 2.0,-1.0 / 2.0,sqrt(3.0) / 2.0,-1.0,0.0,-1.0 / 2.0,-sqrt(3.0) / 2.0,1.0 / 2.0,-sqrt(3.0) / 2.0 };