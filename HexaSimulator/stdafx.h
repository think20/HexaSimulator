#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <cmath>

constexpr double E3 = 1000;
constexpr double E6 = 1000 * E3;
constexpr double E9 = 1000 * E6;
constexpr double EM3 = 0.001;
constexpr double EM6 = 0.001 * EM3;
constexpr double EM9 = 0.001 * EM6;
constexpr double EM11 = 0.01 * EM9;
constexpr double EM14 = 0.001 * EM11;
constexpr double pi = 3.14;

constexpr unsigned int ThreadMax = 4; //for Laptop

constexpr unsigned int SlicesMax = 16;//put here so both Crack.h and Hexa.h can see

constexpr double Tolerance = E3;

constexpr unsigned int CentralCrack = 0;
constexpr unsigned int SideCrack = 1;
constexpr unsigned int SCAlatering = 2;
constexpr unsigned int RectSideCrack = 10;

constexpr unsigned int MaxCycle = 500'0000;
constexpr char Version[] = "20251111v1";

// Reaction Layer damage and following fiber damage should be added, 1400MPa

