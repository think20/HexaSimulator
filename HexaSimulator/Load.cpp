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
void TestPiece_t::FiberLoad(unsigned int Stack, unsigned int TotalStep)
{
	unsigned int LoadStep = TotalStep % LoadStepMax;
	LoadDistribute(Stack, LoadStep);
	LoadRedistribute(Stack);
	FiberDamage(Stack);
}
void TestPiece_t::LoadDistribute(unsigned int Stack,unsigned int LoadStep)
{
	auto& Nodes = this->Hexa->Nodes[Stack];
	for (auto Index = 1; Index < NodeMax; Index++)
	{
		auto& Node = Nodes[Index];

		if (Node->OverBoarder) continue;

		Node->Load = int64_t((LoadPercent[LoadStep]) * MaxLoad * FiberLoadFactor / 100.0); //Only for no yield!
	}
}
void TestPiece_t::LoadRedistribute(unsigned int Stack)
{
	auto& Nodes = this->Hexa->Nodes[Stack];

	for (auto Index = 1; Index < NodeMax; Index++)
	{
		auto& Node = Nodes[Index];

		if (Node->OverBoarder) continue;

		if (Node->Load > Node->MaxStress)
		{
			OverLoadNodes[Stack]->Push(Node);
			//printf("Debug Overload:(%d,%d),%d>%d\n", Stack, Index, Node->Load, Node->MaxStress);
		}
	}
	/*std::vector<int>::iterator iter;
	iter->a;*/
	while (!OverLoadNodes[Stack]->Empty())
	{
		auto* Node = OverLoadNodes[Stack]->Front();
		OverLoadNodes[Stack]->Pop();

		//printf("! %d:%d\n", Node->Stack, Node->Index);

		LLSNode_t StartNode;
		StartNode.Node = Node;
		StartNode.LLSIndex = 0;
		Visited[Stack][Node->Index] = Color[Stack]++;

		BFSSeq[Stack]->Push(StartNode);
		FindNeighbors(Stack);

		double LLSShare = 0;
		
		//printf("%d\n", AnsSeq[Stack]->Empty());

		for (auto LLSNeighborNode = AnsSeq[Stack]->Begin(); LLSNeighborNode != AnsSeq[Stack]->End(); LLSNeighborNode++)
		{
			//auto debugbg = AnsSeq[Stack]->Begin();
			//auto debuged = AnsSeq[Stack]->End();
			LLSShare += LLSFactor[LLSNeighborNode->LLSIndex];
			//printf("Stack:%d,Now:%d,End:%d,Size:%u\n", Stack, LLSNeighborNode.DebugGetIndex(), AnsSeq[Stack]->End().DebugGetIndex(),AnsSeq[Stack]->DebugGetSize());
		}
		//printf("yee\n");
		for (auto LLSNeighborNode=AnsSeq[Stack]->Begin(); LLSNeighborNode != AnsSeq[Stack]->End(); LLSNeighborNode++)
		{
			auto NeighborNode = LLSNeighborNode->Node;
			auto LLSI = LLSNeighborNode->LLSIndex;

			NeighborNode->Load += int64_t(double(Node->Load - Node->MaxStress) * (LLSFactor[LLSI] / LLSShare));

			if (NeighborNode->Load > NeighborNode->MaxStress)
			{
				OverLoadNodes[Stack]->Push(NeighborNode);
				//printf("Stack:%u Neighbor:%u overloaded by Node:%u\n", Stack, Node->Index, NeighborNode->Index);
			}
		}

		
		Node->Load = Node->MaxStress;
		AnsSeq[Stack]->Clear();
	}
}
void TestPiece_t::FiberDamage(unsigned int Stack)
{
	auto& Nodes = this->Hexa->Nodes[Stack];
	for (auto Index = 1; Index < NodeMax; Index++)
	{
		auto& Node = Nodes[Index];

		if (Node->OverBoarder) continue;
			
		if (Node->Load == Node->Strength && (!Node->Broken))
		{
			Node->Broken = true;
			ToUpdate[Stack]->Push(Node);
		}
		
	}

}
void TestPiece_t::UpdateMaxStress()
{
	for (unsigned int Stack = 0; Stack < StacksMax; Stack++)
	{
		while (!ToUpdate[Stack]->Empty())
		{
			auto Node = ToUpdate[Stack]->Front();
			ToUpdate[Stack]->Pop();

			auto& Index = Node->Index;
			int64_t StressMax = 0;
			for (int StackU = Stack - 1; StackU >= 0; StackU--)
			{
				auto& NodeU = this->Hexa->Nodes[StackU][Index];
				StressMax += unsigned int(2.0 * pi * R0 * L * double(NodeU->SlidingStrength));

				NodeU->MaxStress = std::min(NodeU->MaxStress, StressMax);
			}

			for (int StackL = Stack + 1; StackL < StacksMax; StackL++)
			{
				auto& NodeL = this->Hexa->Nodes[StackL][Index];
				StressMax += unsigned int(2.0 * pi * R0 * L * double(NodeL->SlidingStrength));

				NodeL->MaxStress = std::min(NodeL->MaxStress, StressMax);
			}
			DebugCounter[Stack]++;
		}
		//printf("%d:%d\n",Stack,DebugCounter[Stack]);
	}
	//printf("=================================\n");
}
void TestPiece_t::FindNeighbors(unsigned int Stack)
{
	auto Node = BFSSeq[Stack]->Front().Node;
	auto LLSIndex = BFSSeq[Stack]->Front().LLSIndex;
	BFSSeq[Stack]->Pop();

	if (LLSIndex == LLSIMax) return;

	for (unsigned int n = 0; n < Node->NeighborCount; n++)
	{
		auto const& Neighbor = Node->Neighbors[n];

		auto Discard = [&] {
			if (Neighbor->OverBoarder) return true;
			if (Neighbor->Load >= Neighbor->MaxStress) return true;
			if (Visited[Neighbor->Stack][Neighbor->Index] == Visited[Node->Stack][Node->Index]) return true;
			return false;

			};

		if (Discard()) continue;

		LLSNode_t LLSNeighborNode;
		LLSNeighborNode.Node = Neighbor;
		LLSNeighborNode.LLSIndex = LLSIndex + 1;

		BFSSeq[Stack]->Push(LLSNeighborNode);
		AnsSeq[Stack]->Push(LLSNeighborNode);

		Visited[Neighbor->Stack][Neighbor->Index] = Visited[Node->Stack][Node->Index];

	}
	
	if (!BFSSeq[Stack]->Empty()) FindNeighbors(Stack);

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