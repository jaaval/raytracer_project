
#include <cmath>
#include <algorithm>
#include "utilmath.h"
#include <random>

double softmin(double a, double b, double k)
{
	return std::min(a, b) - cubicPolynomial(a, b, k);
}

double softmax(double a, double b, double k)
{
	return std::max(a, b) + cubicPolynomial(a, b, k);
}

double blend(double a, double b, double k) {
	return k * a + (1 - k) * b;
}

double biasSqrt(double x, double k) {
	return sqrt(x * x + k);
}

double cubicPolynomial(double a, double b, double k) {
	if (k == 0)
		return 0;
	return pow(std::max(k - abs(a - b), 0.0), 3.0) / (6.0 * k * k);
}

double smoothStep(double a, double b, double x) {
	double lmbd = (x - a) / (b - a);
	lmbd = std::max(std::min(lmbd, 1.0), -1.0); // unit saturation, this maybe needs to be 0 not -1 limited
	return lmbd * lmbd * (3 - 2 * lmbd);
}

int nearestNeighbor(int value, int ulim1, int ulim2)
{
	if (ulim1 == ulim2)
		return value;
	double scale = (double)ulim1 / (double)ulim2;
	double newvalue = (double)value * scale;
	return (int)newvalue;
}


// thread safe random numbers

double uniformRand()
{
	static thread_local std::mt19937 generator;
	std::uniform_real_distribution<double> distribution(0.0, 1.0);
	return distribution(generator);
}

double uniformRand(double l, double u)
{
	static thread_local std::mt19937 generator;
	std::uniform_real_distribution<double> distribution(l, u);
	return distribution(generator);
}


double normalRand()
{
	static thread_local std::mt19937 generator;
	std::normal_distribution<double> distribution(0.0, 1.0);
	return distribution(generator);
}

double normalRand(double m, double sd)
{
	static thread_local std::mt19937 generator;
	std::normal_distribution<double> distribution(m, sd);
	return distribution(generator);
}