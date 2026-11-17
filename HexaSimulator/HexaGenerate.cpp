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

	printf("[HEXA] before cv.wait Finish=%u\n", this->Finish); fflush(stdout);
	cv.wait(lock, [&]{ return this->Finish == ThreadMax; });
	printf("[HEXA] after cv.wait Finish=%u\n", this->Finish); fflush(stdout);

	//for (unsigned int s = 0; s < SlicesMax; s++)
		//printf("Hexa:Generated Slice 0 with %d Nodes!\n",  this->NodeCount[0]);
	return;
}
void Hexa_t::HexasGenerate(unsigned int s0)
{
	for (unsigned int s = 0; s < StackPerThread; s++)
	{
		printf("[HEXA] gen slice=%u\n", s0 + s); fflush(stdout);
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
	static unsigned int DbgCounter = 0;
	if ((++DbgCounter % 23) == 0) { printf("[HEXA] GenerateLayer slice=%u layer=%u cnt=%u\n", Slice, Layer, DbgCounter); fflush(stdout); }

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
		Node->theta = atan2(Node->y, Node->x);
		
		(NodeCount[Slice])++;
		return;
	}
	
	for (unsigned int Order = 1; Order <= 6 * (Layer-1); Order++)
	{
		Node_t*& NodeNow = Nodes[StartNode + Order - 1];
		NodeNow = new Node_t;
	}
	for (unsigned int Order = 1; Order <= 6 * (Layer - 1); Order++)
	{
		Node_t*& NodeNow = Nodes[StartNode + Order - 1];

		NodeNow->Slice = Slice;

		NodeNow->Layer = Layer;
		NodeNow->Order = Order;
		NodeNow->Index = Convert21(Layer, Order);

		unsigned int Edge, Node;
		Convert23(Layer, Order, Edge, Node);

		if (Node == 1)
		{
			AddNeighbor(Nodes, Convert31(Layer, Edge, Node), Convert31(Layer - 1, Edge, Node));
			AddNeighbor(Nodes, Convert21(Layer, Order), Convert21(Layer, Order - 1));// Node 1 try to neighbor Node 5 before it is generated. Neighbor Nodes after everyone generated
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
			NodeNow->theta = atan2(NodeNow->y, NodeNow->x);
			/*NodeNow->OffsetAngle = OffsetK * NodeNow->r / R0 + OffsetB;
			if (NodeNow->OffsetAngle < ThetaThresh) NodeNow->OffsetDamageFactor = 1;
			else
				NodeNow->OffsetDamageFactor = ((NodeNow->OffsetAngle - ThetaThresh) * OffsetFactor) / 100 + 1;

			if (NodeNow->r > SpecimenRadious- R1) NodeNow->OverBoarder = true;

			NodeNow->FiberHealth = FiberHealthMax;*/

			auto Overboarder = [&]{
				//if (Slice == 0) printf("Hexa:x=%.2lf,y=%.2lf,X0=%.2lf,YLayer=%.2lf,R1=%.2lf\n", NodeNow->x * E6, NodeNow->y * E6, X0 * E6, YLayer * E6, R1 * E6);
				if (!Rect) return (NodeNow->r > SpecimenRadious - R1);
				else return((NodeNow->x < -X0/2 + R1) || (NodeNow->x > X0/2 - R1) || (NodeNow->y < -YLower + R1) || (NodeNow->y > YUpper - R1));
				};


			if (Overboarder()) NodeNow->OverBoarder = true;
			else (NodeCount[Slice])++;
		}
	}

	//Set Position
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
		NodeNow->theta = atan2(NodeNow->y, NodeNow->x);
		/*NodeNow->OffsetAngle = OffsetK * NodeNow->r / R0 + OffsetB;
		if (NodeNow->OffsetAngle < ThetaThresh) NodeNow->OffsetDamageFactor = 1;
		else 
			NodeNow->OffsetDamageFactor = ((NodeNow->OffsetAngle - ThetaThresh) * OffsetFactor) / 100 + 1;
			*/
		auto Overboarder = [&] {
			if (!Rect) return (NodeNow->r > SpecimenRadious - R1);
			else return((NodeNow->x < -X0 / 2 + R1) || (NodeNow->x > X0 / 2 - R1) || (NodeNow->y < -YLower + R1) || (NodeNow->y > YUpper - R1));
			};
		if (Overboarder()) NodeNow->OverBoarder = true;
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
		Node->TwistAngle = OffsetB > 0 ? OffsetB : 0; // r=0 → θ=b
		Node->Strength = int64_t(FiberStrength(Seed));
		Node->Strength0 = Node->Strength;
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
		NodeNow->Strength0 = NodeNow->Strength;

		
		NodeNow->MinStress = 0;
		NodeNow->MaxStress = NodeNow->Strength;
		NodeNow->SlidingStrength = SlidingStrength0;
		// Twist angle: θ = OffsetK * r/d0 + OffsetB (thesis Eq 2-2)
		NodeNow->TwistAngle = OffsetK * NodeNow->r / R0 + OffsetB; // FIX 2026-10-08: abscissa is r/R0 (fibre RADIUS), not r/(2R0); the old form gave exactly half the measured off-axis angle
		if (NodeNow->TwistAngle < 0.0) NodeNow->TwistAngle = 0.0;
		if (!NodeNow->OverBoarder)
		{
			Capablity[Slice] += NodeNow->MaxStress;
		}
			
	}
	if (Slice == 0)
	{
		double Req = sqrtl(NodeCount[Slice] * R0 * R0 / Vf);
		if (CrackShape == CentralCrack)
			CrackManager->CreateCrack(Req, Slice);

		else if (CrackShape == RectSideCrack)
			CrackManager->CreateRectSideCrack(X0, Slice);

		else if (CrackShape == SCAlatering)
			CrackManager->CreateRectSideCrack(X0, Slice);

		else if(CrackShape == SideCrack)
			CrackManager->CreateSideCrack(Slice);
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
	int FilteredOrder = Order;
	if (FilteredOrder == 0)
		FilteredOrder = (Layer - 1) * 6;

	unsigned int Edge, Node;
	Convert23(Layer, FilteredOrder, Edge, Node);
	unsigned int Index = Convert31(Layer, Edge, Node);
	return(Index);
}
Node_t::Node_t()
{
	this->NeighborCount = 0;
	this->LoadFactor = 1;

	this->TwistAngle = 0.0;

	this->Broken = false;

	this->OverBoarder = false;
}
void Hexa_t::ParametersCopying(HexaInit_t Init)
{
	R0 = Init.R0;
	R1 = sqrt(pi / (2 * sqrt(3) * Init.Vf)) * R0;
	SpecimenRadious = Init.SpecimenRadious;
	Rect = Init.Rect;
	X0 = Init.X0;
	YLayer = Init.YLayer;
	Vf = Init.Vf;
	mf = Init.mf * Init.ScatterDecay;
	SigmaL = Init.Sigma0 * Init.StrengthDecay * pow(Init.L, -1.0 / mf) * pow(Init.Lg, 1.0 / mf);
	SlidingStrength0 = Init.SlidingStrength0;

	CrackShape = Init.CrackShape;
	DegTauN = Init.DegTauN; DegTauD = Init.DegTauD;
	DegSigN = Init.DegSigN; DegSigD = Init.DegSigD;
	DegT = Init.DegT;
	OffsetK = Init.OffsetK;
	OffsetB = Init.OffsetB;

	//printf("Hexa:FiberStrength:%.2lfMPa,Scatter:%.2lf\n", SigmaL * EM6, mf);
}
void Hexa_t::CrackManagerInitReady(HexaInit_t Init)
{
	CrackManagerInit = new CrackManagerInit_t;
	CrackManagerInit->ICLCentral = Init.ICLCentral;
	CrackManagerInit->ICLSide = Init.ICLSide;
	CrackManagerInit->ICLRect = Init.ICLRect;
	CrackManagerInit->SpecimenRadious = Init.SpecimenRadious;
	CrackManagerInit->Em = Init.Em;
	CrackManagerInit->Ef = Init.Ef;
	CrackManagerInit->Vm = Init.Vm;
	CrackManagerInit->Vf = Init.Vf;
	CrackManagerInit->SigmamMax = Init.SigmamMax;
	CrackManagerInit->KLow = Init.KLow;
	CrackManagerInit->KIC = Init.KIC;
	CrackManagerInit->Cm = new double[2];
	CrackManagerInit->Mm = new double[2];
	for (unsigned int i = 0; i < 2; i++)
	{
		CrackManagerInit->Cm[i] = Init.Cm[i];
		CrackManagerInit->Mm[i] = Init.Mm[i];
	}
	CrackManagerInit->OrientDistributionModel = Init.OrientDistributionModel;
	CrackManagerInit->SmallCrackCB = Init.SmallCrackCB;
}
Hexa_t::Hexa_t(HexaInit_t Init)
{
	ParametersCopying(Init);
	CrackManagerInitReady(Init);


	//printf("FiberStrength:%.2lfMPa\n", Init.R0);

	this->Nodes = new Node_t * *[SlicesMax];
	this->NodeCount = new unsigned int[SlicesMax];

	for (unsigned int s = 0; s < SlicesMax; s++)
	{
		Nodes[s] = new Node_t * [NodeMax + 1]; // FIX 2026-10-08: node indices are 1-based (Nodes[1] is the first), so NodeMax+1 slots are required; the old NodeMax-sized array overflowed by one element on every run (caught by ASan as heap-buffer-overflow in GenerateLayer)
		//std::cout << s << std::endl;
	}

	for (unsigned int s = 0; s < SlicesMax; s++)
		NodeCount[s] = 0;
	this->Finish = 0;


	this->Capablity = new int64_t[SlicesMax];
	for (unsigned int s = 0; s < SlicesMax; s++)
		Capablity[s] = 0;

	this->CrackManager = new CrackManager_t(*CrackManagerInit);

	this->YUpper = (2 * (YLayer / 2) + 1) * R1 + 10 * EM6; //2(Layer)R1 for real distance,+1R1 for program to -1,10*EM6->a liitle clearance
	this->YLower = (2 * (YLayer / 2 - 1 + YLayer % 2) + 1) * R1 + 10 * EM6;

	this->HexaGenerateP(); //RAII
	printf("DEBUG Hexa: LayerMax=%u, TotalFibers=%u, R1=%.1fum, Vf=%.2f, R0=%.0fum, SpecR=%.0fum\n",
		LayerMax, NodeCount[0], R1*1e6, Vf, R0*1e6, SpecimenRadious*1e6);
}
