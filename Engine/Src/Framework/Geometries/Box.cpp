#include "pch.h"
#include "Framework/Geometries/Box.h"
#include "Framework/Math/Aabb.h"

void Engine::Box::GetAabb(const Matrix4x4f& trans, Vector3f& aabbMin, Vector3f& aabbMax) const
{
	TransformAabb(half_extent_, margin_, trans,aabbMin, aabbMax);
}
