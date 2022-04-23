#ifndef __NUT_PHYSICS_MANAGER_H
#define __NUT_PHYSICS_MANAGER_H
#include "Framework/Interface/IPhysicsManager.h"
#include "Framework/Geometries/Geometry.h"

namespace Engine
{
	class NutPhysicsManager : public IPhysicsManager
	{
	public:
		int Initialize() override;
		void Finalize() override;
		void Tick() override;

		void CreateRigidBody(SceneGeometryNode& node, const SceneObjectGeometry& geometry) override;
		void DeleteRigidBody(SceneGeometryNode& node) override;

		int CreateRigidBodies() override;
		void ClearRigidBodies() override;

		Matrix4x4f GetRigidBodyTransform(void* rigidBody) override;
		void UpdateRigidBodyTransform(SceneGeometryNode& node) override;

		void ApplyCentralForce(void* rigidBody, Vector3f force) override;
#ifdef _DEBUG
		void DrawDebugInfo();
#endif
	protected:
#ifdef _DEBUG
		void DrawAabb(const Geometry& geometry, const Matrix4x4f& trans);
#endif
	};
}

#endif // !__NUT_PHYSICS_MANAGER_H

