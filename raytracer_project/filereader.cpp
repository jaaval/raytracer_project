#include "filereader.h"
#include <stdlib.h>
#include <deque>
#include <iostream>
#include <sstream>
#include <fstream>
#include "constants.h"
#include <memory>


bool createSceneFromFile(Scene& scene, const std::string filename)
{// todo add try catch

	std::cout << "We are here trying to open file " << filename << std::endl;

	std::ifstream infile;
	infile.open(filename);
	if (!infile.is_open())
	{
		std::cout << "unable to open file " << filename << std::endl;
		return false;
	}

	std::stringstream sbuffer;
	std::string buffer;
	std::string line;
	std::pair<std::string, std::string> command;
	

	std::vector<std::string> splitted;

	int stackIndentLevel = 0;
	int currentIndentLevel = 0;
	std::vector<SDFObject*> objects;

	while (std::getline(infile, line))
	{
		if (line.size() == 0) // empty line
			continue;
		if (line[0] == '#') // commented out
			continue;
		sbuffer << line;
	}
	buffer = sbuffer.str(); // buffer now holds the entire file minus commented lines
	strReplaceAll(buffer, " ", ""); // removes spaces
	strReplaceAll(buffer, "\t", ""); // removes tabs

	objects = parseCommands(scene, buffer);
	for (auto o : objects)
	{
		scene.pushObject(o);
	}

	infile.close();
	return true;
}


std::vector<SDFObject*> parseCommands(Scene& scene, std::string& buffer)
{
	std::map<std::string, std::string> cdata;
	std::string key;
	std::string cont;
	std::pair<std::string, std::string> command;
	std::vector<std::string> splitted;
	std::vector<SDFObject*> objects;
	std::vector<double> tempd;

	while (getNextOperation(command, buffer))
	{
		splitted = strSplitGroup(command.second, ',');
		for (auto s : splitted)
		{
			size_t sc = s.find(':');
			
			key = s.substr(0, sc);
			cont = s.substr(sc + 1);
			cdata[key] = cont;
		}
		if (command.first == "group")
		{
			SDFGroup* newgroup = parseGroup(cdata);
			std::string memberdata = cdata.at("members");
			// remove square brackets at ends
			if (memberdata.front() == '[')
				memberdata.erase(memberdata.begin());
			if (memberdata.back() == ']')
				memberdata.pop_back();
			// recursive call for group contents
			std::vector<SDFObject*> temp = parseCommands(scene, memberdata);
			for (auto obj : temp) {
				newgroup->push_back(obj);
			}
			newgroup->setMemberMaterials();
			objects.push_back(newgroup);
		}
		else if (command.first == "screen")
		{
			double w = stoi(cdata.at("width"));
			double h = stoi(cdata.at("height"));
			tempd = parseDArray(cdata.at("cameraPosition"));
			Vec3d campos(tempd[0], tempd[1], tempd[2]);
			tempd = parseDArray(cdata.at("cameraDirection"));
			Vec3d camdir(tempd[0], tempd[1], tempd[2]);
			double pixsize = stod(cdata.at("pixelSize"));
			double sdist = stod(cdata.at("screenDistance"));
			scene.createScreen(w, h, campos, camdir, pixsize, sdist);
		}
		else if (command.first == "bgcol")
		{
			tempd = parseDArray(cdata.at("color"));
			scene.setBgCol(Color(tempd[0], tempd[1], tempd[2]));
		}
		else if (command.first == "roundlight")
		{
			Light* newlight = parseLight(command.first, cdata);
			scene.pushLight(newlight);
			objects.push_back(dynamic_cast<RoundLight*>(newlight));
			
		}
		else
		{
			SDFObject* newobject = parseObject(command.first, cdata);
			objects.push_back(newobject);
		}


		cdata.clear();
	}
	return objects;
}



SDFGroup* parseGroup(const std::map<std::string, std::string> data)
{
	SDFGroup* ng;
	double k = 0.0;
	std::string type = "";
	if (data.count("k"))
		k = stod(data.at("k"));
	if (data.count("type"))
		type = data.at("type");
	std::vector<SDFObject*> contents;
	if (type == "union")
		ng = new SDFGroup(contents, &SDFUnion, k);
	else if (type == "subtract")
		ng = new SDFGroup(contents, &SDFSubtract, k);
	else if (type == "intersect")
		ng = new SDFGroup(contents, &SDFIntersect, k);
	else if (type == "blend")
		ng = new SDFGroup(contents, &SDFBlend, k);
	else
		ng = new SDFGroup(contents, &SDFUnion, k);
	parseMaterial(data, ng);
	parseGeometry(data, ng);
	return ng;
}


SDFObject* parseObject(const std::string& type, const std::map<std::string, std::string> data)
{
	std::vector<double> vals;
	SDFObject* obj;
	if (type == "ellipsoid")
	{
		Ellipsoid* a = new Ellipsoid();
		vals = parseDArray(data.at("r"));
		if (vals.size() > 1)
			a->r = Vec3d(vals[0], vals[1], vals[2]);
		else
			a->r = Vec3d(vals[0]);
		obj = a;
	}
	else if (type == "box")
	{
		Box* a = new Box();
		vals = parseDArray(data.at("s"));
		a->s = Vec3d(vals[0], vals[1], vals[2]);
		obj = a;
	}
	else if (type == "plane")
	{
		Plane* a = new Plane();
		vals = parseDArray(data.at("normal"));
		a->n = Vec3d(vals[0], vals[1], vals[2]);
		obj = a;
	}
	else if (type == "torus")
	{
		Torus* a = new Torus();
		vals = parseDArray(data.at("t"));
		a->t = Vec3d(vals[0], vals[1], 0.0);
		obj = a;
	}
	else if (type == "cone")
	{
		if (data.count("h")) //finite
		{
			Cone* a = new Cone();
			vals = parseDArray(data.at("c"));
			a->c = Vec3d(vals[0], vals[1], 0.0);
			a->h = stod(data.at("h"));
			obj = a;
		}
		else // infinite
		{
			InfCone* a = new InfCone();
			vals = parseDArray(data.at("c"));
			a->c = Vec3d(vals[0], vals[1], 0.0);
			obj = a;
		}
	}
	else if (type == "line")
	{
		Line* a = new Line();
		vals = parseDArray(data.at("a"));
		a->a = Vec3d(vals[0], vals[1], vals[2]);
		vals = parseDArray(data.at("b"));
		a->b = Vec3d(vals[0], vals[1], vals[2]);
		obj = a;
	}
	else if (type == "cylinder")
	{
		Cylinder* a = new Cylinder();
		a->h = stod(data.at("h"));
		a->r = stod(data.at("r"));
		obj = a;
	}
	else
	{
		std::cout << "INVALID OBJECT TYPE: " << type << std::endl;
		return nullptr;
	}

	parseMaterial(data, obj);
	parseGeometry(data, obj);
	return obj;
}


Light* parseLight(const std::string& type, const std::map<std::string, std::string> data)
{
	double intensity = 1.0;
	if (data.count("intensity"))
		intensity = stod(data.at("intensity"));
	std::vector<double> vals;
	if (type == "roundlight")
	{
		RoundLight* a = new RoundLight();
		vals = parseDArray(data.at("color"));
		a->setLightColor(Color(vals[0], vals[1], vals[2]));
		a->setIntensity(intensity);
		vals = parseDArray(data.at("r"));
		a->r = Vec3d(vals[0], vals[1], vals[2]);
		parseGeometry(data, a);
		return a;
	}
}


// adds possible material properties to data. TODO add default values to constants
void parseMaterial(const std::map<std::string, std::string> data, SDFObject* obj)
{
	if (data.count("color"))
	{
		std::vector<double> temp = parseDArray(data.at("color"));
		obj->setColor(Color(temp[0], temp[1], temp[2]));
	}
	if (data.count("refractIndex"))
	{
		obj->setRefractIndex(stod(data.at("refractIndex")));
	}
	if (data.count("reflectance"))
	{
		obj->setReflectance(stod(data.at("reflectance")));
	}
	if (data.count("transparity"))
	{
		obj->setTransparity(stod(data.at("transparity")));
	}
	if (data.count("roughness"))
	{
		obj->setRoughness(stod(data.at("roughness")));
	}
	obj->setDefaultMaterial(); //sets whatever was not already set to default
}


void parseGeometry(const std::map<std::string, std::string> data, SDFObject* obj)
{
	if (data.count("rotation"))
	{
		std::string rottype = "xyz";
		if (data.count("rotationOrder"))
			rottype = data.at("rotationOrder");

		std::vector<double> temp = parseDArray(data.at("rotation"));
		Vec3d rot(temp[0], temp[1], temp[2]);
		rot /= 360.0;
		rot *= 2.0 * PI;
		RotMat rm;
		if (rottype == "xyz")
			rm = rotXYZ(rot);
		if (rottype == "xzy")
			rm = rotXZY(rot);
		if (rottype == "yxz")
			rm = rotYXZ(rot);
		if (rottype == "yzx")
			rm = rotYZX(rot);
		if (rottype == "zxy")
			rm = rotZXY(rot);
		if (rottype == "zyx")
			rm = rotZYX(rot);
		obj->rot = rm;
	}
	if (data.count("orig"))
	{
		std::vector<double> vals;
		vals = parseDArray(data.at("orig"));
		obj->setOrigin(Vec3d(vals[0], vals[1], vals[2]));
	}
	
}


// doubles from string of comma separated numbers
std::vector<double> parseDArray(std::string input)
{
	strReplaceAll(input, "[", "");
	strReplaceAll(input, "]", "");
	std::vector<std::string> vals = strSplit(input, ',');
	std::vector<double> a;
	for (auto v : vals) a.push_back(stod(v));
	return a;
}


// string operations ----------------------------------------

bool getNextOperation(std::pair<std::string, std::string>& sv, std::string& s)
{
	if (s.size() == 0)
		return false;
	const size_t start = s.find('(');
	if (start == std::string::npos)
		return false;

	sv.first = s.substr(0, start);
	strReplaceAll(sv.first, ",", ""); // this is an ugly fix to bad file format design
	int count = 1;
	size_t end = s.size();
	for (size_t i = start + 1; i < s.size(); i++)
	{
		char c = s[i];
		if (c == '(')
			count++;
		else if (c == ')')
			count--;
		if (count == 0)
		{
			end = i;
			break;
		}
	}
	sv.second = s.substr(start + 1, end - start - 1);
	std::string newstring = s.substr(end + 1);
	s.swap(newstring);
	return true;
}


std::vector<std::string> strSplit(const std::string& s, const char delim)
{
	std::vector<std::string> sv;
	std::string token;
	std::istringstream is(s);
	while (std::getline(is, token, delim))
	{
		sv.push_back(token);
	}
	return sv;
}


std::vector<std::string> strSplitGroup(const std::string& s, const char delim)
{
	std::vector<std::string> sv;
	std::string val;
	size_t start = 0;
	size_t prev = 0;
	size_t end = s.size();
	size_t a, b;
	int count = 0;
	while (start < end)
	{
		if (count > 0)
		{
			a = s.find(']', start);
			b = s.find('[', start);
			if (a < b)
				count--;
			else
				count++;
			start = std::min(a, b) + 1;
			continue;
		}
		else
		{
			a = s.find(',', start);
			b = s.find('[', start);
			if (a == std::string::npos)
				break;
			if (b < a)
			{
				count++;
				start = b + 1;
				continue;
			}
			else
			{
				sv.push_back(s.substr(prev, a - prev));
				prev = a + 1;
				start = a + 1;
			}
		}
	}
	sv.push_back(s.substr(prev));
	return sv;
}


void strReplaceAll(std::string& source, const std::string& from, const std::string& to)
{
	std::string newString;
	newString.reserve(source.length());
	std::string::size_type lastPos = 0;
	std::string::size_type findPos;
	while (std::string::npos != (findPos = source.find(from, lastPos)))
	{
		newString.append(source, lastPos, findPos - lastPos);
		newString += to;
		lastPos = findPos + from.length();
	}
	newString += source.substr(lastPos);
	source.swap(newString);
}