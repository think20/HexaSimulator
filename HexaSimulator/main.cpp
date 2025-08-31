#include "stdafx.h"
#include "Hexa.h"
#include "Load.h"
#include "Crack.h"
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

	/*auto TestPiece = new TestPiece_t;
	TestPiece->FiberLoadP(1);

	TVector_t<int> Vec(100);
	printf("%d\n", Vec.Empty());*/

	/*double a0 = 50 * EM6;
	for (unsigned int c = 0; c <= 10'0000; c++)
	{
		a0 += dadN(1250 * E6 * MatrixLoadFactor, a0);
		if (!(c % 100))
			printf("%d:%lf\n", c, a0*E6);
	}*/

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