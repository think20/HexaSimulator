#include "stdafx.h"
#include "Hexa.h"
#include "Load.h"
#include "Visualizer.h"

#include <cmath>
#include <ctime>
#include <iostream>
#include <Windows.h>
#include <cstdlib>
#include <atomic>
#include <random>


int main()
{
	//auto Hexa = new Hexa_t;
	//Hexa->HexaGenerateP();

	unsigned int BrokenFiberCount = 0;

	auto TestPiece = new TestPiece_t;
	TestPiece->FiberLoadP(10000);

	TVector_t<int> Vec(100);
	printf("%d\n", Vec.Empty());

	//int C=SavePic(Nodes);

	//int C = ShowTheta(Nodes);
	//printf("%u\n", C);
	//system("pause");
	/*for (unsigned int Cycle = 1; Cycle < 50'0000; Cycle++)
	{
		BrokenFiberCount += Hexa->FiberLoad(Hexa->Nodes[0], Cycle);
		if (BrokenFiberCount >= Hexa->NodeCount[0] / 2)
		{
			printf("%d,%d\n", Cycle, BrokenFiberCount);
			break;
		}
	}*/

	
	//printf("%d\n", BrokenFiberCount);

	return 0;
}