#include "pch.h"
#include "Framework/Geometries/Sphere.h"

namespace Engine
{
	void Sphere::GetAabb(const Matrix4x4f& trans, Vector3f& aabbMin, Vector3f& aabbMax) const
	{
		Vector3f center;
		GetOrigin(center, trans);
		Vector3f extent(margin_, margin_, margin_);
		aabbMin = center - extent;
		aabbMax = center + extent;
	}
}
