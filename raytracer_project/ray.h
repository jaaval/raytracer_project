#pragma once

#include "vectormath.h"
#include "Scene.h"
#include <utility> // for std::pair

struct Intersect
{
	Intersect() :
		d(0), object(nullptr), pos(Vec3d(0))
	{};
	Intersect(SDFObject* a, double d, Vec3d pos) :
		object(a), d(d), pos(pos)
	{}
	double d ;
	SDFObject* object;
	Vec3d pos;
};


class Ray {

public:

	Ray(Vec3d orig_p, Vec3d direction, double rayStr, int depth) :
		orig_p(orig_p), direction(direction), current_p(orig_p), rayStr(rayStr), recursion(depth)
	{
		direction.normalize();
	};
	Ray(Vec3d orig_p, Vec3d direction) :
		orig_p(orig_p), direction(direction), current_p(orig_p), rayStr(1), recursion(0)
	{
		direction.normalize();
	};
	Ray(Vec3d orig_p , double rayStr) :
		orig_p(orig_p), current_p(orig_p), rayStr(rayStr), recursion(0)
	{};
	Ray(Vec3d orig_p) :
		orig_p(orig_p), current_p(orig_p), rayStr(1), recursion(0)
	{};

	void takeStep(double stepLength) { current_p += stepLength * direction; }

	// compute step length
	Intersect computeStepLength(const std::vector<SDFObject*>& objects, const Vec3d& pos) const;
	// computes march without moving the ray object
	Intersect castRay(const std::vector<SDFObject*>& objects, const Vec3d& pos, const Vec3d& direction) const;

	//

	void setDirection2point(const Vec3d& pixelLoc);
	void setDirection(Vec3d& dirvec);
	void setCurrentRefractIndex(double a) { currentRefractIndex = a; }

	Color march(const Scene& scene);
	Color monteCarloMarch(const Scene& scene);
	Color marchLight(const Scene& scene);
	
	
	const Vec3d& getCurrentP() const { return current_p; }
	const Vec3d& getDirection() const { return direction; }
	const Vec3d& getOrigP() const { return orig_p; }
	int getRecursionLevel() const { return recursion; }

	
	double softshadow(const std::vector<SDFObject*>& objects, double maxt, double k);


private:
	Vec3d direction;
	Vec3d orig_p;
	Vec3d current_p;
	double totalDist = 0;
	double currentRefractIndex = 1.0;
	int recursion; // indicates the number of reflections or refractions
	double rayStr; // value from 0 to 1. represents the strength of the ray
	
	SDFObject* currentObject = nullptr;
};

double computeFresnellReflectance(const Vec3d& normal, const Vec3d& dir, double n1, double n2);