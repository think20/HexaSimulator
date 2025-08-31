#pragma once
class Crack_t
{
public: 
	Crack_t(double a0,double R) :Sigmab(0),Yb(0),a(a0),Req(R) {}
	double const& UpdateCrack(double);
	double Sigmab;

private:
	double Yb;
	double a;
	double Req;
};
