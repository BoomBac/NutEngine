#include "pch.h"
#include "Framework/Geometries/Geometry.h"
#include "Framework/Geometries/Box.h"

namespace Engine
{
	void Geometry::GetBoundingSphere(Vector3f& center, float& radius) const
	{
	}

	float Geometry::GetAngularMotionDisc() const
	{
		return 0.0f;
	}

	void Geometry::CalculateTemporalAabb(const Matrix4x4f& curTrans, const Vector3f& linvel, const Vector3f& angvel, float timeStep, Vector3f& temporalAabbMin, Vector3f& temporalAabbMax) const
	{
	}

}