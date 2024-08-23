#pragma once

#include <cmath>
#include <algorithm>
#include <vector>

#include <iostream>
#include <ostream>
#include <string>

typedef unsigned int int32;


struct Vec3d {

public:

	Vec3d():
		x(0), y(0), z(0)
	{}
	Vec3d(double x, double y, double z):
		x(x), y(y), z(z)
	{}
	Vec3d(double a) :
		x(a), y(a), z(a)
	{}
	Vec3d(const Vec3d& other) :
		x(other.x), y(other.y), z(other.z)
	{}

	double x;
	double y;
	double z;

	double length() const { return sqrt(x * x + y * y + z * z); }
	void normalize();
	void setValue(double a) { x = a; y = a, z = a; };
	double getMaxVal() const { return std::max(x, std::max(y, z)); }
	Vec3d max(const double a);
	Vec3d min(const double a);
	Vec3d max(const Vec3d& other);
	Vec3d min(const Vec3d& other);
	Vec3d abs();

	Vec3d operator-(const Vec3d& other) const;
	Vec3d operator-() const;
	Vec3d operator+(const Vec3d& other) const;

	Vec3d& operator+=(const Vec3d& other);
	Vec3d& operator-=(const Vec3d& other);

	Vec3d operator*(const double other) const;
	Vec3d operator/(const double other) const;
	Vec3d operator*(const Vec3d& other) const;
	Vec3d operator/(const Vec3d& other) const;

	Vec3d& operator*=(const double other);
	Vec3d& operator/=(const double other);

};

Vec3d operator*(const double s, const Vec3d& vec);
Vec3d operator/(const double s, const Vec3d& vec);

std::ostream& operator<< (std::ostream& outs, const Vec3d& v);


// this just implements very basic rotation matrix usage with just the Vec3d. 
// Not elegant or in anyways beautiful.
struct RotMat {
	RotMat()
	{}
	RotMat(Vec3d a, Vec3d b, Vec3d c)
	{
		rmat[0] = a;
		rmat[1] = b;
		rmat[2] = c;
	}

	Vec3d operator*(const Vec3d& other) const; // can use * to apply rotation
	Vec3d apply(const Vec3d& other) const;

	void setRow(int i, Vec3d r) { rmat[i] = r; }

	std::vector<Vec3d> rmat{ Vec3d(1,0,0), Vec3d(0,1,0), Vec3d(0,0,1) };
};


Vec3d elemExp(const Vec3d& a);

double ndot(const Vec3d& a, const Vec3d& b);

double dot(const Vec3d& a, const Vec3d& b);

Vec3d cross(const Vec3d& a, const Vec3d& b);

int32 xy2linear(int32 x, int32 y, int32 sx);

RotMat rotXYZ(const Vec3d& angles);
RotMat rotXZY(const Vec3d& angles);
RotMat rotYXZ(const Vec3d& angles);
RotMat rotYZX(const Vec3d& angles);
RotMat rotZYX(const Vec3d& angles);
RotMat rotZXY(const Vec3d& angles);

Vec3d mirrorByNormal(const Vec3d& input, const Vec3d& normal);
Vec3d computeRefractDir(const Vec3d& input, const Vec3d& normal, double n1, double n2);