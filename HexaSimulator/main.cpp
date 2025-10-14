#include "stdafx.h"
#include "Hexa.h"
#include "Load.h"
#include "Crack.h"
#include "Visualizer.h"
#include "TestManager.h"

#include <cmath>
#include <ctime>
#include <iostream>
#include <Windows.h>
#include <cstdlib>
#include <atomic>
#include <random>


int main()
{
	auto TestManager = new TestManager_t;
	TestManager->CreateTests();
	TestManager->StartTests();
	TestManager->SaveResults();
	//auto Hexa = new Hexa_t;

	//printf("%.2lf\n", sin(6.28));


	return 0;

}