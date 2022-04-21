#include "pch.h"
#include <limits>
#include "Framework/Geometries/Plane.h"


namespace Engine
{
	void Plane::GetAabb(const Matrix4x4f& trans, Vector3f& aabbMin, Vector3f& aabbMax) const
	{
		(void)trans;
		float minf = (std::numeric_limits<float>::min)();
		float maxf = (std::numeric_limits<float>::max)();
		aabbMin = { minf, minf, minf };
		aabbMax = { maxf, maxf, maxf };
	}
}