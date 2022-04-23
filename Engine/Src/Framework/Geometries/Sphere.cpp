#include "pch.h"
#include "Framework/Geometries/Sphere.h"

namespace Engine
{
	void Sphere::GetAabb(const Matrix4x4f& trans, Vector3f& aabbMin, Vector3f& aabbMax) const
	{
		Vector3f center;
		GetOrigin(center, trans);
		Vector3f extent(radius_ * trans[0][0], radius_ * trans[0][0], radius_  * trans[0][0]);
		aabbMin = center - extent;
		aabbMax = center + extent;
	}
}
