
#include "ray.h"
#include "Material.h"
#include <iostream>
#include "constants.h"
#include "utilmath.h"


void Ray::setDirection2point(const Vec3d& pixelLoc)
{
	direction = pixelLoc - current_p;
	direction.normalize();
}

void Ray::setDirection(Vec3d& dirvec)
{
	direction = dirvec;
	direction.normalize();
}





// this is the main raymarching function. Loops computeStepLength and take step
// needs the scene as input TODO
Color Ray::march(const Scene& scene)
{
	Color output(0);

	const std::vector<SDFObject*> objects = scene.getObjects();
	
	std::vector<Light*> lights = scene.getLights();

	int iters = 0;
	double eps = HITEPS;
	Intersect ri;
	double d;

	// actually march the steps
	d = 1e10;
	
	while (d > eps)
	{
		// if not hit anything
		if (++iters > ABSMAXITER || totalDist > DISTLIMIT)
		{
			return scene.getBgCol(); // return scene background color
		}
		ri = computeStepLength(objects, current_p);
		d = std::abs(ri.d);
		takeStep(d);
		totalDist += d;
		eps = HITEPS * 0.5 * totalDist; // reduce accuracy the further we are 
	}

	// object that was hit
	SDFObject* hitObject = ri.object;

	// material color
	const Color mate = hitObject->getColor(current_p);

	int hittype = hitObject->getType();
	// if light
	if (hittype == LIGHT)
		return mate;
	
	// compute surface normal 
	Vec3d normal = hitObject->computeNormal(current_p);

	// check if we are inside an object and coming out of there
	if (ri.d < 0)
	{
		// run one sdf from the other side of boundary
		Intersect testRi = computeStepLength(objects, current_p + 2*eps*normal);
		if (testRi.d > 0)
		{
			hittype = AIR;
		}
	}

	double objReflectance, objTransparity, objRefractIndex, objRoughness;
	Color refraction(0); 
	Color reflection(0);

	// if not coming out of transparent object
	if (hittype != AIR)
	{
		// get reflective properties etc
		objReflectance = hitObject->getReflectance(current_p);
		objTransparity = hitObject->getTransparity(current_p);
		objRefractIndex = hitObject->getRefractIndex(current_p);
		objRoughness = hitObject->getRoughness(current_p) / 5.0;
		if (objReflectance + objTransparity > 1.0) // normalize in case of transparent reflective surfaces
		{
			objReflectance /= objReflectance + objTransparity;
			objTransparity /= objReflectance + objTransparity;
		}
		double specAlpha = clamp(1000.0 * pow((1.0 - objReflectance), 2.0), 0.1, 1000.0); // TODO this needs tuning

		if (objRoughness > 0)
		{
			Vec3d randomVec = Vec3d(normalRand(0, objRoughness), normalRand(0, objRoughness), normalRand(0, objRoughness));
			normal += randomVec;
			normal.normalize();
		}

		// add some ambient lighting 
		output += K_AMBI;

		Color diffusion;
		double shadowFactor;
		Color diffLight;
		Color specLight(0);

		// loop through lights on scene
		for (auto iter = lights.begin(); iter < lights.end(); iter++)
		{
			Color difCol = (*iter)->getLightColor();
			Color specCol = (*iter)->getSpecColor();

			// compute diffuse lighting
			Vec3d lightDir = (*iter)->getOrig() - current_p;
			double lightDist = lightDir.length();
			lightDir.normalize();
			diffusion = K_DIFF * dot(normal, lightDir) * difCol;
			diffLight = diffusion * mate;

			// compute shadowing factor
			if (dot(lightDir, normal) < 0)
				shadowFactor = 0.0; // if covered by the object itself
			else
			{
				Ray lray = Ray(current_p, lightDir, 1.0, recursion + 1);
				double lightSize = (*iter)->getSize();
				shadowFactor = lray.softshadow(objects, lightDist, SOFTSHADOWCONSTANT / lightSize); // the last value tunes softness
			}
			// compute specular lighting
			if (objReflectance > 0.0 && shadowFactor > 0.5)
			{
				Vec3d mirroredDir = mirrorByNormal(-lightDir, normal);
				specLight += K_SPEC * std::max(0.0, pow(dot(-direction, mirroredDir), specAlpha)) * specCol * shadowFactor;
			}

			// add everything to output
			output += diffLight * shadowFactor;
		}

		// compute refractions
		if (recursion < MAXRECURSION && objTransparity > 0.0 && rayStr * objTransparity > 0.05)
		{
			double fr = computeFresnellReflectance(normal, direction, currentRefractIndex, objRefractIndex);
			objReflectance = clamp(objReflectance + (1.0 - objReflectance) * fr, 0.0, 1.0);
			if (objReflectance + objTransparity > 1.0) // normalize again
			{
				objReflectance /= objReflectance + objTransparity;
				objTransparity /= objReflectance + objTransparity;
			}
			Vec3d refractDir = computeRefractDir(direction, -normal, currentRefractIndex, objRefractIndex);
			refractDir.normalize();

			Ray refractRay(current_p + eps * direction - eps * normal + eps * refractDir, refractDir, rayStr * objTransparity, recursion + 1);
			refractRay.setCurrentRefractIndex(objRefractIndex);
			refraction = refractRay.march(scene);
		}
	
		// compute reflections by iteratively casting rays
		if (recursion < MAXRECURSION && objReflectance > 0.0 && rayStr * objReflectance > 0.05)
		{
			// compute reflect direction
			Vec3d reflectDir = mirrorByNormal(direction, normal);
			reflectDir.normalize();
			Ray reflectRay = Ray(current_p + eps * direction + eps * reflectDir, reflectDir, rayStr*objReflectance, recursion + 1);
			reflection = reflectRay.march(scene);
		}

		// add the reflections and refractions

		output = (1.0 - objReflectance - objTransparity) * output + objReflectance * reflection + objTransparity * refraction;

		// add specular reflection at the end TODO adjust the factor
		output += specLight * clamp(objReflectance + 0.5, 0.0, 1.0);
	}
	else if (recursion < MAXRECURSION) // (hittype == AIR) if coming out of transparent object
	{
		objTransparity = 1.0;
		objRefractIndex = 1.0;
		double fr = computeFresnellReflectance(normal, direction, currentRefractIndex, objRefractIndex);
		objReflectance = clamp(fr, 0.0, 1.0);
		objTransparity -= objReflectance;

		if (objTransparity > 0.01 && rayStr * objTransparity > 0.01)
		{
			Vec3d refractDir = computeRefractDir(direction, normal, currentRefractIndex, objRefractIndex);
			if (!std::isnan(refractDir.x))
			{
				refractDir.normalize();
				Ray refractRay(current_p + eps * direction + eps * normal + eps * refractDir, refractDir, rayStr * objTransparity, recursion + 1);
				refractRay.setCurrentRefractIndex(objRefractIndex);
				refraction = refractRay.march(scene);
			}
			else
			{
				//std::cout << "NaN!" << std::endl;
				objTransparity = 0.0; objReflectance += objTransparity;
			}
		}

		if (objReflectance > 0.01 && rayStr * objReflectance > 0.01)
		{
			Vec3d reflectDir = mirrorByNormal(direction, normal);
			reflectDir.normalize();
			Ray reflectRay = Ray(current_p + eps * direction + eps * reflectDir, reflectDir, rayStr * objReflectance, recursion + 1);
			reflectRay.setCurrentRefractIndex(currentRefractIndex);
			reflection = reflectRay.march(scene);
		}
		
		output = (1.0 - objReflectance - objTransparity) * output + objReflectance * reflection + objTransparity * refraction;
	}

	// todo that ligthing correction some of the sdf articles
	return output;
}




double Ray::softshadow(const std::vector<SDFObject*>& objects, double distance, double k)
{
	double res = 1.0;
	double ph = 1e20; // big number
	double y, d;
	double h = MINT;
	double t = MINT;
	Intersect ri;
	Vec3d pos(current_p);

	double transparity, restr = 0; // these are modifications for transparent stuff
	double temp;

	while (t < distance - MINT)
	{
		pos += h * direction;
		ri = computeStepLength(objects, pos);
		h = ri.d;
		
		transparity = ri.object->getTransparity(current_p) * 0.95;
		if (h < HITEPS)
		{
			if (ri.object->getType() == LIGHT)
				break;
			if (transparity < 0.3)
				return 0.0;
			h = 1.0; 
			continue;
		}

		if (ri.object->getType() == LIGHT)
		{
			t += h;
			continue;
		}
			
		y = h * h / (2.0 * ph);
		d = sqrt(h*h-y*y);

		temp = k * d / std::max(HITEPS, t - y);
		if (temp < res)
		{
			res = temp;
			restr = transparity;
		}
		ph = h;
		t += h;
	}
	return clamp(res, 0.0, 1.0);
}


// -------------------------------------------------

// computes SDF and returns the distance
Intersect Ray::computeStepLength(const std::vector<SDFObject*>& objects, const Vec3d& pos) const
{
	double mind = 1000000000000;
	double d;
	int i = 0;
	SDFObject* o = nullptr;
	// takes minimum of the objects in the objects vector so the end result is a union of the objects
	// returns also the object index so that surface normal computation can be done for just that one
	for (auto iter = objects.begin(); iter < objects.end(); iter++, i++)
	{
		d = (*iter)->sdfFun(pos);
		// a bit convoluted, tries to infer if transitioning from inside one object to another
		if (std::abs(d) < std::abs(mind) || (std::abs(d) == std::abs(mind) && mind < d)) {
			mind = d;
			o = *iter;
		}
	}
	return Intersect(o, mind, pos);
}


// does the marching without moving any ray object
Intersect Ray::castRay(const std::vector<SDFObject*>& objects, const Vec3d& startPos, const Vec3d& dir) const
{
	int iters = 0;
	double eps = HITEPS;
	Intersect ri;
	double d;
	double td = 0;

	// actually march the steps
	d = 1e10;
	Vec3d pos(startPos);

	while (d > eps)
	{
		// if not hit anything
		if (++iters > ABSMAXITER || totalDist > DISTLIMIT)
		{
			Intersect nullri; // has nullptr as object
			return nullri;
		}
		ri = computeStepLength(objects, pos);
		d = std::abs(ri.d);
		pos += d * dir;
		td += d;
		eps = HITEPS * td; // reduce accuracy the further we are 
	}
	return ri;
}



double computeFresnellReflectance(const Vec3d& normal, const Vec3d& dir, double n1, double n2)
{
	double R, Ro, cosTheta, c5;
	cosTheta = 1. - std::abs(dot(normal, dir));
	c5 = cosTheta * cosTheta * cosTheta * cosTheta * cosTheta;
	Ro = (n1-n2) / (n1+n2);
	Ro *= Ro;
	R = Ro + (1. - Ro) * c5; // schlick's approximation for fresnell equations
	return R;
}