#pragma once
class Crack_t
{
public: 
	Crack_t(double R) :Req(R) {}

private:
	double dadN(double Sigma, double a);
	double a;
	double Req;
};
