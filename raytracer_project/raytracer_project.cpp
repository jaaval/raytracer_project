// raytracer_project.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include <iostream>
#include <sstream>
#include <fstream>

#include "Renderer.h"
#include <string>
#include <memory>

#include "MinWindow.h"
#include "filereader.h"
#include <utility>
#include <vector>


int main()
{

    int numThreads = 24;

    Scene mainScene;
    bool success = createSceneFromFile(mainScene, 
        "C:\\Users\\jaakk\\source\\repos\\raytracer_project\\raytracer_project\\data\\testiscene.txt");
    renderImage(mainScene, numThreads);
    /*
    int index = 60 * 240 + 120;
    const Vec3d dir = mainScene.getPixelLocation(index);
    Ray a(Vec3d(0), 1.0);
    a.setDirection2point(dir);
    std::vector<SDFObject*> sceneobjects = mainScene.getObjects();
    std::vector<SDFObject*> objects(sceneobjects);
    Color col = a.march(mainScene, objects);
    */

    exit(EXIT_SUCCESS);
}


