#ifndef __CAMERA_MANAGER_H__
#define __CAMERA_MANAGER_H__
#include "SceneObject.h"
#include "Framework/Math/NutMath.hpp"

namespace Engine
{
	class CameraManager
	{
	public:
		CameraManager();
		void Update();
		SceneObjectCamera& GetCamera();
		void SetTargetPosition(const Vector3f& pos);
		void AddPositionOffset(const float& dx, const float& dy, const float& dz);
		Vector3f CalculateNewPosition(const float& dx, const float& dy, const float& dz) const;
	private:
		std::unique_ptr<SceneObjectCamera> p_camera_;
		Vector3f tar_pos_;
		void UpdateCameraTransform();
	};
}

#endif // !__CAMERA_MANAGER_H__

