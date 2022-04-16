#include "pch.h"
#include "Framework/Common/CameraManager.h"
#include <thread>
#include <chrono>


namespace Engine
{
	CameraManager::CameraManager()
	{
		p_camera_ = std::make_unique<SceneObjectPerspectiveCamera>();
		p_camera_->SetLens(1.57f,16.f/9.f,10.f,10000.f);
		tar_pos_ = p_camera_->GetPosition();
		std::thread update_camera(&CameraManager::UpdateCameraTransform,this);
		update_camera.detach();
	}
	void CameraManager::Update()
	{
		p_camera_->SetPosition(lerp(p_camera_->GetPosition(), tar_pos_, 0.1f));
	}
	SceneObjectCamera& Engine::CameraManager::GetCamera()
	{
		return *p_camera_.get();
	}
	void CameraManager::SetTargetPosition(const Vector3f& pos)
	{
		tar_pos_ = pos;
	}
	void CameraManager::AddPositionOffset(const float& dx, const float& dy, const float& dz)
	{
		tar_pos_ = CalculateNewPosition(dx,dy,dz);
	}
	Vector3f CameraManager::CalculateNewPosition(const float& dx, const float& dy, const float& dz) const
	{
		auto f = p_camera_->GetForward();
		auto r = p_camera_->GetRight();
		auto u = p_camera_->GetUp();
		float _dx = DotProduct(f, Vector3f{ dz,0.f,0.f });
		float _dy = DotProduct(f, Vector3f{ 0.f,dz,0.f });
		float _dz = DotProduct(f, Vector3f{ 0.f,0.f,dz });
		_dx += DotProduct(r, Vector3f{ dx,0.f,0.f });
		_dy += DotProduct(r, Vector3f{ 0.f,dx,0.f });
		_dz += DotProduct(r, Vector3f{ 0.f,0.f,dx });
		_dx += DotProduct(u, Vector3f{ dy,0.f,0.f });
		_dx += DotProduct(u, Vector3f{ 0.f,dy,0.f });
		_dx += DotProduct(u, Vector3f{ 0.f,0.f,dy });
		return p_camera_->GetPosition() + Vector3f{_dx,_dy,_dz};
	}
	void CameraManager::UpdateCameraTransform()
	{
		while (true)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
			p_camera_->SetPosition(lerp(p_camera_->GetPosition(), tar_pos_, 0.1f));
		}
	}
}


