#ifndef __PYENGINE_2_0_PLANE_H__
#define __PYENGINE_2_0_PLANE_H__

#include "PrimeEngine/Math/Vector3.h"

struct Plane
{
	float a, b, c, d = 0;

	Plane() {}
	Plane(const Vector3 &n) : a(n.getX()), b(n.getY()), c(n.getZ()) {}
	Plane(const Vector3 &n, const Vector3 &p) : a(n.getX()), b(n.getY()), c(n.getZ()) { normalize(p); }

	void normalize(const Vector3 &p)
	{
		d = a * p.getX() + b * p.getY() + c * p.getZ();
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
};

#endif