#pragma once



double cubicPolynomial(double a, double b, double k);
double smoothStep(double a, double b, double x);
double biasSqrt(double x, double k);
double softmin(double a, double b, double k);
double softmax(double a, double b, double k);
double blend(double a, double b, double k);

int nearestNeighbor(int value, int ulim1, int ulim2);

template<typename T>
T clamp(T value, T lower, T upper)
{
	return std::max(lower, std::min(upper, value));
}

double uniformRand();
double uniformRand(double, double);
double normalRand();
double normalRand(double, double);