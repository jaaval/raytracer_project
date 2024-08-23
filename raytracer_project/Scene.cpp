#include "Scene.h"
#include <iostream>
#include <sstream>


void Screen::computePixelLocations(const Vec3d& pos, const Vec3d& dir)
{
	int numPix = w * h;

	// compute projection to horizontal plane
	Vec3d hProj(dir);
	hProj.z = 0;
	// rotate 90 degrees left and normalize
	double angle = 3.141593 / 2.0;
	Vec3d left;
	left.x = hProj.x * cos(angle) - hProj.y * sin(angle);
	left.y = hProj.x * sin(angle) + hProj.y * cos(angle);
	left.z = 0;
	//if (left.x > 0)
	//	left *= -1;
	left.normalize();
	// find vector that is orthogonal to both
	Vec3d up = cross(dir, left);
	if (up.z < 0)
		up *= -1;
	up.normalize();
	screenLeft = left;
	screenUp = up;

	int index;
	double wf;
	double hf;
	for (int j = 0; j < h; j++)
	{
		hf = (h / 2 - j) * pixelSize;
		for (int i = 0; i < w; i++)
		{
			index = j * w + i;
			wf = (w / 2 - i) * pixelSize;
			Vec3d pix = pos + left * wf + up * hf;
			pixelLocations[index] = pix;
		}
	}
}


void Screen::setCamPos(Vec3d& campos) 
{ 
	cameraPos = campos; 
	Vec3d screenPos = cameraPos + screenDist * cameraDir;
	computePixelLocations(screenPos, cameraDir);
}
void Screen::setCamDir(Vec3d& camdir) 
{ 
	cameraDir = camdir; 
	Vec3d screenPos = cameraPos + screenDist * cameraDir;
	computePixelLocations(screenPos, cameraDir);
}
void Screen::setScreenDist(double sd) 
{ 
	screenDist = sd; 
	Vec3d screenPos = cameraPos + screenDist * cameraDir;
	computePixelLocations(screenPos, cameraDir);
}


std::stringstream Scene::serialize() const
{
	std::stringstream ss;
	for (SDFObject* o : objects)
	{
		ss << o->serialize() << "\n";
	}
	return ss;
}