#include "stdafx.h"
#include "Hexa.h"

#include <thread>
#include <future>
#include <iostream>
#include <chrono>

void Hexa_t::HexaGenerateP()
{
	std::thread t[ThreadMax];
	std::unique_lock<std::mutex> lock(this->mtx);

	for (unsigned int i = 0; i < ThreadMax; i++)
	{
		unsigned int Stack0 = StackPerThread * i;
		t[i] = std::thread(&Hexa_t::HexasGenerate, this, Stack0);
		t[i].detach();
	}

	cv.wait(lock, [&]{ return this->Finish == ThreadMax; });

	for (unsigned int s = 0; s < SlicesMax; s++)
		printf("Generated Slice %d with %d Nodes!\n", s, this->NodeCount[s]);
	return;
}
void Hexa_t::HexasGenerate(unsigned int s0)
{
	for (unsigned int s = 0; s < StackPerThread; s++)
	{
		HexaGenerate(s0 + s);
		//printf("%d:%d\n", s, NodeCount[s]);
	}
	std::unique_lock<std::mutex> lock(mtx);
	this->Finish++;
	cv.notify_one();
}
void Hexa_t::HexaGenerate(unsigned int Slice)
{
	for (unsigned int Layer = 1; Layer <= LayerMax; Layer++)
	{
		GenerateLayer(Slice, Layer);
		InitStrength(Slice, Layer);
		//printf("Slice:%d,Layer:%d,NodeCount:%d,Cap:%lf,AvgCap:%lf\n", Slice, Layer, NodeCount[Slice], double(Capablity[Slice]), double(Capablity[Slice] / NodeCount[Slice]));
	}

}
void Hexa_t::GenerateLayer(unsigned int Slice, unsigned int Layer)
{
	//printf("Layer:%u\n", Layer);
	//auto TimeSeed = std::chrono::high_resolution_clock::now();
	//std::default_random_engine Seed(TimeSeed);

	auto Nodes = this->Nodes[Slice];
	unsigned int StartNode = Convert31(Layer, 1, 1);
	unsigned int EndNode = Convert31(Layer, 6, Layer - 1);
	//unsigned int NodeCount = EndNode - StartNode + 1;

	if (Layer == 1)
	{
		Nodes[1] = new Node_t;

		auto& Node = Nodes[1];

		Node->Slice = Slice;
		Node->Layer = 1;
		Node->Order = 1;
		Node->Index = Convert21(Node->Layer, Node->Order);
		Node->r = 0;
		Node->x = 0;
		Node->y = 0;
		
		(NodeCount[Slice])++;
		return;
	}
	
	for (unsigned int Order = 1; Order <= 6 * (Layer-1); Order++)
	{
		Node_t*& NodeNow = Nodes[StartNode + Order - 1];
		NodeNow = new Node_t;

		NodeNow->Slice = Slice;

		NodeNow->Layer = Layer;
		NodeNow->Order = Order;
		NodeNow->Index = Convert21(Layer, Order);

		unsigned int Edge, Node;
		Convert23(Layer, Order, Edge, Node);

		if (Node == 1)
		{
			AddNeighbor(Nodes, Convert31(Layer, Edge, Node), Convert31(Layer - 1, Edge, Node));
		}
		else
		{
			AddNeighbor(Nodes, Convert21(Layer, Order), Convert21(Layer, Order - 1));
			AddNeighbor(Nodes, Convert31(Layer, Edge, Node), Convert31(Layer - 1, Edge, Node - 1));
			AddNeighbor(Nodes, Convert31(Layer, Edge, Node), Convert31(Layer - 1, Edge, Node));
		}

	}
	for (unsigned int Order = 1; Order <= 6 * (Layer - 1); Order++)
	{
		Node_t*& NodeNow = Nodes[StartNode + Order - 1];
		unsigned int Edge, Node;
		Convert23(Layer, Order, Edge, Node);
		if (Node == 1)
		{

			NodeNow->r = 2.0 * (1.0 * Layer - 1.0) * R1;
			NodeNow->x = 2.0 * (1.0 * Layer - 1.0) * R1 * Orient[Edge - 1][0];
			NodeNow->y = 2.0 * (1.0 * Layer - 1.0) * R1 * Orient[Edge - 1][1];
			/*NodeNow->OffsetAngle = OffsetK * NodeNow->r / R0 + OffsetB;
			if (NodeNow->OffsetAngle < ThetaThresh) NodeNow->OffsetDamageFactor = 1;
			else
				NodeNow->OffsetDamageFactor = ((NodeNow->OffsetAngle - ThetaThresh) * OffsetFactor) / 100 + 1;

			if (NodeNow->r > SpecimenRadious- R1) NodeNow->OverBoarder = true;

			NodeNow->FiberHealth = FiberHealthMax;*/
			if (NodeNow->r > SpecimenRadious - R1) NodeNow->OverBoarder = true;
			else (NodeCount[Slice])++;
		}
	}

	for (unsigned int Order = 1; Order <= 6 * (Layer - 1); Order++)
	{
		Node_t*& NodeNow = Nodes[StartNode + Order - 1];

		unsigned int Edge, Node;
		Convert23(Layer, Order, Edge, Node);

		if (Node == 1) continue; //Last node is the first node of next edge

		auto& NodeStart = Nodes[Convert31(Layer, Edge, 1)];
		auto& NodeEnd = Nodes[Convert31(Layer, Edge % 6 + 1, 1)];
		double Ratio = double((Node - 1) / double(Layer - 1));

		NodeNow->x = Ratio * double(NodeEnd->x) + (1 - Ratio) * double(NodeStart->x);
		NodeNow->y = Ratio * double(NodeEnd->y) + (1 - Ratio) * double(NodeStart->y);
		NodeNow->r = sqrt(pow(NodeNow->x, 2) + pow(NodeNow->y, 2));
		/*NodeNow->OffsetAngle = OffsetK * NodeNow->r / R0 + OffsetB;
		if (NodeNow->OffsetAngle < ThetaThresh) NodeNow->OffsetDamageFactor = 1;
		else 
			NodeNow->OffsetDamageFactor = ((NodeNow->OffsetAngle - ThetaThresh) * OffsetFactor) / 100 + 1;
			*/
		if (NodeNow->r > SpecimenRadious - R1) NodeNow->OverBoarder = true;
		else (NodeCount[Slice])++;
	}
}

void Hexa_t::InitStrength(unsigned int Slice,unsigned int Layer)
{
	std::random_device Seed;
	std::weibull_distribution<double> FiberStrength(mf, SigmaL);

	auto Nodes = this->Nodes[Slice];

	if (Layer == 1)
	{
		auto& Node = Nodes[1];
		Node->OffsetAngle = 0;
		Node->OffsetDamageFactor = 0;
		Node->Strength = int64_t(FiberStrength(Seed));
		Node->MinStress = 0;
		Node->MaxStress = Node->Strength;
		Node->SlidingStrength = SlidingStrength0;

		Capablity[Slice] += Node->MaxStress;

		return;
	}
	unsigned int StartNode = Convert31(Layer, 1, 1);
	unsigned int EndNode = Convert31(Layer, 6, Layer - 1);

	for (unsigned int Order = 1; Order <= 6 * (Layer - 1); Order++)
	{
		Node_t*& NodeNow = Nodes[StartNode + Order - 1];

		unsigned int Edge, Node;
		Convert23(Layer, Order, Edge, Node);

		NodeNow->Strength = int64_t(FiberStrength(Seed));
		//printf("Strength:%u\n", NodeNow->Strength/1000'000);
		NodeNow->MinStress = 0;
		NodeNow->MaxStress = NodeNow->Strength;
		NodeNow->SlidingStrength = SlidingStrength0;
		if (!NodeNow->OverBoarder)
			Capablity[Slice] += NodeNow->MaxStress;
	}
	if (Slice == 0)
	{
		double Req = sqrtl(NodeCount[Slice] * R0 * R0 / Vf);
#ifdef CrackModel
		CrackManager->CreateCrack(Req, Slice);
#endif
#ifdef SideCrackModel
		CrackManager->CreateSideCrack(Slice);
#endif
	}
		
}

void Hexa_t::AddNeighbor(Node_t** Nodes, unsigned int Index1, unsigned int Index2)
{
	//printf("%d,%d Neighboring\n", Index1, Index2);

	Node_t*& Node1 = Nodes[Index1];
	Node_t*& Node2 = Nodes[Index2];

	Node1->Neighbors[Node1->NeighborCount++] = Node2;
	Node2->Neighbors[Node2->NeighborCount++] = Node1;

}
void Hexa_t::Convert23(const unsigned int& Layer, const unsigned int& Order, unsigned int& Edge, unsigned int& Node)
{
	Edge = (Order - 1) / (Layer - 1) + 1;
	Node = (Order - 1) % (Layer - 1) + 1;
}
unsigned int Hexa_t::Convert31(const unsigned int& Layer, const unsigned int& Edge, const unsigned int& Node)
{
	if (Layer == 1) return(1);
	unsigned int Index = ((2 - 1) + (Layer - 2)) * (Layer - 2) / 2 * 6 + 2 + (Edge - 1) * (Layer - 1) + (Node - 1);
	return(Index);
}
unsigned int Hexa_t::Convert21(const unsigned int& Layer, const unsigned int& Order)
{
	if (Layer == 1) return(1);
	unsigned int Edge, Node;
	Convert23(Layer, Order, Edge, Node);
	unsigned int Index = Convert31(Layer, Edge, Node);
	return(Index);
}
Node_t::Node_t()
{
	this->NeighborCount = 0;
	this->LoadFactor = 1;

	this->Broken = false;

	this->OverBoarder = false;
}
Hexa_t::Hexa_t()
{
	this->Nodes = new Node_t * *[SlicesMax];
	this->NodeCount = new unsigned int[SlicesMax];

	for (unsigned int s = 0; s < SlicesMax; s++)
	{
		Nodes[s] = new Node_t * [NodeMax];
		//std::cout << s << std::endl;
	}

	for (unsigned int s = 0; s < SlicesMax; s++)
		NodeCount[s] = 0;
	this->Finish = 0;


	this->Capablity = new int64_t[SlicesMax];
	for (unsigned int s = 0; s < SlicesMax; s++)
		Capablity[s] = 0;

	this->CrackManager = new CrackManager_t();

	this->HexaGenerateP(); //RAII
}
