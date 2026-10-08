#include "stdafx.h"
#include "Load.h"
#include "Crack.h"

#include <cmath>
#include <ctime>
#include <cstdlib>
#include <random>
#include <queue>

unsigned int DebugHasLLSAns = 0;
unsigned int DebugTarget = 10;
unsigned int DebugHasMaxStressUpdate = 0;

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
		//if (Cycle == 0) 
			//printf("TestPiece:Cycle%d\n", SyncTotalStep/LoadStepMax);

		LoadStep = SyncTotalStep % LoadStepMax;
		LoadCycle = SyncTotalStep / LoadStepMax;

		std::unique_lock<std::mutex> lock(this->mtx);

		//printf("Syncing TotalStep %u\n", this->SyncTotalStep);
		this->UpdateMaxStress();
		if (LoadCycle % 100 == 0) this->UpdateDegradation(LoadCycle);
		//this->SyncLoadDistibute(LoadStep, LoadCycle);
		//printf("Sync Finish\n");

		cvm.wait_for(lock,std::chrono::seconds(1), [&] {return this->Finish == ThreadMax; });
		

		for (auto t = 0; t < ThreadMax; t++)
		{
			if (FailureReport[t] != nullptr)
			{
				HasResult = true;
				Result = FailureReport[t];
				break;
			}
		}
		
		if (HasResult)
		{
			break;
		}

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
	bool DebugOutput = false;
	/*if (Slice->Slice == 0 && LoadStep == 13 && LoadCycle == 3000)
		DebugOutput = true;*/

	//Otherwise Distributed in sync
	LoadDistribute(Slice, LoadStep, LoadCycle); 
	LoadRedistribute(Slice, LoadStep, LoadCycle);
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
void TestPiece_t::LoadRedistribute(Slice_t* Slice, unsigned int LoadStep, unsigned int LoadCycle)
{
	//Reset Overload status
	for (auto Node = Slice->Nodes->Begin(); Node != Slice->Nodes->End(); Node++)
		(*Node)->Overload = false;
	//Reset Father
	for (auto i = 0; i < Slice->NodeCount; i++)
		Slice->Father[i] = i;

	if (LoadSharingModel == 1) //GLS
	{
		while (GLSFiberOverload(Slice, LoadStep, LoadCycle) == true);
	}
	else //LLS default
	{
		while (LLSFiberOverload(Slice, LoadStep, LoadCycle) == true);
	}
}
void TestPiece_t::UpdateDegradation(unsigned int Cycle)
{
	if (DegTauN <= 0 && DegSigN <= 0) return; //no degradation configured

	double TauFactor = 1.0, SigFactor = 1.0;
	double Arrhenius = 1.0;

	if (DegT > 0) //temperature acceleration
	{
		constexpr double QoverR = 5000.0; //K
		constexpr double Tref = 298.0;
		Arrhenius = exp(QoverR * (1.0 / Tref - 1.0 / DegT));
	}

	if (DegTauN > 0 && DegTauD > 0)
		TauFactor = 1.0 - DegTauD * Arrhenius * (1.0 - exp(-double(Cycle) / DegTauN));
	if (DegSigN > 0 && DegSigD > 0)
		SigFactor = 1.0 - DegSigD * (1.0 - exp(-double(Cycle) / DegSigN));

	if (TauFactor < 0.01) TauFactor = 0.01;
	if (SigFactor < 0.01) SigFactor = 0.01;

	for (unsigned int s = 0; s < SlicesMax; s++)
	{
		int64_t OldCap = this->Hexa->Capablity[s];
		for (auto Node = Slice[s]->Nodes->Begin(); Node != Slice[s]->Nodes->End(); Node++)
		{
			if ((*Node)->Broken) continue;

			if (DegTauN > 0)
				(*Node)->SlidingStrength = int64_t(double(SlidingStrength0) * TauFactor);

			if (DegSigN > 0)
			{
				int64_t NewStrength = int64_t(double((*Node)->Strength) * SigFactor / 1.0);
				if (NewStrength < (*Node)->Strength)
				{
					(*Node)->Strength = NewStrength;
					if ((*Node)->MaxStress > NewStrength)
						(*Node)->MaxStress = NewStrength;
				}
			}
		}
		//Capablity must be recalculated
		int64_t NewCap = 0;
		for (auto Node = Slice[s]->Nodes->Begin(); Node != Slice[s]->Nodes->End(); Node++)
		{
			if (!(*Node)->Broken) NewCap += (*Node)->MaxStress;
		}
		this->Hexa->Capablity[s] = NewCap;
	}
}
void TestPiece_t::FiberDamage(Slice_t* Slice)
{
	for (auto Node = Slice->Nodes->Begin(); Node != Slice->Nodes->End(); Node++)
	{
		if ((*Node)->Load >= (*Node)->Strength && (!(*Node)->Broken))
		{
			(*Node)->Broken = true;
			Slice->ToUpdate->Push(*Node);
		}
	}
}
void TestPiece_t::UpdateMaxStress()
{
	//printf("TestPiece:Updating MaxStress\n");
	for (unsigned int s = 0; s < SlicesMax; s++)
	{
		while (!Slice[s]->ToUpdate->Empty())
		{
			//printf("TestPiece:Slice %u not empty\n", s);
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

				if (DebugHasMaxStressUpdate == 10)
				{
					//printf("%d,%lld,%lld,%lld\n", StackU, NodeU->Strength, NodeU->MaxStress, StressMax);
				}
			}
			StressMax = 0;


			for (int StackL = s + 1; StackL < SlicesMax; StackL++)
			{
				auto& NodeL = this->Hexa->Nodes[StackL][Index];
				StressMax += int64_t(2.0 * pi * R0 * L * double(NodeL->SlidingStrength) / (pi * R0 * R0));

				this->Hexa->Capablity[StackL] -= NodeL->MaxStress;
				NodeL->MaxStress = std::min(NodeL->MaxStress, StressMax);
				this->Hexa->Capablity[StackL] += NodeL->MaxStress;

			}
			Node->MaxStress = 0;//Now broken

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

	if (CrackLen > Crack->MaxCrackLen)
	{
		printf("FAIL|Crack_MaxCrackLen|Sl=%u|Cy=%u|St=%u|Len=%.1f|Max=%.1f\n", Slice->Slice, LoadCycle, LoadStep, CrackLen*E6, Crack->MaxCrackLen*E6);
		throw FailureReport_t(Slice->Slice, LoadCycle, CrackLen, this);
	}
	if (Crack->Yield)
	{
		double CompositeForce = double(CompositeLoad) * pi * SpecimenRadious * SpecimenRadious;
		double MatrixForce = Vm * pi * (SpecimenRadious * SpecimenRadious - CrackLen * CrackLen) * SigmamMax;
		double FiberForce = CompositeForce - MatrixForce;
		double FiberLoad = FiberForce / (Vf * pi * SpecimenRadious * SpecimenRadious);
		SliceFiberLoadFactor = FiberLoad / CompositeLoad; //Matrix Yield
		/*if (LoadCycle % 100 == 0 && Slice->Slice == 0 && LoadStep == 13)
		{
			printf("Yield!\n");
		}*/
	}
	else SliceFiberLoadFactor = Crack->Yb * Ef / Em; //No Matrix Yield

	/*if (LoadCycle % 100 == 0 && Slice->Slice == 0 && LoadStep == 13)
	{
		double MatrixStress;
		MatrixStress = double(CompositeLoad) * MatrixLoadFactor * Crack->Yb;
		if (Crack->Yield)
			MatrixStress = SigmamMax;
		double MatrixForceAll = Vm * pi * (SpecimenRadious * SpecimenRadious - CrackLen * CrackLen) * MatrixStress;
		double FiberLoad = double(CompositeLoad) * SliceFiberLoadFactor;
		double FiberForceAll = FiberLoad * (Vf * pi * SpecimenRadious * SpecimenRadious);
		double CompositeForceAll = double(CompositeLoad) * pi * SpecimenRadious * SpecimenRadious;
		printf("%.2lf,%.2lf,%.2lf,%.2lf,%.2lf\n", CompositeForceAll*EM3,(MatrixForceAll+FiberForceAll)*EM3, MatrixForceAll * EM3, FiberForceAll * EM3, MatrixStress * EM6);
	}*/

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
		printf("FAIL|RectSideCrack_MaxCrackLen|Sl=%u|Cy=%u|St=%u|Len=%.1f|Max=%.1f\n", Slice->Slice, LoadCycle, LoadStep, CrackLen*E6, RectSideCrack->MaxCrackLen*E6);
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
	else SliceFiberLoadFactor = RectSideCrack->Yb * Ef / Em;

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
	int64_t CompositeLoad = int64_t((LoadPercent[LoadStep]) * MaxLoad / 100.0);
	auto& SideCrack = Slice->SideCrack;

	// Crack growth
	double CrackLen = SideCrack->UpdateCrack(double(CompositeLoad) * MatrixLoadFactor);
	if (CrackLen > SideCrack->MaxCrackLen)
	{
		printf("FAIL|SideCrack_MaxCrackLen|Sl=%u|Cy=%u|St=%u|Len=%.1f|Max=%.1f\n", Slice->Slice, LoadCycle, LoadStep, CrackLen*E6, SideCrack->MaxCrackLen*E6);
		throw FailureReport_t(Slice->Slice, LoadCycle, CrackLen, this);
	}

	// Characteristic crack length c0 (Curtin 2001, thesis Eq 9: 100-200um)
	double c0 = WakeC0 * EM6;

	// Intact-zone fiber stress: mixing rule (elastic, matrix intact)
	double sigma_f_intact = double(CompositeLoad) * Ef / (Ef * Vf + Em * Vm);

	// Wake-zone fiber stress: matrix fully cracked, load carried by fibers only
	// Per Thouless (10a): T = const * sqrt(u), u ~ crack opening displacement
	// At deep wake (Dis << -c0): u ~ u_max, fiber carries sigma_comp / Vf
	double sigma_f_wake = double(CompositeLoad) / Vf;

	// Matrix yield check: if sigma_f_intact exceeds the yield-equivalent level,
	// matrix yields, transferring more load to fibers everywhere
	double sigma_m_intact = double(CompositeLoad) * Em / (Ef * Vf + Em * Vm);
	if (sigma_m_intact > SigmamMax)
	{
		double CompositeForce = double(CompositeLoad) * pi * SpecimenRadious * SpecimenRadious;
		double FullMatrixArea = Vm * pi * SpecimenRadious * SpecimenRadious;
		double FullFiberArea = Vf * pi * SpecimenRadious * SpecimenRadious;
		double MatrixForce = FullMatrixArea * SigmamMax;
		double FiberForce = CompositeForce - MatrixForce;
		sigma_f_intact = FiberForce / FullFiberArea;
		// Wake: matrix carries zero in cracked zone; ensure sigma_f_wake >= sigma_f_intact
		if (sigma_f_intact > sigma_f_wake) sigma_f_wake = sigma_f_intact;
	}

	// Cap sigma_f_wake to physical maximum (fiber strength order)
	if (sigma_f_wake > sigma_f_intact * 4.0)
		sigma_f_wake = sigma_f_intact * 4.0;

	double tip_zone = TipZone * EM6; // convert um to m

	for (auto Node = Slice->Nodes->Begin(); Node != Slice->Nodes->End(); Node++)
	{
		double Dis = Slice->SideCrack->DistanceToCrackFront((*Node)->r, (*Node)->theta);

		if (Dis < -c0)
		{
			// Deep wake: matrix fully cracked, fiber takes full load
			(*Node)->Load = int64_t(sigma_f_wake);
		}
		else if (Dis < 0.0)
		{
			// Transition zone: crack opening grows from 0 to u_max
			// Fiber stress interpolates via sqrt(|Dis|/c0) (Thouless 10a: T ~ sqrt(u))
			double ratio = sqrtl(-Dis / c0);
			(*Node)->Load = int64_t(sigma_f_intact + (sigma_f_wake - sigma_f_intact) * ratio);
		}
		else
		{
			// Intact zone: standard mixing rule
			(*Node)->Load = int64_t(sigma_f_intact);
		}

		// Fiber twist: axial load projection = cos(theta)
		// 3D FEM (19-fiber RVE, 2026): fiber axial stress follows cos(theta) exactly
		// Old linear model (1.5%/deg) deprecated — likely degrees/radians confusion
		// → only apply in INTACT zone (Dis > 0), not in wake
		// twist correction model (2026-10-08):
		//   0 = legacy cos(theta)   - 3D RVE with a TILTED LOAD and axial fibres
		//   1 = cos^3.3(theta)      - new 19-fibre PBC off-axis cell, vf=0.45 (=F2)
		constexpr int    TwistModel = 0;
		constexpr double TwistExp   = 3.3;

		double TwistAngle = (*Node)->TwistAngle;
		if (TwistAngle > 0.0 && Dis > 0.0)
		{
			double TwistRad = TwistAngle * 3.14159265358979323846 / 180.0;
			double TwistFactor = (TwistModel == 0) ? cos(TwistRad) : pow(cos(TwistRad), TwistExp);
			if (TwistFactor < 0.5) TwistFactor = 0.5; // safety floor
			(*Node)->Load = int64_t(double((*Node)->Load) * TwistFactor);
		}

		// Crack-tip SCF: amplifies fiber stress in the crack WAKE near the front
		// Wake fibers near crack front experience highest bridging stress
		if (TipSCF > 1.0 && tip_zone > 0.0 && Dis < 0.0 && Dis > -tip_zone)
		{
			double decay = 1.0 - Dis / tip_zone; // 1 at crack front, 0 at zone edge
			double scf = 1.0 + (TipSCF - 1.0) * decay;
			(*Node)->Load = int64_t(double((*Node)->Load) * scf);
		}
	}

	// Compute actual fiber capacity ratio from total fiber load / total fiber capacity
	int64_t TotalFiberLoad = 0;
	for (auto Node = Slice->Nodes->Begin(); Node != Slice->Nodes->End(); Node++)
		TotalFiberLoad += (*Node)->Load;
	double LoadRatio = double(TotalFiberLoad) / double(Slice->Capablity);
	if (LoadRatio > MaxLoadRatio)
	{
		double CrackLen = Slice->SideCrack->Len();
		printf("FAIL|SideCrack_LoadRatio|Sl=%u|Cy=%u|St=%u|Ratio=%.4f|MaxLR=%.2f|Cap=%lld|Load=%lld|CrackLen=%.1f\n", Slice->Slice, LoadCycle, LoadStep, LoadRatio, MaxLoadRatio, Slice->Capablity, TotalFiberLoad, CrackLen*E6);
		throw FailureReport_t(Slice->Slice, LoadCycle, CrackLen, this);
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
bool TestPiece_t::GLSFiberOverload(Slice_t* Slice, unsigned int LoadStep, unsigned int LoadCycle)
{
	int64_t TotalExcess = 0;
	unsigned int CanAcceptCount = 0;

	// Pass 1: find overloaded fibers, cap them, sum excess
	for (auto Node = Slice->Nodes->Begin(); Node != Slice->Nodes->End(); Node++)
	{
		if ((*Node)->Broken) continue;
		if ((*Node)->Load > (*Node)->MaxStress)
		{
			TotalExcess += (*Node)->Load - (*Node)->MaxStress;
			(*Node)->Load = (*Node)->MaxStress;
		}
		if ((*Node)->Load < (*Node)->MaxStress)
			CanAcceptCount++;
	}

	if (TotalExcess == 0) return false;
	if (CanAcceptCount == 0) return false;

	// Pass 2: distribute equally (GLS: entire slice is one pool)
	int64_t BaseShare = TotalExcess / CanAcceptCount;
	int64_t Remainder = TotalExcess % CanAcceptCount;
	unsigned int ExtraGiven = 0;

	for (auto Node = Slice->Nodes->Begin(); Node != Slice->Nodes->End(); Node++)
	{
		if ((*Node)->Broken) continue;
		if ((*Node)->Load < (*Node)->MaxStress)
		{
			(*Node)->Load += BaseShare;
			if (ExtraGiven < Remainder)
			{
				(*Node)->Load += 1;
				ExtraGiven++;
			}
		}
	}

	return true; // may have created new overloads, caller loops
}
bool TestPiece_t::LLSFiberOverload(Slice_t* Slice, unsigned int LoadStep, unsigned int LoadCycle)
{
	bool HasOverloadNodes = false;
	//Find All Overload Nodes
	Slice->OverLoadNodes->Clear();
	for (auto Node = Slice->Nodes->Begin(); Node != Slice->Nodes->End(); Node++)
	{
		if ((*Node)->Load >= (*Node)->MaxStress)
		{
			Slice->OverLoadNodes->Push(*Node);//Overload != broken,might also be sliding
			(*Node)->Overload = true;
			//printf("Cyc:%u,Stp:%u,Slice:%u,SI:%d\n",LoadCycle,LoadStep,Slice->Slice, (*Node)->SIndex);

			if ((*Node)->Load > (*Node)->MaxStress)
				HasOverloadNodes = true;
		}
	}
	if (HasOverloadNodes == false) return false;

	//Disjoint All Overload Nodes
	for (auto Node = Slice->OverLoadNodes->Begin(); Node != Slice->OverLoadNodes->End(); Node++)
	{
		for (auto n = 0; n < (*Node)->NeighborCount; n++)
		{
			auto& Neighbor = (*Node)->Neighbors[n];
			if (Neighbor->Overload)
			{
				auto Ancestor = Slice->FindAncestor((*Node)->SIndex);
				auto NAncestor = Slice->FindAncestor(Neighbor->SIndex);

				Slice->Father[NAncestor] = Ancestor;
			}
		}
	}

	//Reset Load to transfer
	for (auto n = 0; n < Slice->NodeCount; n++)
	{
		Slice->LoadtoTransfer[n] = 0;
	}

	//Add up load to transfer
	for (auto Node = Slice->OverLoadNodes->Begin(); Node != Slice->OverLoadNodes->End(); Node++)
	{
		auto Overload = (*Node)->Load - (*Node)->MaxStress;
		auto Ancestor = Slice->FindAncestor((*Node)->SIndex);
		Slice->LoadtoTransfer[Ancestor] += Overload;
		(*Node)->Load = (*Node)->MaxStress;

		//printf("Cyc:%u,Stp:%u,Slice:%u,Load to transfer:%lld\n", LoadCycle, LoadStep, Slice->Slice, Overload);
	}

	//Transfer Load
	while (Slice->OverLoadNodes->Empty() == false)
	{
		auto size = Slice->OverLoadNodes->DebugGetSize();
		auto Bench = Slice->FindAncestor(Slice->OverLoadNodes->Front()->SIndex);

		//Add a set of connected nodes to BFSSeq for topology sorting
		for (auto OIndex = 0; OIndex < size; OIndex++)
		{
			auto Node = Slice->OverLoadNodes->Front();
			Slice->OverLoadNodes->Pop();

			//re-push those with different color
			if (Slice->FindAncestor(Node->SIndex) != Bench)
				Slice->OverLoadNodes->Push(Node);

			else
			{
				LLSNode_t StartNode;
				StartNode.Node = Node;
				StartNode.LLSIndex = 0;
				Slice->Visited[Node->Index] = Slice->Color;

				Slice->BFSSeq->Push(StartNode);
			}
		}
		FindNeighbors(Slice, LoadStep, LoadCycle);

		Slice->BFSSeq->Clear();
		Slice->Color++;

		// Matrix plasticity absorbs (1-LLSEff) of the excess load
		Slice->LoadtoTransfer[Bench] = uint64_t(double(Slice->LoadtoTransfer[Bench]) * LLSEff);
		if (Slice->LoadtoTransfer[Bench] == 0) { Slice->AnsSeq->Clear(); continue; }

		double LLSShare = 0;
		for (auto LLSNeighborNode = Slice->AnsSeq->Begin(); LLSNeighborNode != Slice->AnsSeq->End(); LLSNeighborNode++)
			LLSShare += LLSFactor[LLSNeighborNode->LLSIndex];

		if (LLSShare <= 0.0) { Slice->AnsSeq->Clear(); continue; } //no neighbors can accept

		for (auto LLSNeighborNode = Slice->AnsSeq->Begin(); LLSNeighborNode != Slice->AnsSeq->End(); LLSNeighborNode++)
		{
			auto NeighborNode = LLSNeighborNode->Node;
			auto LLSI = LLSNeighborNode->LLSIndex;
			NeighborNode->Load += int64_t(double(Slice->LoadtoTransfer[Bench]) * (LLSFactor[LLSI] / LLSShare));
		}
		Slice->AnsSeq->Clear();
	}
	return true;
}
void TestPiece_t::SliceFiberOverload(Slice_t* Slice, unsigned int LoadStep, unsigned int LoadCycle, double SliceFiberLoadFactor)
{
	int64_t Load = int64_t((LoadPercent[LoadStep]) * MaxLoad * SliceFiberLoadFactor / 100.0);
	int64_t TotalLoad = Load * Slice->NodeCount;
	//int64_t CompositeLoad = int64_t((LoadPercent[LoadStep]) * MaxLoad / 100.0);

	double LoadRatio = double(TotalLoad) / double(Slice->Capablity);
	//Fiber Area is canceled out

	/*if (LoadStep == 11)
		printf("Slice %d:NodeLoad:%.2lf,AvgCap:%.2lf,Ratio:%.2lf,Nodes:%d\n", Slice, double(Load)*EM6, double(this->Hexa->Capablity[Slice])/double(this->Hexa->NodeCount[Slice]) * EM6, LoadRatio,this->Hexa->NodeCount[Slice]);
		*/
	//if (LoadCycle == 0 && LoadStep == 1)
		//printf("Cyc:%u Slice:%u LoadRatio:%.3lf Load:%lld TotalLoad:%lld Cap:%lld\n",LoadCycle,Slice->Slice, LoadRatio,Load,TotalLoad,Slice->Capablity);

	if (LoadRatio > MaxLoadRatio)
	{
		printf("FAIL|SliceFiberOverload|Sl=%u|Cy=%u|St=%u|Ratio=%.4f|MaxLR=%.2f|Cap=%lld|TotalLoad=%lld|NodeCount=%u\n", Slice->Slice, LoadCycle, LoadStep, LoadRatio, MaxLoadRatio, Slice->Capablity, TotalLoad, Slice->NodeCount);

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
void TestPiece_t::FindNeighbors(Slice_t* Slice, unsigned int LoadStep, unsigned int LoadCycle)
{
	auto Node = Slice->BFSSeq->Front().Node;
	auto LLSIndex = Slice->BFSSeq->Front().LLSIndex;
	Slice->BFSSeq->Pop();

	//if (LoadCycle == 1900 && Slice->Slice == 0 && LoadStep == 13)
	
		//printf("Searching for Node %d with Index %d\n", Node->Index, LLSIndex);
	

	if (LLSIndex == LLSIMax) return;

	for (unsigned int n = 0; n < Node->NeighborCount; n++)
	{
		auto const& Neighbor = Node->Neighbors[n];

		if (Slice->Visited[Neighbor->Index] == Slice->Visited[Node->Index])
			continue;
		
		LLSNode_t LLSNeighborNode;
		LLSNeighborNode.Node = Neighbor;
		LLSNeighborNode.LLSIndex = LLSIndex + 1;

		Slice->BFSSeq->Push(LLSNeighborNode);
		auto Discard = [&] {
			if (Neighbor->OverBoarder)
			{
				return true;
			}
			if (Neighbor->Overload)
			{
				return true;
			}
			return false;
			};
		if (not Discard())
		{
			auto Result = Slice->AnsSeq->Push(LLSNeighborNode);
			//if (LoadCycle == 1900 && Slice->Slice == 0 && LoadStep == 13)
				//printf("Node %d willing to share,Neighbor Level %u,Push result is %d\n", LLSNeighborNode.Node->Index, LLSNeighborNode.LLSIndex,Result);
		}


		Slice->Visited[Neighbor->Index] = Slice->Visited[Node->Index];

	}
	
	if (!Slice->BFSSeq->Empty()) 
		FindNeighbors(Slice, LoadStep, LoadCycle);

}
unsigned int Slice_t::FindAncestor(unsigned int SIndex)
{
	if (Father[SIndex] == SIndex) 
		return SIndex;
	else
	{
		Father[SIndex] = FindAncestor(Father[SIndex]);
		return Father[SIndex];
	}
}

void TestPiece_t::ShowSideCrackFront(Slice_t* Slice)
{
	auto& SideCrack = Slice->SideCrack;

	PlanePos_t** Pos = new PlanePos_t* [3];

	SideCrack->Front(Pos);

	printf("%.2lf,%.2lf,%.2lf,%.2lf,%.2lf,%.2lf\n", Pos[0]->X, Pos[0]->Y, Pos[1]->X, Pos[1]->Y, Pos[2]->X, Pos[2]->Y);
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
	CrackFronts = new TQueue_t<PlanePos_t*>*[SlicesMax];
	for (unsigned int s = 0; s < SlicesMax; s++)
	{
		Fractures[s] = new TQueue_t<PlanePos_t*>(NodeMax);
		CrackFronts[s] = new TQueue_t<PlanePos_t*>(4);
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
			}
		}
		auto& Slice = TestPiece->Slice[s];
		if (Slice->Crack != nullptr) { }
		else if (Slice->SideCrack != nullptr)
		{
			PlanePos_t** Pos = new PlanePos_t* [3];
			Slice->SideCrack->Front(Pos);
			CrackFronts[s]->Push(Pos[0]);
			CrackFronts[s]->Push(Pos[1]);
			CrackFronts[s]->Push(Pos[2]);
		}
		else if (Slice->RectSideCrack != nullptr);
	}
}
FailureReport_t::FailureReport_t(FailureReport_t& From)
{
	this->Slice = From.Slice;
	this->Cycle = From.Cycle;
	this->CrackLen = From.CrackLen;

	Fractures = new TQueue_t<PlanePos_t*>*[SlicesMax];
	CrackFronts = new TQueue_t<PlanePos_t*>*[SlicesMax];
	for (unsigned int s = 0; s < SlicesMax; s++)
	{
		Fractures[s] = new TQueue_t<PlanePos_t*>(NodeMax);
		CrackFronts[s] = new TQueue_t<PlanePos_t*>(4);
	}
	for (unsigned int s = 0; s < SlicesMax; s++)
	{
		while (!From.Fractures[s]->Empty())
		{
			auto NewPlanePos = new PlanePos_t(*From.Fractures[s]->Front());
			Fractures[s]->Push(NewPlanePos);
			From.Fractures[s]->Pop();
		}
		while (!From.CrackFronts[s]->Empty())
		{
			auto NewPlanePos = new PlanePos_t(*From.CrackFronts[s]->Front());
			CrackFronts[s]->Push(NewPlanePos);
			From.CrackFronts[s]->Pop();
		}
	}
}
FailureReport_t::~FailureReport_t()
{
	// LET IT LEAK
	return;
}