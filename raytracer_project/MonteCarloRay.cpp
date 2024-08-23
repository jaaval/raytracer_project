
#include "ray.h"
#include "Material.h"
#include <iostream>
#include "constants.h"
#include "utilmath.h"

Color Ray::monteCarloMarch(const Scene& scene)
{
	Color output;

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
	Color mate = hitObject->getColor(current_p);

	// compute surface normal 
	Vec3d normal = hitObject->computeNormal(current_p);

	int hittype = hitObject->getType();
	// check if we are inside an object and coming out of there
	if (ri.d < 0)
	{
		// run one sdf from the other side of boundary
		Intersect testRi = computeStepLength(objects, current_p + 1.1 * eps * normal);
		if (testRi.d > 0)
		{
			hittype = AIR;
		}
	}

	double objReflectance, objTransparity, objRefractIndex;

	if (hittype != AIR)
	{

	}
	// TODO
	return output;
}