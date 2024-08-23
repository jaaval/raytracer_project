#include "Material.h"
#include <cmath>
#include <algorithm>


double Material::getRefractAngle(double rInd2, double angle)
{
	double sina2 = sin(angle) * refractIndex / rInd2;
	return asin(sina2);
}


byte Color::red()
{
	double temp = x * 255;
	if (temp >= 255)
		return 255;
	else if (temp <= 0)
		return 0;
	else
		return (byte)temp;
}

byte Color::green()
{
	double temp = y * 255;
	if (temp >= 255)
		return 255;
	else if (temp <= 0)
		return 0;
	else
		return (byte)temp;
}

byte Color::blue()
{
	double temp = z * 255;
	if (temp >= 255)
		return 255;
	else if (temp <= 0)
		return 0;
	else
		return (byte)temp;
}

std::vector<byte> Color::colors() 
{
	std::vector<byte> cols(3);
	cols[0] = this->red();
	cols[1] = this->green();
	cols[2] = this->blue();
	return cols;
}

void Color::clip()
{
	x = std::max(0.0, std::min(x, 1.0));
	y = std::max(0.0, std::min(y, 1.0));
	z = std::max(0.0, std::min(z, 1.0));
}

Color averageColor(std::vector<std::pair<Color, double>> colors)
{
	Color res;
	double sum = 0;
	for (auto iter = colors.begin(); iter < colors.end(); iter++)
	{
		res += iter->first * iter->second;
		sum += iter->second;
	}
	res /= sum;
	res.x = sqrt(res.x);
	res.y = sqrt(res.y);
	res.z = sqrt(res.z);
	res.clip();
	return res;
}