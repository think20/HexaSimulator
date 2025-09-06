#pragma once
class Crack_t
{
public: 
	Crack_t(double a0 = 0, double R = 0) : Yb(0), Yield(false), MaxCrackLen(0.99 * R), Sigmab(0), a(a0), Req(R) {}
	double const& UpdateCrack(double);
	double const& Len();
	double Yb;
	bool Yield;
	double MaxCrackLen;

private:
	double Sigmab;
	double a;
	double Req;
};

constexpr unsigned int bTerm = 5;
constexpr unsigned int cTerm = 5;
constexpr double bPara[bTerm] = { 0.906,-1.23,5.99,12,-11.2 };
constexpr double cPara[cTerm] = { 1.12,-2.61,7.34,1.41,-4.28 };
constexpr double dbFactor = 1.5;
constexpr double dcFactor = 1;


class SideCrack_t
{
public:
	SideCrack_t(double b0 = 0, double c0 = 0, double R = 0) : S(0), MaxCrackLen(0.99 * 2 * R), Yb(0), Yc(0), Kb(0), Kc(0), b(b0), c(c0), D(2 * R), R(R), k0(0), b0(0) {}
	double UpdateCrack(double);
	double DistanceToCrackFront(double x, double y);
	double S;
	double MaxCrackLen;
private:
	double Yb, Yc;
	double Kb, Kc;
	double b, c, D, R;
	double k0, b0;
};