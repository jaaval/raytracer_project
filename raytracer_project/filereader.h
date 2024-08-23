#pragma once

#include "Scene.h"
#include "sdfobject.h"
#include <string>
#include <vector>
#include <map>


bool createSceneFromFile(Scene& scene, const std::string filename);

std::vector<std::string> strSplit(const std::string&, const char delim);
void strReplaceAll(std::string& str, const std::string& from, const std::string& to);


SDFGroup* parseGroup(const std::map<std::string, std::string> data);
SDFObject* parseObject(const std::string& type, const std::map<std::string, std::string> data);
Light* parseLight(const std::string& type, const std::map<std::string, std::string> data);

bool getNextOperation(std::pair<std::string, std::string>&, std::string&);
std::vector<std::string> strSplitGroup(const std::string& s, const char delim);

std::vector<SDFObject*> parseCommands(Scene& scene, std::string& buffer);

void parseMaterial(const std::map<std::string, std::string> data, SDFObject* obj);
void parseGeometry(const std::map<std::string, std::string> data, SDFObject* obj);

std::vector<double> parseDArray(std::string input);