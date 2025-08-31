#include "stdafx.h"
#include "Load.h"

#include <cmath>
#include <ctime>
#include <cstdlib>
#include <random>
#include <queue>
void TestPiece_t::FiberLoadP(unsigned int CycleMax)
{
	auto TotalStepMax = CycleMax * LoadStepMax;

	std::thread th[ThreadMax];
	for (auto i = 0; i < ThreadMax; i++)
	{
		th[i] = std::thread(&TestPiece_t::FibersLoad, this, i, TotalStepMax);
		th[i].detach();
	}
	while (this->SyncTotalStep < TotalStepMax)
	{
		auto Cycle = SyncTotalStep %(100* LoadStepMax);
		if (Cycle == 0) printf("%d\n", SyncTotalStep/LoadStepMax);

		std::unique_lock<std::mutex> lock(this->mtx);

		//printf("Syncing TotalStep %u\n", this->SyncTotalStep);
		this->UpdateMaxStress();
		//printf("Sync Finish\n");

		cvm.wait(lock, [&] {return this->Finish == ThreadMax; });

		this->SyncTotalStep++;
		this->Finish = 0;
		//printf("%d\n", SyncTotalStep);

		cv.notify_all();
	}

}
void TestPiece_t::FibersLoad(unsigned int thread, unsigned int TotalStepMax)
{
	auto s0 = thread * StackPerThread;
	for (unsigned int TotalStep = 1; TotalStep < TotalStepMax; TotalStep++)
	{
		//printf("thread %u Start Tstep%u\n", thread, TotalStep);
		for (auto s = 0; s < StackPerThread; s++)
		{
			FiberLoad(s + s0, TotalStep);		
		}
		std::unique_lock<std::mutex> lock(this->mtx);
		this->Finish++;
		//printf("thread %u Finished TStep %u,Waiting for sync\n", thread, TotalStep);
		cvm.notify_one();	

		cv.wait(lock, [&] {return TotalStep == this->SyncTotalStep - 1; });
	}

}
void TestPiece_t::FiberLoad(unsigned int Slice, unsigned int TotalStep)
{
	unsigned int LoadStep = TotalStep % LoadStepMax;
	LoadDistribute(Slice, LoadStep);
	LoadRedistribute(Slice);
	FiberDamage(Slice);
	return;
}
void TestPiece_t::LoadDistribute(unsigned int Slice,unsigned int LoadStep)
{
	int64_t Load = int64_t((LoadPercent[LoadStep]) * MaxLoad * FiberLoadFactor / 100.0);
	uint64_t TotalLoad = Load * this->Hexa->NodeCount[Slice];

	double LoadRatio = double(TotalLoad) / double(this->Hexa->Capablity[Slice]);

	/*if (LoadStep == 11)
		printf("Slice %d:NodeLoad:%.2lf,AvgCap:%.2lf,Ratio:%.2lf,Nodes:%d\n", Slice, double(Load)*EM6, double(this->Hexa->Capablity[Slice])/double(this->Hexa->NodeCount[Slice]) * EM6, LoadRatio,this->Hexa->NodeCount[Slice]);
		*/
	if (LoadRatio > MaxLoadRatio)
	{
		printf("Overload at Slice %d at Step %d with AvgCap%.2lf,Load%.2lf\n", Slice,LoadStep,double(TotalLoad / this->Hexa->NodeCount[Slice]), double(Load));
		system("pause");
	}

	auto& Nodes = this->Hexa->Nodes[Slice];
	for (auto Index = 1; Index < NodeMax; Index++)
	{
		auto& Node = Nodes[Index];

		if (Node->OverBoarder) continue;

		Node->Load = Load; //Only for no yield!
	}
}
void TestPiece_t::LoadRedistribute(unsigned int Slice)
{
	auto& Nodes = this->Hexa->Nodes[Slice];

	for (auto Index = 1; Index < NodeMax; Index++)
	{
		auto& Node = Nodes[Index];

		if (Node->OverBoarder) continue;

		if (Node->Load > Node->MaxStress)
		{
			OverLoadNodes[Slice]->Push(Node);
			//printf("Debug Overload:(%d,%d),%d>%d\n", Slice, Index, Node->Load, Node->MaxStress);
		}
	}
	/*std::vector<int>::iterator iter;
	iter->a;*/
	while (!OverLoadNodes[Slice]->Empty())
	{
		auto* Node = OverLoadNodes[Slice]->Front();
		OverLoadNodes[Slice]->Pop();

		//printf("! %d:%d\n", Node->Slice, Node->Index);

		LLSNode_t StartNode;
		StartNode.Node = Node;
		StartNode.LLSIndex = 0;
		Visited[Slice][Node->Index] = Color[Slice]++;

		BFSSeq[Slice]->Push(StartNode);
		FindNeighbors(Slice);

		double LLSShare = 0;
		
		//printf("%d\n", AnsSeq[Slice]->Empty());

		for (auto LLSNeighborNode = AnsSeq[Slice]->Begin(); LLSNeighborNode != AnsSeq[Slice]->End(); LLSNeighborNode++)
		{
			//auto debugbg = AnsSeq[Slice]->Begin();
			//auto debuged = AnsSeq[Slice]->End();
			LLSShare += LLSFactor[LLSNeighborNode->LLSIndex];
			//printf("Slice:%d,Now:%d,End:%d,Size:%u\n", Slice, LLSNeighborNode.DebugGetIndex(), AnsSeq[Slice]->End().DebugGetIndex(),AnsSeq[Slice]->DebugGetSize());
		}
		//printf("yee\n");
		for (auto LLSNeighborNode=AnsSeq[Slice]->Begin(); LLSNeighborNode != AnsSeq[Slice]->End(); LLSNeighborNode++)
		{
			auto NeighborNode = LLSNeighborNode->Node;
			auto LLSI = LLSNeighborNode->LLSIndex;

			NeighborNode->Load += int64_t(double(Node->Load - Node->MaxStress) * (LLSFactor[LLSI] / LLSShare));

			if (NeighborNode->Load > NeighborNode->MaxStress)
			{
				OverLoadNodes[Slice]->Push(NeighborNode);
				//printf("Slice:%u Neighbor:%u overloaded by Node:%u\n", Slice, Node->Index, NeighborNode->Index);
			}
		}

		
		Node->Load = Node->MaxStress;
		AnsSeq[Slice]->Clear();
	}
}
void TestPiece_t::FiberDamage(unsigned int Slice)
{
	auto& Nodes = this->Hexa->Nodes[Slice];
	for (auto Index = 1; Index < NodeMax; Index++)
	{
		auto& Node = Nodes[Index];

		if (Node->OverBoarder) continue;
			
		if (Node->Load == Node->Strength && (!Node->Broken))
		{
			Node->Broken = true;
			ToUpdate[Slice]->Push(Node);
		}
		
	}

}
void TestPiece_t::UpdateMaxStress()
{
	for (unsigned int Slice = 0; Slice < StacksMax; Slice++)
	{
		while (!ToUpdate[Slice]->Empty())
		{
			auto Node = ToUpdate[Slice]->Front();
			ToUpdate[Slice]->Pop();

			auto& Index = Node->Index;
			int64_t StressMax = 0;
			for (int StackU = Slice - 1; StackU >= 0; StackU--)
			{
				auto& NodeU = this->Hexa->Nodes[StackU][Index];
				StressMax += unsigned int(2.0 * pi * R0 * L * double(NodeU->SlidingStrength));

				this->Hexa->Capablity[StackU] -= NodeU->MaxStress;
				NodeU->MaxStress = std::min(NodeU->MaxStress, StressMax);
				this->Hexa->Capablity[StackU] += NodeU->MaxStress;
			}

			for (int StackL = Slice + 1; StackL < StacksMax; StackL++)
			{
				auto& NodeL = this->Hexa->Nodes[StackL][Index];
				StressMax += unsigned int(2.0 * pi * R0 * L * double(NodeL->SlidingStrength));

				this->Hexa->Capablity[StackL] -= NodeL->MaxStress;
				NodeL->MaxStress = std::min(NodeL->MaxStress, StressMax);
				this->Hexa->Capablity[StackL] += NodeL->MaxStress;
			}
			DebugCounter[Slice]++;
		}
		//printf("Slice:%d Broken:%d\n",Slice,DebugCounter[Slice]);
	}
	//printf("=================================\n");
}
void TestPiece_t::FindNeighbors(unsigned int Slice)
{
	auto Node = BFSSeq[Slice]->Front().Node;
	auto LLSIndex = BFSSeq[Slice]->Front().LLSIndex;
	BFSSeq[Slice]->Pop();

	if (LLSIndex == LLSIMax) return;

	for (unsigned int n = 0; n < Node->NeighborCount; n++)
	{
		auto const& Neighbor = Node->Neighbors[n];

		auto Discard = [&] {
			if (Neighbor->OverBoarder) return true;
			if (Neighbor->Load >= Neighbor->MaxStress) return true;
			if (Visited[Neighbor->Slice][Neighbor->Index] == Visited[Node->Slice][Node->Index]) return true;
			return false;

			};

		if (Discard()) continue;

		LLSNode_t LLSNeighborNode;
		LLSNeighborNode.Node = Neighbor;
		LLSNeighborNode.LLSIndex = LLSIndex + 1;

		BFSSeq[Slice]->Push(LLSNeighborNode);
		AnsSeq[Slice]->Push(LLSNeighborNode);

		Visited[Neighbor->Slice][Neighbor->Index] = Visited[Node->Slice][Node->Index];

	}
	
	if (!BFSSeq[Slice]->Empty()) FindNeighbors(Slice);

}
TestPiece_t::TestPiece_t()
{
	this->Hexa =  new Hexa_t;

	this->Visited = new uint64_t* [StacksMax];
	for (auto s = 0; s < StacksMax; s++)
	{
		this->Visited[s] = new uint64_t [NodeMax];
		for (auto n = 0; n < NodeMax; n++)
			this->Visited[s][n] = 0;
	}

	this->Color = new uint64_t[StacksMax];

	for (auto s = 0; s < StacksMax; s++)
		Color[s] = 0;
	
	this->Ready = true;
	this->SyncTotalStep = 1;

	this->OverLoadNodes = new TQueue_t<Node_t*>* [StacksMax];
	this->BFSSeq = new TQueue_t<LLSNode_t>* [StacksMax];
	this->AnsSeq = new TVector_t<LLSNode_t>* [StacksMax];
	this->ToUpdate = new TQueue_t<Node_t*>* [StacksMax];

	for (unsigned int s = 0; s < StacksMax; s++)
	{
		this->OverLoadNodes[s] = new TQueue_t<Node_t*>(NodeMax);
		this->BFSSeq[s] = new TQueue_t<LLSNode_t>(NodeMax);
		this->AnsSeq[s] = new TVector_t<LLSNode_t>(NodeMax);
		this->ToUpdate[s] = new TQueue_t<Node_t*>(NodeMax);
	}

	this->DebugCounter = new unsigned int[StacksMax];
	for (unsigned int s = 0; s < StacksMax; s++)
		DebugCounter[s] = 0;



	this->Finish = 0;
}