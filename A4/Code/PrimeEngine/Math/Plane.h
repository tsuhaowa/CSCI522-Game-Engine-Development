#ifndef __PYENGINE_2_0_PLANE_H__
#define __PYENGINE_2_0_PLANE_H__

#include "Vector3.h"

// Notice : The equation here is "ax + by + cz + d = 0" instead of "ax + by + cz = d"
struct Plane
{
	float a, b, c, d;

	Plane() : a(0), b(0), c(0), d(0) { }
	Plane(const Vector3 &n) : a(n.getX()), b(n.getY()), c(n.getZ()) {}
	Plane(const Vector3 &n, const Vector3 &p) : a(n.getX()), b(n.getY()), c(n.getZ()) { normalize(p); }
	float getD() { return d; }
	Vector3 getN() { return Vector3(a, b, c); }  // usually the inner normal, toward the inner of box
	Vector3 getOutsideN() { return getN() * -1.0f; }
	float getFloorHeight() { return (-d) / b; }
	void normalize(const Vector3 &p) { d = (a * p.getX() + b * p.getY() + c * p.getZ()) * -1; }
	bool checkPlaneIntersect(Plane &target) { return !(getN().crossProduct(target.getN()) == Vector3()); }

	void buildPlaneByPoints(const Vector3 &p1, const Vector3 &p2, const Vector3 &p3)
	{
		Vector3 v1 = p2 - p1;
		Vector3 v2 = p3 - p1;
		Vector3 n = v1.crossProduct(v2);
		*this = Plane(n, p1);
	}

	// for left, bottom, near clipping plane
	void buildPlaneByPlus(const Vector3 &v1, const Vector3 &v2, const float &m44, const float &mi4)
	{
		a = v1.getX() + v2.getX();
		b = v1.getY() + v2.getY();
		c = v1.getZ() + v2.getZ();
		d = m44 + mi4;
	}

	// for right, top, far clipping plane
	void buildPlaneByMinus(const Vector3 &v1, const Vector3 &v2, const float &m44, const float &mi4)
	{
		a = v1.getX() - v2.getX();
		b = v1.getY() - v2.getY();
		c = v1.getZ() - v2.getZ();
		d = m44 - mi4;
	}

	bool isInsidePlane(const Vector3 &p)
	{
		float z = a * p.getX() + b * p.getY() + c * p.getZ() + d;
		return (z >= 0.0f);
	}

	bool isOutsidePlane(const Vector3 &p)
	{
		float z = a * p.getX() + b * p.getY() + c * p.getZ() + d;
		return (z < 0.0f);
	}

	bool operator == (const Plane &rP)
	{
		return (a == rP.a) && (b == rP.b) && (c == rP.c) && (d == rP.d);
	}

	bool operator != (const Plane &rP)
	{
		return !(*this == rP);
	}
};

#endif