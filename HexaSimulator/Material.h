#pragma once
//0.45Vf = 361(361) fibers = F1
//0.45Vf = 361(365) fibers = F2
//0.43Vf = 337(338) fibers = F3
//0.37Vf = 295(295) fibers = LF1
//0.36vf = 199(195) fibers = BLFRT1
//For each Fiber, Vf is actual, for Specimen, Vf>Vf_Actual. Rim Filled by matrix without fiber.

#ifdef vFiberF11
constexpr double Vf = 0.45;
constexpr double OffsetK = 0.2529;
constexpr double OffsetB = -0.6441;
#define SiCf_Ti6242HT true
//no Expdata
#endif

#ifdef vFiberF12
constexpr double Vf = 0.45;
constexpr double OffsetK = 0.2529;
constexpr double OffsetB = -0.6441;
#define SiCf_Ti6242HT true
//no Expdata
#endif

#ifdef vFiberF13
constexpr double Vf = 0.45;
constexpr double OffsetK = 0.2529;
constexpr double OffsetB = -0.6441;
#define SiCf_Ti6242HT true
//no Expdata
#endif

#ifdef vFiberF14
constexpr double Vf = 0.45;
constexpr double OffsetK = 0.2529;
constexpr double OffsetB = -0.6441;
#define SiCf_Ti6242HT true
//no Expdata
#endif

#ifdef F1
constexpr double Vf = 0.45;
constexpr double OffsetK = 0.2529;
constexpr double OffsetB = -0.6441;
#define SiCf_Ti6242HT true
//Exp 44345 SpectrumCyc
#endif

#ifdef F2
constexpr double Vf = 0.45;
constexpr double OffsetK = 0.2529;
constexpr double OffsetB = -0.6441;
#define SiCf_Ti6242HT true
//Exp 4326 SCyc
#endif

#ifdef F3
constexpr double Vf = 0.43;
constexpr double OffsetK = 0.3363;
constexpr double OffsetB = 2.2076;
#define SiCf_Ti6242HT true
//Exp 2813 SCyc
#endif

#ifdef FiberT1
constexpr double Vf = 0.43;
constexpr double OffsetK = 0.3363;
constexpr double OffsetB = 2.2076;
#define SiCf_Ti6242HT true
//Tension predection requires multiple cracks in a slice,do not use
#endif

#ifdef LF1
constexpr double Vf = 0.37;
constexpr double OffsetK = 0.3363;
constexpr double OffsetB = 2.2076;
#define SiCf_Ti6242RT true
//Exp 6955 SCyc
#endif

#ifdef BLFRT1
constexpr double Vf = 0.36; 
constexpr double OffsetK = 0.3363;
constexpr double OffsetB = 2.2076;
#define SCS6_Ti6242RT true
#endif

#ifdef BLFRT2
constexpr double Vf = 0.36;
constexpr double OffsetK = 0.3363;
constexpr double OffsetB = 2.2076;
#define SCS6_Ti6242RT true
#endif

#ifdef BLFRT3
constexpr double Vf = 0.36;
constexpr double OffsetK = 0.3363;
constexpr double OffsetB = 2.2076;
#define SCS6_Ti6242RT true
#endif

#ifdef BLFRT4
constexpr double Vf = 0.36;
constexpr double OffsetK = 0.3363;
constexpr double OffsetB = 2.2076;
#define SCS6_Ti6242RT true
#endif

#ifdef BLFRT5
constexpr double Vf = 0.36;
constexpr double OffsetK = 0.3363;
constexpr double OffsetB = 2.2076;
#define SCS6_Ti6242RT true
#endif

#ifdef BLFRT6
constexpr double Vf = 0.36;
constexpr double OffsetK = 0.3363;
constexpr double OffsetB = 2.2076;
#define SCS6_Ti6242RT true
#endif

#ifdef BLFHT1
constexpr double Vf = 0.36;
constexpr double OffsetK = 0.3363;
constexpr double OffsetB = 2.2076;
#define SCS6_Ti6242HT true
#endif

#ifdef BLFHT2
constexpr double Vf = 0.36;
constexpr double OffsetK = 0.3363;
constexpr double OffsetB = 2.2076;
#define SCS6_Ti6242HT true
#endif

#ifdef BLFHT3
constexpr double Vf = 0.36;
constexpr double OffsetK = 0.3363;
constexpr double OffsetB = 2.2076;
#define SCS6_Ti6242HT true
#endif

#ifdef BLFHT4
constexpr double Vf = 0.36;
constexpr double OffsetK = 0.3363;
constexpr double OffsetB = 2.2076;
#define SCS6_Ti6242HT true
#endif

#ifdef BLFHT5
constexpr double Vf = 0.36;
constexpr double OffsetK = 0.3363;
constexpr double OffsetB = 2.2076;
#define SCS6_Ti6242HT true
#endif

#ifdef BLFHT6
constexpr double Vf = 0.36;
constexpr double OffsetK = 0.3363;
constexpr double OffsetB = 2.2076;
#define SCS6_Ti6242HT true
#endif

constexpr double Vm = 1 - Vf;

#ifdef SiCf_Ti6242HT
//Elastic Modules
constexpr double Ef = 434.3 * E9;
constexpr double Em = 88.3 * E9;

//Yielding Strength
constexpr double Sigmams = 550 * E6;

//Heat Residual Stress
constexpr double Alpham = 8.7 * EM6;
constexpr double Alphaf = 4.5 * EM6;
constexpr double Nium = 0.33;
constexpr double Niuf = 0.25;
constexpr double DeltaT = -320;
constexpr double dint = 2.5 * EM6;

constexpr double Alphac = (Alpham * Vm * Em + Alphaf * Vf * Ef) / (Vf * Ef + Vm * Em);
constexpr double Gamma = Ef * Em / (Ef * (1 + Nium) + Em * (1 - Niuf));

constexpr double Sigmam_heat_radical = Gamma * (Alphac - Alpham) * DeltaT;
constexpr double Sigmam_heat_axial = Em * (Alphac - Alpham) * DeltaT;


const double SigmamMax = (2.0 * Sigmam_heat_radical + sqrt(16.0 * Sigmams * Sigmams - 12.0 * Sigmam_heat_radical * Sigmam_heat_radical) - 4.0 * Sigmam_heat_axial) / 4.0;

//Crack Propgation 
//dadN(m) = Cm*(DeltaK(Mpa))^Mm
constexpr double Cm[] = { 2 * EM14,3 * EM11 }; //3*10^-11
constexpr double Mm[] = { 11.0,3.1256 };
constexpr double KLow = 2.5;
constexpr double KIC = 70;

constexpr double Sigma0 = 3160 * E6 * 0.85;//3160MPa
//constexpr double Sigma0 = 2000 * E6;
constexpr double mf = 14.89*0.455;
constexpr double Lg = 25.4 * EM3; //25.4mm Gauge Length

constexpr double InitCrackRadious = 55 * EM6;

constexpr double R0 = 102 * EM6 * 0.5;

constexpr double SpecimenRadious = 1500 * EM6;

#endif

#ifdef SiCf_Ti6242RT
//Elastic Modules
constexpr double Ef = 429 * E9;
constexpr double Em = 126 * E9;

//Yielding Strength
constexpr double Sigmams = 900 * E6;

//Heat Residual Stress
constexpr double Alpham = 8.7 * EM6;
constexpr double Alphaf = 4.5 * EM6;
constexpr double Nium = 0.33;
constexpr double Niuf = 0.25;
constexpr double DeltaT = -820;
constexpr double dint = 2.5 * EM6;

constexpr double Alphac = (Alpham * Vm * Em + Alphaf * Vf * Ef) / (Vf * Ef + Vm * Em);
constexpr double Gamma = Ef * Em / (Ef * (1 + Nium) + Em * (1 - Niuf));

constexpr double Sigmam_heat_radical = Gamma * (Alphac - Alpham) * DeltaT;
constexpr double Sigmam_heat_axial = Em * (Alphac - Alpham) * DeltaT;


const double SigmamMax = (2.0 * Sigmam_heat_radical + sqrt(16.0 * Sigmams * Sigmams - 12.0 * Sigmam_heat_radical * Sigmam_heat_radical) - 4.0 * Sigmam_heat_axial) / 4.0;

//Crack Propgation 
//dadN(m) = Cm*(DeltaK(Mpa))^Mm
constexpr double Cm[] = {2 * EM14,2.678 * EM11}; //2.678*10^-11
constexpr double Mm[] = { 11.0,3.812 };
constexpr double KLow = 2.5;
constexpr double KIC = 70;

constexpr double Sigma0 = 3160 * E6 * 0.85;//3160MPa
//constexpr double Sigma0 = 2000 * E6;
constexpr double mf = 14.89 * 0.455;
constexpr double Lg = 25.4 * EM3; //25.4mm Gauge Length

constexpr double InitCrackRadious = 55 * EM6;

constexpr double R0 = 102 * EM6 * 0.5;

constexpr double SpecimenRadious = 1500 * EM6;

#endif

#ifdef SCS6_Ti6242HT
//Elastic Modules
constexpr double Ef = 429 * E9;
constexpr double Em = 88.3 * E9;

//Yielding Strength
constexpr double Sigmams = 523 * E6;

//Heat Residual Stress
constexpr double Alpham = 8.7 * EM6;
constexpr double Alphaf = 4.5 * EM6;
constexpr double Nium = 0.33;
constexpr double Niuf = 0.25;
constexpr double DeltaT = -320;
constexpr double dint = 2.5 * EM6;

constexpr double Alphac = (Alpham * Vm * Em + Alphaf * Vf * Ef) / (Vf * Ef + Vm * Em);
constexpr double Gamma = Ef * Em / (Ef * (1 + Nium) + Em * (1 - Niuf));

constexpr double Sigmam_heat_radical = Gamma * (Alphac - Alpham) * DeltaT;
constexpr double Sigmam_heat_axial = Em * (Alphac - Alpham) * DeltaT;


const double SigmamMax = (2.0 * Sigmam_heat_radical + sqrt(16.0 * Sigmams * Sigmams - 12.0 * Sigmam_heat_radical * Sigmam_heat_radical) - 4.0 * Sigmam_heat_axial) / 4.0;

//Crack Propgation 
//dadN(m) = Cm*(DeltaK(Mpa))^Mm
constexpr double Cm[] = { 2 * EM14,3 * EM11 }; //3*10^-11
constexpr double Mm[] = { 11.0,3.1256 };
constexpr double KLow = 2.5;
constexpr double KIC = 70;

constexpr double Sigma0 = 4600 * E6 * 0.85;//3160MPa
//constexpr double Sigma0 = 2000 * E6;
constexpr double mf = 14.89 * 0.455;
constexpr double Lg = 25.4 * EM3; //25.4mm Gauge Length

constexpr double InitCrackRadious = 55 * EM6;

constexpr double R0 = 142 * EM6 * 0.5;

constexpr double SpecimenRadious = 3500 * EM6 * 0.5;

#endif

#ifdef SCS6_Ti6242RT
//Elastic Modules
constexpr double Ef = 429 * E9;
constexpr double Em = 126 * E9;

//Yielding Strength
constexpr double Sigmams = 1163 * E6;

//Heat Residual Stress
constexpr double Alpham = 8.7 * EM6;
constexpr double Alphaf = 4.5 * EM6;
constexpr double Nium = 0.33;
constexpr double Niuf = 0.25;
constexpr double DeltaT = -820;
constexpr double dint = 2.5 * EM6;

constexpr double Alphac = (Alpham * Vm * Em + Alphaf * Vf * Ef) / (Vf * Ef + Vm * Em);
constexpr double Gamma = Ef * Em / (Ef * (1 + Nium) + Em * (1 - Niuf));

constexpr double Sigmam_heat_radical = Gamma * (Alphac - Alpham) * DeltaT;
constexpr double Sigmam_heat_axial = Em * (Alphac - Alpham) * DeltaT;


const double SigmamMax = (2.0 * Sigmam_heat_radical + sqrt(16.0 * Sigmams * Sigmams - 12.0 * Sigmam_heat_radical * Sigmam_heat_radical) - 4.0 * Sigmam_heat_axial) / 4.0;

//Crack Propgation 
//dadN(m) = Cm*(DeltaK(Mpa))^Mm
constexpr double Cm[] = { 2 * EM14,2.678 * EM11 }; //2.678*10^-11
constexpr double Mm[] = { 14,3.812 };
constexpr double KLow = 2.5;
constexpr double KIC = 70;

constexpr double Sigma0 = 4600 * E6 * 0.85;//3160MPa
//constexpr double Sigma0 = 2000 * E6;
constexpr double mf = 14.89 * 0.455;
constexpr double Lg = 25.4 * EM3; //25.4mm Gauge Length

constexpr double InitCrackRadious = 55 * EM6;

constexpr double R0 = 142 * EM6 * 0.5;

constexpr double SpecimenRadious = 3500 * EM6 * 0.5;

#endif

constexpr double L = 100 * EM6; //100um segment
const double SigmaL = Sigma0 * powf(L, -1.0 / mf) * powf(Lg, 1.0 / mf);

constexpr double FiberLoadFactor = Ef / (Ef * Vf + Em * Vm);
constexpr double MatrixLoadFactor = Em / (Ef * Vf + Em * Vm);


const double R1 = sqrt(pi / (2 * sqrt(3) * Vf)) * R0; //Inner circle radius of a virtual hexagen

constexpr double ThetaThresh = 7; //above 7 degree of deviation cause compression damage
constexpr double OffsetFactor = 60;// 60% per deg

constexpr unsigned int SlidingStrength0 = 100 * E6;


