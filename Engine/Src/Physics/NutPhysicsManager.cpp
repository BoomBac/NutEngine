#include "pch.h"
#include "Physics/NutPhysicsManager.h"
#include "Physics/RigidBody.h"
#include "Framework/Geometries/Box.h"
#include "Framework/Geometries/Sphere.h"
#include "Framework/Geometries/Plane.h"

namespace Engine
{
	int NutPhysicsManager::Initialize()
	{
		return 0;
	}

	void NutPhysicsManager::Finalize()
	{
		ClearRigidBodies();
	}

	void NutPhysicsManager::Tick()
	{
		if(g_pSceneManager->IsSceneChanged())
		{
			ClearRigidBodies();
			CreateRigidBodies();
			g_pSceneManager->NotifySceneIsRenderingQueued();
		}
	}

	void NutPhysicsManager::CreateRigidBody(SceneGeometryNode& node, const SceneObjectGeometry& geometry)
	{
		const float* param = geometry.GetCollisionParam();
		RigidBody* rigidBody = nullptr;
		switch (geometry.GetConllisionType())
		{
		case Engine::ESceneObjectCollisionType::kSceneObjectCollisionTypeNone:
			break;
		case Engine::ESceneObjectCollisionType::kSceneObjectCollisionTypeSphere:
		{
			auto col_box = std::make_shared<Sphere>(param[0]);
			const auto trans = node.GetCalculatedTransform();
			auto ms = std::make_shared<MotionState>(*trans);
			rigidBody = new RigidBody(col_box,ms);
		}
			break;
		case Engine::ESceneObjectCollisionType::kSceneObjectCollisionTypeBox:
		{
			auto col_box = std::make_shared<Box>(Vector3f{ param[0], param[1], param[2] });
			const auto trans = node.GetCalculatedTransform();
			auto ms = std::make_shared<MotionState>(*trans);
			rigidBody = new RigidBody(col_box, ms);
		}
			break;
		case Engine::ESceneObjectCollisionType::kSceneObjectCollisionTypeCylinder:
			break;
		case Engine::ESceneObjectCollisionType::kSceneObjectCollisionTypeCapsule:
			break;
		case Engine::ESceneObjectCollisionType::kSceneObjectCollisionTypeCone:
			break;
		case Engine::ESceneObjectCollisionType::kSceneObjectCollisionTypeMultiSphere:
			break;
		case Engine::ESceneObjectCollisionType::kSceneObjectCollisionTypeConvexHull:
			break;
		case Engine::ESceneObjectCollisionType::kSceneObjectCollisionTypeConvexMesh:
			break;
		case Engine::ESceneObjectCollisionType::kSceneObjectCollisionTypeBvhMesh:
			break;
		case Engine::ESceneObjectCollisionType::kSceneObjectCollisionTypeHeightfield:
			break;
		case Engine::ESceneObjectCollisionType::kSceneObjectCollisionTypePlane:
		{
			auto col_box = std::make_shared<Plane>(Vector3f{ param[0], param[1], param[2]},param[3]);
			const auto trans = node.GetCalculatedTransform();
			auto ms = std::make_shared<MotionState>(*trans);
			rigidBody = new RigidBody(col_box, ms);
		}
			break;
		default:
			break;
		}
		node.LinkRigidBody(rigidBody);
	}

	void NutPhysicsManager::DeleteRigidBody(SceneGeometryNode& node)
	{
		RigidBody* rigidBody = reinterpret_cast<RigidBody*>(node.UnlinkRigidBody());
		if (rigidBody) delete rigidBody;
	}

	int NutPhysicsManager::CreateRigidBodies()
	{
		auto& scene = *g_pSceneManager->GetSceneForRendering();
		// Geometries
		for (auto _it : scene.GeometryNodes)
		{
			auto pGeometryNode = _it.second;
			auto pGeometry = scene.GetGeometry(pGeometryNode->GetSceneObjectRef());
			assert(pGeometry);
			CreateRigidBody(*pGeometryNode, *pGeometry);
		}

		return 0;
	}

	void NutPhysicsManager::ClearRigidBodies()
	{
		auto& scene = *g_pSceneManager->GetSceneForRendering();
		// Geometries
		for (auto _it : scene.GeometryNodes)
		{
			auto pGeometryNode = _it.second;
			DeleteRigidBody(*pGeometryNode);
		}
	}

	Matrix4x4f NutPhysicsManager::GetRigidBodyTransform(void* rigidBody)
	{
		Matrix4x4f trans;
		RigidBody* _rigidBody = reinterpret_cast<RigidBody*>(rigidBody);
		auto motionState = _rigidBody->GetMotionState();
		trans = motionState->GetTransition();
		return trans;
	}

	void NutPhysicsManager::UpdateRigidBodyTransform(SceneGeometryNode& node)
	{
		const auto trans = node.GetCalculatedTransform();
		auto rigidBody = node.RigidBody();
		auto motionState = reinterpret_cast<RigidBody*>(rigidBody)->GetMotionState();
		motionState->SetTransition(*trans);
	}

	void NutPhysicsManager::ApplyCentralForce(void* rigidBody, Vector3f force)
	{
	}

}