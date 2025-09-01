#pragma once
class Crack_t
{
public: 
	Crack_t(double a0 = 0, double R = 0) : Yb(0), MaxCrackLen(0.9 * R), Sigmab(0), a(a0), Req(R) {}
	double const& UpdateCrack(double);
	double Yb;
	double MaxCrackLen;

private:
	double Sigmab;
	double a;
	double Req;
};
