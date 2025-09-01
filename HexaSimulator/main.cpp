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
	TestPiece->FiberLoadP(100'0000);

	return 0;
}