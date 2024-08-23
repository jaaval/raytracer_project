#pragma once

#include <vector>
#include <tuple>
#include <cmath>
#include <functional>
#include "vectormath.h"
#include "Material.h"
#include <string>
#include "constants.h"
#include <sstream>



double SDFBlend(double a, double b, double k);
double SDFUnion(double a, double b, double k);
double SDFIntersect(double a, double b, double k);
double SDFSubtract_first(double a, double b, double k);
double SDFSubtract(double a, double b, double k);


class SDFObject {

public:
	// origin
	Vec3d orig;
	RotMat rot;

	virtual ~SDFObject() {};

	virtual SDFObject* clone() const = 0;

	virtual double sdfFun(const Vec3d& iPnt) const = 0;

	virtual std::string serialize() const
	{
		std::stringstream ss;
		ss << "Object at " << orig << std::endl;
		return ss.str();
	}

	Vec3d computeNormal(Vec3d& p);
	int getType() { return type; }
	
	virtual void setOrigin(const Vec3d& norig) { orig = Vec3d(norig); }

	virtual Color getColor(const Vec3d& pos) const { return getMaterial(pos).color; };
	virtual double getRefractIndex(const Vec3d& pos) const { return getMaterial(pos).refractIndex; };
	virtual double getReflectance(const Vec3d& pos) const { return getMaterial(pos).reflectance; };
	virtual double getTransparity(const Vec3d& pos) const { return getMaterial(pos).transparity; };
	virtual double getRoughness(const Vec3d& pos) const { return getMaterial(pos).roughness; }
	virtual const Material& getMaterial(const Vec3d& pos) const { return mat; }
	
	void setColor(Color col) { mat.color = col; mat.colorSet = true; }
	void setRefractIndex(double a) { mat.refractIndex = a; mat.refractIndexSet = true; }
	void setTransparity(double a) { mat.transparity = a; mat.transparitySet = true; }
	void setReflectance(double a) { mat.reflectance = a; mat.reflectanceSet = true; }
	void setRoughness(double a) { mat.roughness = a; mat.roughnessSet = true; }
	void setDefaultMaterial() 
	{
		if (!mat.colorSet)
			mat.color = Color(DEFR, DEFG, DEFB);
		if (!mat.refractIndexSet)
			mat.refractIndex = REFRACTINDEX;
		if (!mat.reflectanceSet)
			mat.reflectance = REFLECTANCE;
		if (!mat.transparitySet)
			mat.transparity = TRANSPARITY;
		if (!mat.roughnessSet)
			mat.roughness = ROUGHNESS;
	}

protected:
	Material mat;
	int type = -1;

};


class SDFGroup : public SDFObject {


public:
	// bind k to the sum function. no need to store it elsewhere
	SDFGroup(std::vector<SDFObject*> contents, std::function<double(double, double, double)> sumFun, double k = 0) :
		contents(contents), 
		sdfSumFun(std::bind(sumFun, std::placeholders::_1, std::placeholders::_2, k))
	{
		type = GROUP;
		orig = Vec3d(0);
	}
	// alternatively if k has been taken care of elsewhere
	SDFGroup(std::vector<SDFObject*> contents, std::function<double(double, double)> sumFun) :
		contents(contents),
		sdfSumFun(sumFun)
	{
		type = GROUP;
		orig = Vec3d(0);
	}
	virtual ~SDFGroup()
	{
		for (auto iter = contents.begin(); iter < contents.end(); iter++)
			delete* iter;
	}
	SDFGroup* clone()  const {
		std::vector<SDFObject*> nc;
		for (auto c : contents)
		{
			nc.push_back(c->clone());
		}
		SDFGroup* no =  new SDFGroup(nc, sdfSumFun);
		no->orig = orig;
		no->mat = Material(mat);
		no->rot = rot;
		no->type = type;
		return no;
	}

	double sdfFun(const Vec3d& iPnt) const;
	void push_back(SDFObject* newobj) { 
		contents.push_back(newobj);
	}

	Color getColor(const Vec3d& pos) const;
	double getRefractIndex(const Vec3d& pos) const;
	double getReflectance(const Vec3d& pos) const;
	double getTransparity(const Vec3d& pos) const;
	double getRoughness(const Vec3d& pos) const;
	const Material& getMaterial(const Vec3d& pos) const;

	void setMemberMaterials(); // this one is just to update the correct materials for members

private:
	std::vector<SDFObject*> contents;
	std::function<double(double, double)> sdfSumFun;
};



class Ellipsoid : public SDFObject {

public:
	Ellipsoid() : r(0) { type = OBJECT;  };
	Ellipsoid(const Vec3d& r) : r(r) { type = OBJECT; };
	virtual ~Ellipsoid() {};
	// radius to different directions
	Vec3d r;

	double sdfFun(const Vec3d& iPnt) const;

	Ellipsoid* clone() const {
		return new Ellipsoid(*this);
	}
};


class Plane : public SDFObject {


public:
	Plane() : n(0,0,1) { type = OBJECT; };
	virtual ~Plane() {};
	//normal vector
	Vec3d n;
	//double h;

	double sdfFun(const Vec3d& iPnt) const;

	Plane* clone() const {
		return new Plane(*this);
	}
};


class Box : public SDFObject {

public:
	Box() : softFactor(0), s(0) { type = OBJECT; };
	virtual ~Box() {};
	double softFactor;
	Vec3d s;

	double sdfFun(const Vec3d& iPnt) const;

	Box* clone() const {
		return new Box(*this);
	}
};


class Torus : public SDFObject {

public:
	Torus() : t(0) { type = OBJECT; };
	virtual ~Torus() {};
	Vec3d t;

	double sdfFun(const Vec3d& iPnt) const;

	Torus* clone() const {
		return new Torus(*this);
	}
};


class InfCone : public SDFObject {

public:
	InfCone() : c(0) { type = OBJECT; };
	virtual ~InfCone() {};
	Vec3d c;

	double sdfFun(const Vec3d& iPnt) const;

	InfCone* clone() const {
		return new InfCone(*this);
	}
};

class Cone : public SDFObject {

public:
	Cone() : c(0), h(0) { type = OBJECT; };
	virtual ~Cone() {};
	Vec3d c;
	double h;

	double sdfFun(const Vec3d& iPnt) const;

	Cone* clone() const {
		return new Cone(*this);
	}
};


class Line : public SDFObject {

public:
	Line() : a(0), b(0), r(0) { type = OBJECT; };
	virtual ~Line() {};
	Vec3d a;
	Vec3d b;
	double r;

	double sdfFun(const Vec3d& iPnt) const;

	Line* clone() const {
		return new Line(*this);
	}
};


class Cylinder : public SDFObject {

public:
	Cylinder() : h(0), r(0) { type = OBJECT; };
	virtual ~Cylinder() {};
	double h;
	double r;

	double sdfFun(const Vec3d& iPnt) const;

	Cylinder* clone() const {
		return new Cylinder(*this);
	}
};


// lights
class Light
{
public:

	Light() :
		intensity(1.0), specIntensity(1.0), diffColor(Color(1.0)), specColor(Color(1.0))
	{}
	Light(double intensity, Color c) :
		intensity(intensity), specIntensity(1.0), diffColor(c), specColor(c)
	{}
	Light(const std::string& descr) :
		intensity(1.0), specIntensity(1.0)
	{
		if (descr == "white")
		{
			diffColor = Color(1.0); specColor = Color(1.0);
		}
		else if (descr == "warm")
		{
			diffColor = Color(1.0, 0.9, 0.6); specColor = Color(1.0, 1.0, 0.8);
		}
		else if (descr == "cold")
		{
			diffColor = Color(0.8, 0.8, 1.0); specColor = Color(0.9, 0.9, 1.0);
		}
	}
	virtual ~Light() {};

	virtual void setIntensity(double a) = 0;
	virtual void setLightColor(const Color& c) = 0;
	virtual Light* clone() const = 0;

	Color getLightColor() const {
		Color a = diffColor * intensity;
		return a;
	}
	Color getSpecColor() const {
		Color a = specColor * specIntensity;
		return a;
	}

	// these assume implementations of light also inherit from some sdfobject
	virtual double getSize() const = 0;
	virtual Vec3d getOrig() const = 0;

protected:
	double intensity;
	double specIntensity;
	Color diffColor;
	Color specColor;
};


class RoundLight : public Light, public Ellipsoid 
{

public:
	RoundLight() :
		Ellipsoid(), Light()
	{
		type = LIGHT;
	}
	RoundLight(double inte) :
		Ellipsoid(), Light()
	{
		intensity = inte;
		specIntensity = inte;
		type = LIGHT;
	}
	RoundLight(double inte, Color c) :
		Ellipsoid(), Light(inte, c)
	{
		intensity = inte;
		specIntensity = inte;
		type = LIGHT;
	}
	RoundLight(const RoundLight& other) :
		Ellipsoid(other.r), Light(other.intensity, other.diffColor)
	{
		type = LIGHT;
		mat = Material(other.mat);
	}
	virtual ~RoundLight() {};

	virtual void setIntensity(double a) {
		intensity = a;
	}

	virtual void setLightColor(const Color& c)
	{
		diffColor = Color(c);
		specColor = Color(c);
		setColor(c / c.getMaxVal());
	}

	double getSize() const
	{
		return (r.x + r.y + r.z) / 3.0;
	}
	
	Vec3d getOrig() const
	{
		return orig;
	}

	RoundLight* clone() const
	{
		return new RoundLight(*this);
	}


};