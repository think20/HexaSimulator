#include "stdafx.h"
#include "Load.h"
#include "Crack.h"

#include <cmath>
#include <ctime>
#include <cstdlib>
#include <random>
#include <queue>

FailureReport_t TestPiece_t::FiberLoadP(unsigned int CycleMax)
{
	auto TotalStepMax = CycleMax * LoadStepMax;
	unsigned int LoadStep = 0;
	unsigned int LoadCycle = 0;

	std::thread th[ThreadMax];
	for (auto i = 0; i < ThreadMax; i++)
	{
		th[i] = std::thread(&TestPiece_t::FibersLoad, this, i, TotalStepMax);
		th[i].detach();
	}
	while (this->SyncTotalStep < TotalStepMax)
	{
		auto Cycle = SyncTotalStep %(100* LoadStepMax);
		//if (Cycle == 0) printf("TestPiece:Cycle%d\n", SyncTotalStep/LoadStepMax);

		LoadStep = SyncTotalStep % LoadStepMax;
		LoadCycle = SyncTotalStep / LoadStepMax;

		std::unique_lock<std::mutex> lock(this->mtx);

		//printf("Syncing TotalStep %u\n", this->SyncTotalStep);
		this->UpdateMaxStress();
		//this->SyncLoadDistibute(LoadStep, LoadCycle);
		//printf("Sync Finish\n");

		cvm.wait_for(lock,std::chrono::seconds(1), [&] {return this->Finish == ThreadMax; });
		

		//printf("TestPiece:SyncTotalStep %u\n", this->SyncTotalStep);

		for (auto t = 0; t < ThreadMax; t++)
		{
			if (FailureReport[t] != nullptr)
			{
				HasResult = true;
				Result = FailureReport[t];
				break;
			}
		}
		
		if (HasResult) break;

		this->SyncTotalStep++;
		this->Finish = 0;
		//printf("%d\n", SyncTotalStep);

		cv.notify_all();

		//now unique_lock destructs, mtx unlocked
	}

	if (HasResult) return(*Result);
	else return(FailureReport_t(0, MaxCycle, 0, this));
}
void TestPiece_t::SyncLoadDistibute(unsigned int LoadStep, unsigned int LoadCycle)
{
	for (unsigned int s = 0; s < SlicesMax - 1; s++)
	{
		LoadDistribute(Slice[s], LoadStep, LoadCycle);
		SetFiberMinimumStress(Slice[s], Slice[s + 1]);
	}

	LoadDistribute(Slice[SlicesMax-1], LoadStep, LoadCycle);
}
void TestPiece_t::FibersLoad(unsigned int thread, unsigned int TotalStepMax)
{
	auto s0 = thread * StackPerThread;
	try 
	{
		for (unsigned int TotalStep = 1; TotalStep < TotalStepMax; TotalStep++)
		{
			//printf("thread %u Start Tstep%u\n", thread, TotalStep);
			for (auto s = 0; s < StackPerThread; s++)
			{
				FiberLoad(Slice[s + s0], TotalStep);
			}
			std::unique_lock<std::mutex> lock(this->mtx);
			this->Finish++;
			//printf("thread %u Finished TStep %u,Waiting for sync\n", thread, TotalStep);
			cvm.notify_one();

			cv.wait(lock, [&] {return TotalStep == this->SyncTotalStep - 1; });
		}
	}
	//All other threads will be stuck waiting for sync
	catch (FailureReport_t& Report)
	{
		//this.failurereport = report
		//printf("%d,%d\n", Report.Slice, Report.Cycle);

		//Force Deep Copying.Report destructs exiting this function.
		FailureReport[thread] = new FailureReport_t(Report);

		//printf("%d,%d\n", FailureReport[thread]->Slice, FailureReport[thread]->Cycle);

		std::unique_lock<std::mutex> lock(this->mtx);
		this->Finish++;
		cvm.notify_one();

	}

}
void TestPiece_t::FiberLoad(Slice_t* Slice, unsigned int TotalStep)
{
	unsigned int LoadStep = TotalStep % LoadStepMax;
	unsigned int LoadCycle = TotalStep / LoadStepMax;

	//Otherwise Distributed in sync

	LoadDistribute(Slice, LoadStep, LoadCycle); 
	LoadRedistribute(Slice);
	FiberDamage(Slice);
	return;
}

void TestPiece_t::LoadDistribute(Slice_t* Slice,unsigned int LoadStep,unsigned int LoadCycle)
{
	auto& Crack = Slice->Crack;
	auto& RectSideCrack = Slice->RectSideCrack;
	auto& SideCrack = Slice->SideCrack;
	if (Crack != nullptr)  CrackPlaneLoadDistribute(Slice, LoadStep, LoadCycle);
	else if (RectSideCrack != nullptr) RectSideCrackPlaneLoadDistribute(Slice, LoadStep, LoadCycle);
	else if (SideCrack != nullptr) SideCrackPlaneLoadDistribute(Slice, LoadStep, LoadCycle);
	else NonCrackLoadDistribute(Slice, LoadStep, LoadCycle);

}	
void TestPiece_t::LoadRedistribute(Slice_t* Slice)
{
	
	for (auto Node= Slice->Nodes->Begin();Node!=Slice->Nodes->End();Node++)
	{
		
		if ((*Node)->Load > (*Node)->MaxStress)
		{
			Slice->OverLoadNodes->Push(*Node);
			//printf("Debug Overload:(%d,%d),%d>%d\n", Slice, Index, Node->Load, Node->MaxStress);
		}
	}
	/*std::vector<int>::iterator iter;
	iter->a;*/
	while (!Slice->OverLoadNodes->Empty())
	{
		auto* Node = Slice->OverLoadNodes->Front();
		Slice->OverLoadNodes->Pop();

		//printf("! %d:%d\n", Node->Slice, Node->Index);

		LLSNode_t StartNode;
		StartNode.Node = Node;
		StartNode.LLSIndex = 0;
		Slice->Visited[Node->Index] = Slice->Color++;

		Slice->BFSSeq->Push(StartNode);
		FindNeighbors(Slice);

		double LLSShare = 0;
		
		//printf("%d\n", AnsSeq[Slice]->Empty());

		for (auto LLSNeighborNode = Slice->AnsSeq->Begin(); LLSNeighborNode != Slice->AnsSeq->End(); LLSNeighborNode++)
		{
			//auto debugbg = AnsSeq[Slice]->Begin();
			//auto debuged = AnsSeq[Slice]->End();
			LLSShare += LLSFactor[LLSNeighborNode->LLSIndex];
			//printf("Slice:%d,Now:%d,End:%d,Size:%u\n", Slice, LLSNeighborNode.DebugGetIndex(), AnsSeq[Slice]->End().DebugGetIndex(),AnsSeq[Slice]->DebugGetSize());
		}
		//printf("yee\n");
		for (auto LLSNeighborNode=Slice->AnsSeq->Begin(); LLSNeighborNode != Slice->AnsSeq->End(); LLSNeighborNode++)
		{
			auto NeighborNode = LLSNeighborNode->Node;
			auto LLSI = LLSNeighborNode->LLSIndex;

			NeighborNode->Load += int64_t(double(Node->Load - Node->MaxStress) * (LLSFactor[LLSI] / LLSShare));

			if (NeighborNode->Load > NeighborNode->MaxStress)
			{
				Slice->OverLoadNodes->Push(NeighborNode);
				//printf("Slice:%u Neighbor:%u overloaded by Node:%u\n", Slice, Node->Index, NeighborNode->Index);
			}
		}

		Node->Load = Node->MaxStress;
		Slice->AnsSeq->Clear();
	}
}
void TestPiece_t::FiberDamage(Slice_t* Slice)
{
	for (auto Node = Slice->Nodes->Begin(); Node != Slice->Nodes->End(); Node++)
	{
		if ((*Node)->Load == (*Node)->Strength && (!(*Node)->Broken))
		{
			(*Node)->Broken = true;
			Slice->ToUpdate->Push(*Node);
		}
	}
}
void TestPiece_t::UpdateMaxStress()
{
	for (unsigned int s = 0; s < SlicesMax; s++)
	{
		while (!Slice[s]->ToUpdate->Empty())
		{
			auto Node = Slice[s]->ToUpdate->Front();
			Slice[s]->ToUpdate->Pop();

			auto& Index = Node->Index;
			int64_t StressMax = 0;

			//Here vertical iteration is used,so nodes are updated directly by hexa instead of Slice
			for (int StackU = s - 1; StackU >= 0; StackU--)
			{
				auto& NodeU = this->Hexa->Nodes[StackU][Index];
				StressMax += int64_t(2.0 * pi * R0 * L * double(NodeU->SlidingStrength) / (pi * R0 * R0));

				this->Hexa->Capablity[StackU] -= NodeU->MaxStress;
				NodeU->MaxStress = std::min(NodeU->MaxStress, StressMax);
				this->Hexa->Capablity[StackU] += NodeU->MaxStress;
			}

			for (int StackL = s + 1; StackL < SlicesMax; StackL++)
			{
				auto& NodeL = this->Hexa->Nodes[StackL][Index];
				StressMax += int64_t(2.0 * pi * R0 * L * double(NodeL->SlidingStrength) / (pi * R0 * R0));

				this->Hexa->Capablity[StackL] -= NodeL->MaxStress;
				NodeL->MaxStress = std::min(NodeL->MaxStress, StressMax);
				this->Hexa->Capablity[StackL] += NodeL->MaxStress;
			}
			DebugCounter[s]++;
		}
		//printf("Slice:%d Broken:%d\n",Slice,DebugCounter[Slice]);
	}
	//printf("=================================\n");
}
void TestPiece_t::CrackPlaneLoadDistribute(Slice_t* Slice, unsigned int LoadStep, unsigned int LoadCycle)
{
	double SliceFiberLoadFactor = FiberLoadFactor; //default
	int64_t CompositeLoad = int64_t((LoadPercent[LoadStep]) * MaxLoad / 100.0);

	auto& Crack = Slice->Crack;

	double CrackLen = Crack->UpdateCrack(double(CompositeLoad));
	
	//printf("%.2lf\n", CrackLen);
	if (CrackLen > Crack->MaxCrackLen)
	{
		//printf("Meow!\n");
		throw FailureReport_t(Slice->Slice, LoadCycle, CrackLen, this);
		/*printf("Crack too long at Slice %d at Cycle %d with CrackLen %.2lfmm\n", Slice->Slice, LoadCycle, CrackLen * E3);
		for (unsigned int s = 0; s < SlicesMax; s++)
			printf("Slice:%u,Broken:%u\n", s, DebugCounter[s]);
		printf("Debug Matrix LoadFactor:%.2lf,Fiber LoadFactor:%.2lf\n", Crack->Yb, SliceFiberLoadFactor);
		for (auto Node = Slice->Nodes->Begin(); Node != Slice->Nodes->End(); Node++)
		{
			if ((*Node)->Broken)
			{
				printf("%d,%d\n", int((*Node)->x * E6), int((*Node)->y * E6));
			}
		}
		system("pause");*/
	}
	if (Crack->Yield)
	{
		double CompositeForce = double(CompositeLoad) * pi * SpecimenRadious * SpecimenRadious;
		double MatrixForce = Vm * pi * (SpecimenRadious * SpecimenRadious - CrackLen * CrackLen) * SigmamMax;
		double FiberForce = CompositeForce - MatrixForce;
		double FiberLoad = FiberForce / (Vf * pi * SpecimenRadious * SpecimenRadious);
		SliceFiberLoadFactor = FiberLoad / CompositeLoad;
	}
	else SliceFiberLoadFactor == Crack->Yb * Ef / Em;

	SliceFiberOverload(Slice, LoadStep, LoadCycle, SliceFiberLoadFactor);

	double BaseLine = FindFiberLoadBaseline(Slice);
	SliceFiberLoadFactor *= BaseLine;
	int64_t Load = int64_t(CompositeLoad * SliceFiberLoadFactor);

	double Len = Crack->Len();

	for (auto Node = Slice->Nodes->Begin(); Node != Slice->Nodes->End(); Node++)
	{

		auto& r = (*Node)->r;
		double Factor = 1;//default, Fiber not in Crack

		if (r < Len)
			Factor = FiberOverloadFactor * sqrtl((Len - r));

		(*Node)->Load = int64_t(Load * Factor);

		//Load = k*u^1/2
	}

}
void TestPiece_t::RectSideCrackPlaneLoadDistribute(Slice_t* Slice, unsigned int LoadStep, unsigned int LoadCycle)
{
	double SliceFiberLoadFactor = FiberLoadFactor; //default
	int64_t CompositeLoad = int64_t((LoadPercent[LoadStep]) * MaxLoad / 100.0);

	auto& RectSideCrack = Slice->RectSideCrack;

	double CrackLen = RectSideCrack->UpdateCrack(double(CompositeLoad));


	if (CrackLen > RectSideCrack->MaxCrackLen)
	{
		throw FailureReport_t(Slice->Slice, LoadCycle, CrackLen, this);
	}
	if (RectSideCrack->Yield)
	{
		double CompositeForce = double(CompositeLoad) * pi * SpecimenRadious * SpecimenRadious;
		double MatrixForce = Vm * pi * (SpecimenRadious * SpecimenRadious - CrackLen * CrackLen) * SigmamMax;
		double FiberForce = CompositeForce - MatrixForce;
		double FiberLoad = FiberForce / (Vf * pi * SpecimenRadious * SpecimenRadious);
		SliceFiberLoadFactor = FiberLoad / CompositeLoad;
	}
	else SliceFiberLoadFactor == RectSideCrack->Yb * Ef / Em;

	SliceFiberOverload(Slice, LoadStep, LoadCycle, SliceFiberLoadFactor);

	double BaseLine = FindFiberLoadBaseline(Slice);
	SliceFiberLoadFactor *= BaseLine;
	int64_t Load = int64_t(CompositeLoad * SliceFiberLoadFactor);

	double Len = RectSideCrack->Len();

	for (auto Node = Slice->Nodes->Begin(); Node != Slice->Nodes->End(); Node++)
	{

		auto& x = (*Node)->x;
		double Factor = 1;//default, Fiber not in RectSideCrack

		if (x < Len - X0 / 2)
			Factor = 1 + FiberOverloadFactor * sqrtl((Len - X0 / 2 - x));

		(*Node)->Load = int64_t(Load * Factor);

		//Load = k*u^1/2
	}
}
void TestPiece_t::SideCrackPlaneLoadDistribute(Slice_t* Slice, unsigned int LoadStep, unsigned int LoadCycle)
{
	double SliceFiberLoadFactor = FiberLoadFactor; //default
	double SliceMatrixLoadFactor = MatrixLoadFactor;
	int64_t CompositeLoad = int64_t((LoadPercent[LoadStep]) * MaxLoad / 100.0);

	auto& SideCrack = Slice->SideCrack;

	double CrackLen = SideCrack->UpdateCrack(double(CompositeLoad) * MatrixLoadFactor); // Far field load, not local
	if (CrackLen > SideCrack->MaxCrackLen)
	{
		//printf("TestPiece:Fail at Slice %u!\n", Slice->Slice);
		for (auto s = 0; s < SlicesMax; s++)
		{
			//if (this->Slice[s]->SideCrack != nullptr)
				//printf("TestPiece: Slice %u SideCrackLen: %.2lf um\n", s, this->Slice[s]->SideCrack->Len()*E6);
		}
		throw FailureReport_t(Slice->Slice, LoadCycle, CrackLen, this);
		/*printf("Crack to long at Slice %d at Cycle %d with CrackLen %.2lfmm\n", Slice->Slice, LoadCycle, CrackLen * E3);
		for (unsigned int s = 0; s < SlicesMax; s++)
			printf("Slice:%u,Broken:%u\n", s, DebugCounter[s]);
		//printf("Debug Matrix LoadFactor:%.2lf,Fiber LoadFactor:%.2lf\n", SideCrack->Yb, SliceFiberLoadFactor);

		for (auto Node = Slice->Nodes->Begin(); Node != Slice->Nodes->End(); Node++)
		{
			if ((*Node)->Broken)
			{
				printf("%d,%d\n", int((*Node)->x * E6), int((*Node)->y * E6));
			}
		}
		system("pause");*/
	}

	double FullCompositeArea = pi * SpecimenRadious * SpecimenRadious;
	double MatrixArea = (FullCompositeArea - SideCrack->S) * Vm;
	double FiberArea = Vf * FullCompositeArea;
	double SliceVf = FiberArea / (FiberArea + MatrixArea);
	double SliceVm = 1 - SliceVf;

	SliceFiberLoadFactor = Ef / (Ef * SliceVf + Em * SliceVm);
	SliceMatrixLoadFactor = Em / (Ef * SliceVf + Em * SliceVm);

	double SliceMatrixLoad = SliceMatrixLoadFactor * CompositeLoad;
	if (SliceMatrixLoad > SigmamMax)
	{
		double CompositeForce = double(CompositeLoad) * pi * SpecimenRadious * SpecimenRadious;
		double MatrixForce = MatrixArea * SigmamMax;
		double FiberForce = CompositeForce - MatrixForce;
		double FiberLoad = FiberForce / FiberArea;
		SliceFiberLoadFactor = FiberLoad / CompositeLoad;
	}

	SliceFiberOverload(Slice, LoadStep, LoadCycle, SliceFiberLoadFactor);

	double BaseLine = FindFiberLoadBaseline(Slice);
	SliceFiberLoadFactor *= BaseLine;
	int64_t Load = int64_t(CompositeLoad * SliceFiberLoadFactor);

	for (auto Node = Slice->Nodes->Begin(); Node != Slice->Nodes->End(); Node++)
	{

		auto& R = (*Node)->r;
		auto& Theta = (*Node)->theta;
		double Factor = 1;//default, Fiber not in Crack

		double Dis = Slice->SideCrack->DistanceToCrackFront(R, Theta);
		if (Dis < 0)
			Factor = 1 + FiberOverloadFactor * sqrtl((-Dis));

		(*Node)->Load = int64_t(Load * Factor);

		//Load = k*u^1/2
	}

}
void TestPiece_t::NonCrackLoadDistribute(Slice_t* Slice, unsigned int LoadStep, unsigned int LoadCycle)
{
	double SliceFiberLoadFactor = FiberLoadFactor; //default
	int64_t CompositeLoad = int64_t((LoadPercent[LoadStep]) * MaxLoad / 100.0);

	if (double(CompositeLoad) * MatrixLoadFactor > SigmamMax)
	{
		double CompositeForce = double(CompositeLoad) * pi * SpecimenRadious * SpecimenRadious;
		double MatrixForce = Vm * pi * SpecimenRadious * SpecimenRadious * SigmamMax;
		double FiberForce = CompositeForce - MatrixForce;
		double FiberLoad = FiberForce / (Vf * pi * SpecimenRadious * SpecimenRadious);
		SliceFiberLoadFactor = FiberLoad / CompositeLoad;

		double Req = sqrtl(Slice->NodeCount * R0 * R0 / Vf);
		
		if (CrackShape == CentralCrack)
			Slice->Crack = Hexa->CrackManager->CreateCrack(Req, Slice->Slice);

		else if (CrackShape == RectSideCrack)
			Slice->RectSideCrack = Hexa->CrackManager->CreateRectSideCrack(X0, Slice->Slice);

		else if (CrackShape == SCAlatering)
		{
			if (Slice->Slice % 2 == 0)
				Slice->SideCrack = Hexa->CrackManager->CreateSideCrack(Slice->Slice);
			else
				Slice->Crack = Hexa->CrackManager->CreateCrack(Req, Slice->Slice);
		}


		else if(CrackShape==SideCrack)
			Slice->SideCrack = Hexa->CrackManager->CreateSideCrack(Slice->Slice);
			
	}

	SliceFiberOverload(Slice, LoadStep, LoadCycle, SliceFiberLoadFactor);

	int64_t Load = int64_t((LoadPercent[LoadStep]) * MaxLoad * SliceFiberLoadFactor / 100.0);
	for (auto Node = Slice->Nodes->Begin(); Node != Slice->Nodes->End(); Node++)
	{
		(*Node)->Load = Load;

		if ((*Node)->Load < (*Node)->MinStress)
		{
			//printf("TestPiece:Node:%d LoadUpscale due to neighbor Slice,from %lld to %lld,Cap %lld at Step:%d,Cyc:%d\n", (*Node)->Index, (*Node)->Load * EM6, (*Node)->MinStress * EM6, (*Node)->Strength * EM6, LoadStep, LoadCycle);
			(*Node)->Load = (*Node)->MinStress;
		}
			
	}
}

void TestPiece_t::SliceFiberOverload(Slice_t* Slice, unsigned int LoadStep, unsigned int LoadCycle, double SliceFiberLoadFactor)
{
	int64_t Load = int64_t((LoadPercent[LoadStep]) * MaxLoad * SliceFiberLoadFactor / 100.0);
	int64_t TotalLoad = Load * Slice->NodeCount;
	int64_t CompositeLoad = int64_t((LoadPercent[LoadStep]) * MaxLoad / 100.0);

	double LoadRatio = double(TotalLoad) / double(Slice->Capablity);

	/*if (LoadStep == 11)
		printf("Slice %d:NodeLoad:%.2lf,AvgCap:%.2lf,Ratio:%.2lf,Nodes:%d\n", Slice, double(Load)*EM6, double(this->Hexa->Capablity[Slice])/double(this->Hexa->NodeCount[Slice]) * EM6, LoadRatio,this->Hexa->NodeCount[Slice]);
		*/
	if (LoadRatio > MaxLoadRatio)
	{
		/*printf("Overload at Slice %d at Cycle %d Step %d with AvgCap%.2lf,Load%.2lf,ExtLoad%.2lfMPa\n", Slice->Slice, LoadCycle, LoadStep, double(Slice->Capablity / Slice->NodeCount) * EM6, double(Load) * EM6, double(CompositeLoad) * EM6);
		for (unsigned int s = 0; s < SlicesMax; s++)
			printf("Slice:%u,Broken:%u\n", s, DebugCounter[s]);
		system("pause");*/

		double CrackLen = 0;
		if (Slice->Crack != nullptr) CrackLen = Slice->Crack->Len();
		if (Slice->RectSideCrack != nullptr) CrackLen = Slice->RectSideCrack->Len();
		if (Slice->SideCrack != nullptr) CrackLen = Slice->SideCrack->Len();

		throw FailureReport_t(Slice->Slice, LoadCycle, CrackLen, this);
	}
}
double TestPiece_t::FindFiberLoadBaseline(Slice_t* Slice)
{
	if (Slice->Crack != nullptr) return FindFiberLoadBaselineWithCrack(Slice);
	else if (Slice->RectSideCrack != nullptr) return FindFiberLoadBaselineWithRectSideCrack(Slice);
	else if (Slice->SideCrack != nullptr) return FindFiberLoadBaselineWithSideCrack(Slice);
}
void TestPiece_t::SetFiberMinimumStress(Slice_t* LoadedSlice, Slice_t* NotLoadedSlice)
{
	auto LoadedNode = LoadedSlice->Nodes->Begin();
	auto NotLoadedNode = NotLoadedSlice->Nodes->Begin();

	while (LoadedNode != LoadedSlice->Nodes->End())
	{
		int64_t MaxLoadDrop = int64_t(2.0 * pi * R0 * L * double((*LoadedNode)->SlidingStrength) / (pi * R0 * R0));
		int64_t MinLoad = (*LoadedNode)->Load - MaxLoadDrop;
		if (MinLoad > 0) (*NotLoadedNode)->MinStress = MinLoad;
		else (*NotLoadedNode)->MinStress = 0;

		LoadedNode++;
	}
}
void TestPiece_t::FindNeighbors(Slice_t* Slice)
{
	auto Node = Slice->BFSSeq->Front().Node;
	auto LLSIndex = Slice->BFSSeq->Front().LLSIndex;
	Slice->BFSSeq->Pop();

	if (LLSIndex == LLSIMax) return;

	for (unsigned int n = 0; n < Node->NeighborCount; n++)
	{
		auto const& Neighbor = Node->Neighbors[n];

		auto Discard = [&] {
			if (Neighbor->OverBoarder) return true;
			if (Neighbor->Load >= Neighbor->MaxStress) return true;
			if (Slice->Visited[Neighbor->Index] == Slice->Visited[Node->Index]) return true;
			return false;

			};

		if (Discard()) continue;

		LLSNode_t LLSNeighborNode;
		LLSNeighborNode.Node = Neighbor;
		LLSNeighborNode.LLSIndex = LLSIndex + 1;

		Slice->BFSSeq->Push(LLSNeighborNode);
		Slice->AnsSeq->Push(LLSNeighborNode);

		Slice->Visited[Neighbor->Index] = Slice->Visited[Node->Index];

	}
	
	if (!Slice->BFSSeq->Empty()) FindNeighbors(Slice);

}

double TestPiece_t::FindFiberLoadBaselineWithCrack(Slice_t* Slice)
{
	auto& Crack = Slice->Crack;
	double Len = Crack->Len();


	double TotalFactor = 0;
	for (auto Node = Slice->Nodes->Begin(); Node != Slice->Nodes->End(); Node++)
	{

		auto& r = (*Node)->r;

		double Factor = 1;//default, Fiber not in Crack

		if (r < Len)
			Factor = 1 + FiberOverloadFactor * sqrtl((Len - r));

		TotalFactor += Factor;
	}
	double BaseLine = double(Slice->NodeCount) / TotalFactor;
	return BaseLine;
}
double TestPiece_t::FindFiberLoadBaselineWithRectSideCrack(Slice_t* Slice)
{
	auto& RectSideCrack = Slice->RectSideCrack;
	double Len = RectSideCrack->Len();


	double TotalFactor = 0;
	for (auto Node = Slice->Nodes->Begin(); Node != Slice->Nodes->End(); Node++)
	{

		auto& x = (*Node)->x;

		double Factor = 1;//default, Fiber not in RectSideCrack

		if (x < Len - X0 / 2)
			Factor = 1 + FiberOverloadFactor * sqrtl((Len - X0 / 2 - x));

		TotalFactor += Factor;
	}
	double BaseLine = double(Slice->NodeCount) / TotalFactor;
	return BaseLine;
}
double TestPiece_t::FindFiberLoadBaselineWithSideCrack(Slice_t* Slice)
{
	auto& SideCrack = Slice->SideCrack;

	double TotalFactor = 0;
	for (auto Node = Slice->Nodes->Begin(); Node != Slice->Nodes->End(); Node++)
	{

		auto& R = (*Node)->r;
		auto& Theta = (*Node)->theta;

		double Factor = 1;//default, Fiber not in Crack

		double Dis = Slice->SideCrack->DistanceToCrackFront(R, Theta);
		if (Dis < 0)
			Factor = 1 + FiberOverloadFactor * sqrtl((-Dis));

		TotalFactor += Factor;
	}
	double BaseLine = double(Slice->NodeCount) / TotalFactor;
	return BaseLine;
}
FailureReport_t::FailureReport_t(unsigned int Slice, unsigned int Cycle, double CrackLen, TestPiece_t* TestPiece)
{
	this->Slice = Slice;
	this->Cycle = Cycle;
	this->CrackLen = CrackLen;

	Fractures = new TQueue_t<PlanePos_t*>* [SlicesMax];
	for (unsigned int s = 0; s < SlicesMax; s++)
	{
		Fractures[s] = new TQueue_t<PlanePos_t*>(NodeMax);
	}

	for (unsigned int s = 0; s < SlicesMax; s++)
	{
		auto& Nodes = TestPiece->Slice[s]->Nodes;
		for (auto Node = Nodes->Begin(); Node != Nodes->End(); Node++)
		{
			if ((*Node)->Broken)
			{
				auto NewPlanePos = new PlanePos_t((*Node)->x / (2 * TestPiece->R0), (*Node)->y / (2 * TestPiece->R0));
				Fractures[s]->Push(NewPlanePos);
				//printf("FailureReport:Broken!\n");
			}
		}
	}
}
FailureReport_t::FailureReport_t(FailureReport_t& From)
{
	this->Slice = From.Slice;
	this->Cycle = From.Cycle;
	this->CrackLen = From.CrackLen;

	Fractures = new TQueue_t<PlanePos_t*>*[SlicesMax];
	for (unsigned int s = 0; s < SlicesMax; s++)
		Fractures[s] = new TQueue_t<PlanePos_t*>(NodeMax);

	for (unsigned int s = 0; s < SlicesMax; s++)
	{
		while (!From.Fractures[s]->Empty())
		{
			auto NewPlanePos = new PlanePos_t(*From.Fractures[s]->Front());
			Fractures[s]->Push(NewPlanePos);
			From.Fractures[s]->Pop();
		}
	}
}
FailureReport_t::~FailureReport_t()
{
	// LET IT LEAK
	return;
}