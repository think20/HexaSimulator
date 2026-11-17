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

constexpr unsigned int FixedOrient = 20;
constexpr unsigned int FanShapeOrient = 21;
constexpr unsigned int RandomOrient = 22;

constexpr unsigned int MaxCycle = 20'000'000;

// 1 = 对照组：仅保留 K_IC 判据，屏蔽 MCL 与 MLR
// 模型只保留 MLR + K_IC 两条失效通道；最大裂纹长度判据（b/D=0.5）已取消
#ifndef KIC_ONLY
#define KIC_ONLY 0
#endif
constexpr char Version[] = "20251126v1";

// Reaction Layer damage and following fiber damage should be added, 1400MPa

