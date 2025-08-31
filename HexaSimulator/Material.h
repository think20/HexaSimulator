#pragma once
//0.45Vf = 361(361) fibers = F1
//0.45Vf = 361(365) fibers = F2
//0.43Vf = 337(338) fibers = F3
//For each Fiber, Vf is actual, for Specimen, Vf>Vf_Actual. Rim Filled by matrix without fiber.

#ifdef FiberF1
constexpr double Vf = 0.45;
constexpr double OffsetK = 0.2529;
constexpr double OffsetB = -0.6441;
#define SiCf_Ti6242HT true
#endif

#ifdef FiberF2
constexpr double Vf = 0.45;
constexpr double OffsetK = 0.2529;
constexpr double OffsetB = -0.6441;
#define SiCf_Ti6242HT true
#endif

#ifdef FiberF3
constexpr double Vf = 0.43;
constexpr double OffsetK = 0.3363;
constexpr double OffsetB = 2.2076;
#define SiCf_Ti6242HT true
#endif

constexpr double Vm = 1 - Vf;

#ifdef SiCf_Ti6242HT
constexpr double Ef = 434.3 * E9;
constexpr double Em = 88.3 * E9;

//dadN(m) = Cm*(DeltaK(Mpa))^Mm
constexpr double Cm = 3 * EM9 * 0.01; //3*10^-11
constexpr double Mm = 3.1256;

constexpr double MartixYieldStrength = 700 * E6;//to be looked up
constexpr double HeatResidualStress = 100 * E6;//to be looked up

constexpr double Sigma0 = 3160 * E6;//3160MPa
constexpr double mf = 14.89;
constexpr double Lg = 25.4 * EM3; //25.4mm Gauge Length

#endif

constexpr double L = 100 * EM6; //100um segment
const double SigmaL = Sigma0 * powf(L, -1.0 / mf) * powf(Lg, 1.0 / mf);

constexpr double FiberLoadFactor = Ef / (Ef * Vf + Em * Vm);
constexpr double MatrixLoadFactor = Em / (Ef * Vf + Em * Vm);

constexpr double R0 = 102 * EM6 * 0.5;
const double R1 = sqrt(pi / (2 * sqrt(3) * Vf)) * R0; //Inner circle radius of a virtual hexagen

constexpr double ThetaThresh = 7; //above 7 degree of deviation cause compression damage
constexpr double OffsetFactor = 60;// 60% per deg

constexpr unsigned int SlidingStrength0 = 100 * E6;

