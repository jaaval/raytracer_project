
#include "vectormath.h"
#include "utilmath.h"

double ndot(const Vec3d& a, const Vec3d& b)
{
	return a.x * b.x - a.y * b.y;
}


double dot(const Vec3d& a, const Vec3d& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}



Vec3d Vec3d::operator-(const Vec3d& other) const {
	Vec3d res;
	res.x = x - other.x;
	res.y = y - other.y;
	res.z = z - other.z;
	return res;
}

Vec3d Vec3d::operator-() const {
	Vec3d res;
	res.x = -x;
	res.y = -y;
	res.z = -z;
	return res;
}

Vec3d Vec3d::operator+(const Vec3d& other) const {
	Vec3d res;
	res.x = x + other.x;
	res.y = y + other.y;
	res.z = z + other.z;
	return res;
}

Vec3d& Vec3d::operator+=(const Vec3d& other) {
	x += other.x;
	y += other.y;
	z += other.z;
	return *this;
}

Vec3d& Vec3d::operator-=(const Vec3d& other) {
	x -= other.x;
	y -= other.y;
	z -= other.z;
	return *this;
}


Vec3d Vec3d::operator*(const double other) const {
	Vec3d res;
	res.x = x * other;
	res.y = y * other;
	res.z = z * other;
	return res;
}
Vec3d Vec3d::operator/(const double other) const {
	Vec3d res;
	res.x = x / other;
	res.y = y / other;
	res.z = z / other;
	return res;
}
Vec3d Vec3d::operator*(const Vec3d& other) const {
	Vec3d res;
	res.x = x * other.x;
	res.y = y * other.y;
	res.z = z * other.z;
	return res;
}
Vec3d Vec3d::operator/(const Vec3d& other) const {
	Vec3d res;
	res.x = x / other.x;
	res.y = y / other.y;
	res.z = z / other.z;
	return res;
}


Vec3d operator*(const double s, const Vec3d& vec) {
	return vec * s;
}
Vec3d operator/(const double s, const Vec3d& vec) {
	return vec / s;
}

Vec3d& Vec3d::operator*=(const double other) {
	x *= other;
	y *= other;
	z *= other;
	return *this;
}
Vec3d& Vec3d::operator/=(const double other) {
	x /= other;
	y /= other;
	z /= other;
	return *this;
}


Vec3d Vec3d::max(const Vec3d& other)
{
	Vec3d res;
	res.x = std::max(x, other.x);
	res.y = std::max(y, other.y);
	res.z = std::max(z, other.z);
	return res;
}
Vec3d Vec3d::max(const double a)
{
	Vec3d res;
	res.x = std::max(x, a);
	res.y = std::max(y, a);
	res.z = std::max(z, a);
	return res;
}
Vec3d Vec3d::min(double a)
{
	Vec3d res;
	res.x = std::min(x, a);
	res.y = std::min(y, a);
	res.z = std::min(z, a);
	return res;
}
Vec3d Vec3d::min(const Vec3d& other)
{
	Vec3d res;
	res.x = std::min(x, other.x);
	res.y = std::min(y, other.y);
	res.z = std::min(z, other.z);
	return res;
}
Vec3d Vec3d::abs()
{
	Vec3d res;
	res.x = std::abs(x);
	res.y = std::abs(y);
	res.z = std::abs(z);
	return res;
}



void Vec3d::normalize() {
	double len = this->length();
	if (std::abs(len - 1) < 1e-12)
		return;
	this->x /= len;
	this->y /= len;
	this->z /= len;
}


Vec3d cross(const Vec3d& a, const Vec3d& b)
{
	Vec3d c;
	c.x = a.y * b.z - a.z * b.y;
	c.y = a.z * b.x - a.x * b.z;
	c.z = a.x * b.y - a.y * b.x;
	return c;
}


int32 xy2linear(int32 x, int32 y, int32 sx)
{
	return y * sx + x;
}


Vec3d mirrorByNormal(const Vec3d& input, const Vec3d& normal)
{ // i think this should work regardless of the "sign" of the normal because we multiply with dp
	double dp = dot(input, normal);
	Vec3d output = input - 2 * dp * normal;
	return output;
}


Vec3d computeRefractDir(const Vec3d& input, const Vec3d& normal, double n1, double n2)
{ // vector form of snell's law
	/*
	Vec3d cp = cross(normal, input);
	double dcp = dot(cp, cp);
	double nrat = n1 / n2;
	Vec3d ccp = cross(normal, -cp);
	double temp = sqrt(1.0 - nrat*nrat * dcp);
	return nrat * ccp - normal * temp;
	*/
	
	double ni = dot(normal, input);
	double nrat = n1 / n2;
	double temp = 1.0 - nrat*nrat * (1.0 - ni*ni);
	return sqrt(temp) * normal + nrat * (input - ni * normal);
}


Vec3d elemExp(const Vec3d& a) {
	Vec3d result;
	result.x = exp(a.x);
	result.y = exp(a.y);
	result.z = exp(a.z);
	return result;
}


Vec3d RotMat::apply(const Vec3d& other) const {
	Vec3d result;
	result.x = dot(rmat[0], other);
	result.y = dot(rmat[1], other);
	result.z = dot(rmat[2], other);
	return result;
}

Vec3d RotMat::operator*(const Vec3d& other) const {
	return apply(other);
}


// rotation matrix definitions (Tait-Bryan) ---------------------------------

RotMat rotXYZ(const Vec3d& angles)
{
	double c1, c2, c3, s1, s2, s3;
	c1 = cos(angles.x);
	c2 = cos(angles.y);
	c3 = cos(angles.z);
	s1 = sin(angles.x);
	s2 = sin(angles.y);
	s3 = sin(angles.z);
	Vec3d v1(c2 * c3, -c2 * s3, s2);
	Vec3d v2(c1 * s3 + c3 * s1 * s2, c1 * c3 - s1 * s2 * s3, -c2 * s1);
	Vec3d v3(s1 * s3 - c1 * c3 * s2, c3 * s1 + c1 * s2 * s3, c1 * c2);
	RotMat r(v1, v2, v3);
	return r;
}

RotMat rotXZY(const Vec3d& angles)
{
	double c1, c2, c3, s1, s2, s3;
	c1 = cos(angles.x);
	c2 = cos(angles.y);
	c3 = cos(angles.z);
	s1 = sin(angles.x);
	s2 = sin(angles.y);
	s3 = sin(angles.z);
	Vec3d v1(c2 * c3, -s2, c2*s3);
	Vec3d v2(s1*s3 + c1*c3*s2, c1*c2, c1*s2*s3 - c3*s1);
	Vec3d v3(c3*s1*s2 - c1*s3, c2*s1, c1*c3 + s1*s2*s3);
	RotMat r(v1, v2, v3);
	return r;
}

RotMat rotYXZ(const Vec3d& angles)
{
	double c1, c2, c3, s1, s2, s3;
	c1 = cos(angles.x);
	c2 = cos(angles.y);
	c3 = cos(angles.z);
	s1 = sin(angles.x);
	s2 = sin(angles.y);
	s3 = sin(angles.z);
	Vec3d v1(c1*c3 + s1*s2*s3, c3*s1*s2 - c1*s3, c2*s1);
	Vec3d v2(c2*s3, c2*c3, -s2);
	Vec3d v3(c1*s2*s3 - c3*s1, c1*c3*s2 + s1*s3, c1*c2);
	RotMat r(v1, v2, v3);
	return r;
}

RotMat rotYZX(const Vec3d& angles)
{
	double c1, c2, c3, s1, s2, s3;
	c1 = cos(angles.x);
	c2 = cos(angles.y);
	c3 = cos(angles.z);
	s1 = sin(angles.x);
	s2 = sin(angles.y);
	s3 = sin(angles.z);
	Vec3d v1(c1*c2, s1*s3 - c1*c3*s2, c3*s1 + c1*s2*s3);
	Vec3d v2(s2, c2*c3, -c2*s3);
	Vec3d v3(-c2*s1, c1*s3 + c3*s1*s2, c1*c3 - s1*s2*s3);
	RotMat r(v1, v2, v3);
	return r;
}

RotMat rotZYX(const Vec3d& angles)
{
	double c1, c2, c3, s1, s2, s3;
	c1 = cos(angles.x);
	c2 = cos(angles.y);
	c3 = cos(angles.z);
	s1 = sin(angles.x);
	s2 = sin(angles.y);
	s3 = sin(angles.z);
	Vec3d v1(c1*c2, c1*s2*s3 - c3*s1, s1*s3 + c1*c3*s2);
	Vec3d v2(c2*s1, c1*c3 + s1*s2*s3, c3*s1*s2 - c1*s3);
	Vec3d v3(-s2, c2*s3, c2*c3);
	RotMat r(v1, v2, v3);
	return r;
}

RotMat rotZXY(const Vec3d& angles)
{
	double c1, c2, c3, s1, s2, s3;
	c1 = cos(angles.x);
	c2 = cos(angles.y);
	c3 = cos(angles.z);
	s1 = sin(angles.x);
	s2 = sin(angles.y);
	s3 = sin(angles.z);
	Vec3d v1(c1*c3 - s1*s2*s3, -c2*s1, c1*s3 + c3*s1*s2);
	Vec3d v2(c3*s1 + c1*s2*s3, c1*c2, s1*s3 - c1*c3*s2);
	Vec3d v3(-c2*s3, s2, c2*c3);
	RotMat r(v1, v2, v3);
	return r;
}


// sample from bidirectional reflectance distribution
Vec3d BRDF(const Vec3d& refDir, const Vec3d& normal, double var)
{
	if (var == 0) 
		return Vec3d(refDir);

	double rx, ry, rz;
	Vec3d res;

	if (var >= 1.0)
	{
		rx = uniformRand(-1.0, 1.0);
		ry = uniformRand(-1.0, 1.0);
		rz = uniformRand(-1.0, 1.0);
		res =  Vec3d(rx, ry, rz);
		if (dot(res, normal) < 0)
			res = -res;
	}
	else
	{
		while (1)
		{
			var *= 2.0; // TODO tune this factor
			rx = normalRand(refDir.x, var);
			ry = normalRand(refDir.y, var);
			rz = normalRand(refDir.z, var);
			res = Vec3d(rx, ry, rz);
			if (dot(res, normal) >= 0)
				break;
		}
	}
	res.normalize();
	return res;
}


// to string
std::ostream& operator<< (std::ostream& outs, const Vec3d& v)
{
	return outs << "(" << v.x << ", " << v.y << ", " << v.z << ")";
}