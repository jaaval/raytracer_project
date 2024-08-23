
#include "sdfobject.h"
#include <cmath>
#include <algorithm>
#include "utilmath.h"
#include <sstream>
#include <iostream>
#include "constants.h"

double SDFBlend(double a, double b, double k) {
	return blend(a, b, k);
}
double SDFUnion(double a, double b, double k)
{
	return softmin(a, b, k);
}
double SDFIntersect(double a, double b, double k)
{
	return softmax(a, b, k);
}
double SDFSubtract_first(double a, double b, double k)
{
	return softmax(-a, b, k);
}
double SDFSubtract(double a, double b, double k)
{
	return softmax(a, -b, k);
}


Vec3d SDFObject::computeNormal(Vec3d& p)
{
	// evaluates sdffun 4 times. heavy.
	double fp = sdfFun(p);
	Vec3d ph1 = p + Vec3d(NORMALCOMPCONST, 0, 0);
	Vec3d ph2 = p + Vec3d(0, NORMALCOMPCONST, 0);
	Vec3d ph3 = p + Vec3d(0, 0, NORMALCOMPCONST);
	double val1 = sdfFun(ph1) - fp;
	double val2 = sdfFun(ph2) - fp;
	double val3 = sdfFun(ph3) - fp;
	Vec3d grad(val1, val2, val3);
	grad.normalize();
	return grad;
}


const Material& SDFGroup::getMaterial(const Vec3d& pos) const
{
	Vec3d p = rot * (pos - orig);
	double d, d1 = 1e20;
	int pick = 0, i = 0;
	for (auto iter = contents.begin(); iter < contents.end(); iter++, i++)
	{
		d = (*iter)->sdfFun(p);
		if (d < d1)
		{
			d1 = d;
			pick = i;
		}
	}
	return contents[pick]->getMaterial(p);
}

Color SDFGroup::getColor(const Vec3d& pos) const
{
	Vec3d p = rot * (pos - orig);
	double d;
	double sum = 0;
	Color output(0);
	for (auto iter = contents.begin(); iter < contents.end(); iter++)
	{
		d = 1.0/(0.0001 + (*iter)->sdfFun(p));
		output += d * (*iter)->getMaterial(p).color;
		sum += d;
	}
	output /= sum;
	return output;
}
double SDFGroup::getRefractIndex(const Vec3d& pos) const
{
	return getMaterial(pos).refractIndex;
}
double SDFGroup::getReflectance(const Vec3d& pos) const
{
	Vec3d p = rot * (pos - orig);
	double d;
	double sum = 0;
	double output = 0;
	for (auto iter = contents.begin(); iter < contents.end(); iter++)
	{
		d = 1.0 / (0.0001 + (*iter)->sdfFun(p));
		output += d * (*iter)->getMaterial(p).reflectance;
		sum += d;
	}
	output /= sum;
	return output;
}
double SDFGroup::getTransparity(const Vec3d& pos) const
{
	return getMaterial(pos).transparity;
}
double SDFGroup::getRoughness(const Vec3d& pos) const
{
	Vec3d p = rot * (pos - orig);
	double d;
	double sum = 0;
	double output = 0;
	for (auto iter = contents.begin(); iter < contents.end(); iter++)
	{
		d = 1.0 / (0.0001 + (*iter)->sdfFun(p));
		output += d * (*iter)->getMaterial(p).roughness;
		sum += d;
	}
	output /= sum;
	return output;
}

void SDFGroup::setMemberMaterials() 
{
	for (auto iter = contents.begin(); iter < contents.end(); iter++)
	{
		if (mat.colorSet)
			(*iter)->setColor(mat.color);
		if (mat.refractIndexSet)
			(*iter)->setRefractIndex(mat.refractIndex);
		if (mat.reflectanceSet)
			(*iter)->setReflectance(mat.reflectance);
		if (mat.transparitySet)
			(*iter)->setTransparity(mat.transparity);
		if (mat.roughnessSet)
			(*iter)->setRoughness(mat.roughness);
	}
}



double Ellipsoid::sdfFun(const Vec3d& op) const
{
	double d, k0, k1;
	Vec3d p = rot * (op - orig);
	//std::cout << orig << std::endl;
	
	if (r.x == r.y && r.x == r.z) {
		d = p.length() - r.x; // sphere, exact
	}
	else {// not mathematically exact ellipsoid. Can't be helped.
		k0 = (p / r).length(); 
		k1 = (p / (r * r)).length();
		d =  k0 * (k0 - 1.0) / k1;
	}
	return d;
}


double Box::sdfFun(const Vec3d& op) const
{
	Vec3d p = rot * (op - orig);
	Vec3d q = p.abs() - s;
	double d = (q.max(0.0) + std::min(q.getMaxVal(), 0.0)).length() - softFactor;
	return d;
}


double Torus::sdfFun(const Vec3d& op) const
{
	Vec3d p = rot * (op - orig);
	Vec3d q = Vec3d(sqrt(p.x*p.x + p.z*p.z)-t.x, p.y, 0.0);
	return q.length() - t.y;
}


double Plane::sdfFun(const Vec3d& op) const
{	
	Vec3d p = rot * (op - orig);
	return dot(p, n);
}


double Cone::sdfFun(const Vec3d& op) const
{
	Vec3d p = rot * (op - orig);
	double q = sqrt(p.x * p.x + p.y * p.y);
	return std::max(c.x*q + c.y*p.z, -h-p.z);
}


double InfCone::sdfFun(const Vec3d& op) const
{
	Vec3d p = rot * (op - orig);
	Vec3d q = Vec3d(sqrt(p.x * p.x + p.y * p.y), -p.z, 0.0);
	double d = (q - c * std::max(dot(q, c), 0.0)).length();
	return d * (q.x * c.y - q.y * c.x < 0.0) ? -1.0 : 1.0;
}

double Line::sdfFun(const Vec3d& op) const
{
	Vec3d p = rot * (op - orig);
	Vec3d pa = p - a;
	Vec3d ba = b - a;
	double h = clamp(dot(pa, ba)/dot(ba,ba), 0.0, 1.0);
	return (pa - ba * h).length() - r;
}

double Cylinder::sdfFun(const Vec3d& op) const
{
	Vec3d p = rot * (op - orig);
	Vec3d d = Vec3d(sqrt(p.x * p.x + p.y * p.y) - r, std::abs(p.z) - h, 0.0);
	return std::min(std::max(d.x, d.y), 0.0) + d.max(0.0).length();
}

double SDFGroup::sdfFun(const Vec3d& op) const
{
	Vec3d p = rot * (op - orig);
	//Vec3d p = op;
	//std::cout << orig << std::endl;
	double d1;
	d1 = contents[0]->sdfFun(p);
	if (contents.size() == 1)
		return d1;

	for (auto iter = ++contents.begin(); iter < contents.end(); iter++)
	{
		d1 = sdfSumFun(d1, (*iter)->sdfFun(p));
	}
	return d1;
}




