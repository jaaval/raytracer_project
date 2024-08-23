#pragma once

#include <vector>
#include <utility>
#include "vectormath.h"

typedef unsigned char byte;


struct Color : public Vec3d
{
	Color() :
		Vec3d(0.0)
	{}
	Color(double R, double G, double B) :
		Vec3d(R, G, B)
	{}
	Color(double a) :
		Vec3d(a)
	{}
	Color(const Color& other) :
		Vec3d(other)
	{}
	Color(const Vec3d& other) :
		Vec3d(other)
	{}

	byte red();
	byte green();
	byte blue();
	std::vector<byte> colors();

	void clip();
};

Color averageColor(std::vector<std::pair<Color, double>>);


class Material
{

public:

	Material():
		color(Color(0)), refractIndex(0), reflectance(0), transparity(0), roughness(0),
		colorSet(false), reflectanceSet(false), refractIndexSet(false), transparitySet(false),
		roughnessSet(false)
	{}

	Color color;
	double refractIndex; // how to refract
	double reflectance; // how much reflect
	double transparity; // how much go through
	double roughness; // randomness of surface

	bool colorSet;
	bool reflectanceSet;
	bool refractIndexSet;
	bool transparitySet;
	bool roughnessSet;

	double getRefractAngle(double refInd2, double angle);
	//double getRefractivity(double rInd2, double angle);

private:
	

};
