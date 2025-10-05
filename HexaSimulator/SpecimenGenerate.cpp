#include "Load.h"

Slice_t::Slice_t(Hexa_t* Hexa, unsigned int Slice)
{
	this->Hexa = Hexa;
	this->Slice = Slice;
	this->Nodes = new TVector_t<Node_t*>(Hexa->NodeCount[Slice] + 10);
	for (unsigned int Index = 1; Index < NodeMax; Index++)
	{
		auto& Node = Hexa->Nodes[Slice][Index];
		if (!Node->OverBoarder) Nodes->Push(Node);
	}
	this->Capablity = Hexa->Capablity[Slice];
	this->Crack = Hexa->CrackManager->Crack[Slice];
	this->SideCrack = Hexa->CrackManager->SideCrack[Slice];
	this->NodeCount = Hexa->NodeCount[Slice];

	this->Visited = new int64_t[NodeMax];
	for (auto n = 0; n < NodeMax; n++)
		this->Visited[n] = 0;
	
	this->Color = 0;

	this->OverLoadNodes = new TQueue_t<Node_t*>(NodeMax);
	this->BFSSeq = new TQueue_t<LLSNode_t>(NodeMax);
	this->AnsSeq = new TVector_t<LLSNode_t>(NodeMax);
	this->ToUpdate = new TQueue_t<Node_t*>(NodeMax);
}
TestPiece_t::TestPiece_t()
{
	this->Hexa = new Hexa_t;

	this->Slice = new Slice_t* [SlicesMax];
	for (unsigned int s = 0; s < SlicesMax; s++)
	{
		Slice[s] = new Slice_t(this->Hexa, s);
	}

	this->Ready = true;
	this->SyncTotalStep = 1;

	this->DebugCounter = new unsigned int[SlicesMax];
	for (unsigned int s = 0; s < SlicesMax; s++)
		DebugCounter[s] = 0;

	this->Finish = 0;
}