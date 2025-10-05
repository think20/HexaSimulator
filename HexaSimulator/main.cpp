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
	auto TestPiece = new TestPiece_t;
	TestPiece->FiberLoadP(1000'0000);
	//auto Hexa = new Hexa_t;

	//printf("%.2lf\n", sin(6.28));


	return 0;

}