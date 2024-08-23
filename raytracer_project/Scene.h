#pragma once

#include <vector>
#include <memory>
#include <utility>
#include <functional>
#include <string>
#include "sdfobject.h"
#include "vectormath.h"
#include <iostream>


class Screen
{
public:
	Screen():
		w(0), h(0), pixelSize(0), screenDist(0)
	{};
	Screen(int w, int h, Vec3d campos, Vec3d camdir, double pixSize, double screenDist)
		: h(h), w(w), pixelSize(pixSize), cameraPos(campos), cameraDir(camdir), 
			screenDist(screenDist)
	{
		// initializes the size of the container
		std::vector<Vec3d> temp((size_t)h*w, Vec3d(0,0,0));
		pixelLocations = temp;
		screenCenter = cameraPos + screenDist * cameraDir;
		computePixelLocations(screenCenter, cameraDir);
	}
	Screen(const Screen& other)
		: h(other.h), w(other.w), pixelSize(other.pixelSize), cameraPos(other.cameraPos), 
		cameraDir(other.cameraDir), screenDist(other.screenDist), pixelLocations(other.pixelLocations),
		screenUp(other.screenUp), screenLeft(other.screenLeft), screenCenter(other.screenCenter)
	{}

	const std::vector<Vec3d>& getPixelLocations() const { return pixelLocations; };
	const Vec3d& getPixelLocation(int index) const { return pixelLocations[index]; }
	std::pair<int, int> getScreenSize() const { return std::pair<int, int>(w, h); }
	const Vec3d& getCamPos() const { return cameraPos; }
	const Vec3d& getCamDir() const { return cameraDir; }
	double getScreenDist() const { return screenDist; }
	double getPixelSize() const { return pixelSize; }
	const Vec3d& getScreenLeft() const { return screenLeft; }
	const Vec3d& getScreenUp() const { return screenUp; }
	const Vec3d& getScreenCenter() const { return screenCenter; }

	void computePixelLocations(const Vec3d& pos, const Vec3d& dir);
	void setCamPos(Vec3d& campos);
	void setCamDir(Vec3d& camdir);
	void setScreenDist(double sd);

private:
	std::vector<Vec3d> pixelLocations;
	int h;
	int w;
	Vec3d cameraPos;
	Vec3d cameraDir;
	Vec3d screenLeft;
	Vec3d screenUp;
	Vec3d screenCenter;
	
	double pixelSize;
	double screenDist;

};

class Scene
{

public:
	Scene(Vec3d campos, Vec3d camdir, int sw, int sh, double sd) 
		: screen(sw, sh, campos, camdir, 0.03, sd)
	{
		
	}
	
	Scene(const Scene& other)
		: screen(other.screen), bgCol(other.bgCol)
	{
		std::vector<SDFObject*> nobs;
		for (auto c : other.objects)
		{
			nobs.push_back(c->clone());
		}
		objects = nobs;

		std::vector<Light*> nlights;
		for (auto l : other.lights)
		{
			nlights.push_back(dynamic_cast<Light*>(l->clone()));
		}
		lights = nlights;
	}

	Scene()
	{
		// this is used to create empty scene to fill from file
	}

	~Scene() //TODO check this destructor
	{
		for (auto iter = objects.begin(); iter < objects.end(); iter++)
		{
			delete* iter;
		}
		objects.clear();
		lights.clear(); // objects should include pointers to all ligths
	}


	// these just access member object screen
	const Vec3d& getCamPos() const { return screen.getCamPos(); }
	const Vec3d& getCamDir() const { return screen.getCamDir(); }
	double getScreenDist() const { return screen.getScreenDist(); }
	double getPixelSize() const { return screen.getPixelSize(); }
	const Vec3d& getScreenLeft() const { return screen.getScreenLeft(); }
	const Vec3d& getScreenUp() const { return screen.getScreenUp(); }
	const Vec3d& getScreenCenter() const { return screen.getScreenCenter(); }
	std::pair<int, int> getScreenSize() const { return screen.getScreenSize(); }
	const std::vector<Vec3d>& getPixelLocations() const { return screen.getPixelLocations(); }
	const Vec3d& getPixelLocation(int index) const { return screen.getPixelLocation(index); }

	void createScreen(int w, int h, Vec3d cp, Vec3d cd, double ps, double sd) { screen = Screen(w, h, cp, cd, ps, sd); }
	void setCamPos(Vec3d& campos) { screen.setCamPos(campos); }
	void setCamDir(Vec3d& camdir) { screen.setCamDir(camdir); }
	void setScreenDist(double sd) { screen.setScreenDist(sd); }

	const std::vector<SDFObject*>& getObjects() const { return objects; }
	const std::vector<Light*>& getLights() const { return lights; }
	const Vec3d& getBgCol() const { return bgCol; }

	std::stringstream serialize() const;

	void pushObject(SDFObject* o) { objects.push_back(o); }
	void pushLight(Light* l) { lights.push_back(l); }
	void setBgCol(Color bc) { bgCol = bc; }

private:
	Screen screen;
	Vec3d bgCol;

	std::vector<SDFObject*> objects;
	std::vector<Light*> lights;
};

