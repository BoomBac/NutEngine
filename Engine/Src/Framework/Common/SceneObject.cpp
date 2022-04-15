#include "pch.h"
#include "Framework/Common/SceneNode.h"
#include <directxmath.h>


namespace Engine
{
	void Engine::SceneObjectCamera::SetLens(float fovy, float aspect, float nz, float fz)
	{
		fov_ = fovy;
		aspect_ = aspect;
		near_clip_distance_ = nz;
		far_clip_distance_ = fz;
		near_plane_height_ = 2.f * nz  * tanf(0.5f * fov_);
		near_plane_height_ = 2.f * fz  * tanf(0.5f * fov_);
		BuildPerspectiveFovLHMatrix(projection_,fov_,aspect_,nz,fz);
	}
	float SceneObjectCamera::GetFovX() const
	{
		float half = 0.5f * GetNearPlaneWidth();
		return 2.f * atan(half / near_clip_distance_);
	}
	float SceneObjectCamera::GetFovY() const
	{
		return 0.0f;
	}
	float SceneObjectCamera::GetNearPlaneWidth() const
	{
		return aspect_ * near_plane_height_;
	}
	float SceneObjectCamera::GetFarPlaneWidth() const
	{
		return aspect_ * far_plane_height_;
	}
	float SceneObjectCamera::GetNearPlaneHeight() const
	{
		return near_plane_height_;
	}
	float SceneObjectCamera::GetFarPlaneHeight() const
	{
		return far_plane_height_;
	}
	void SceneObjectCamera::MoveForward(float dis)
	{
		Vector3f dx = DotProduct(forawrd_,Vector3f{ dis,0.f,0.f});
		Vector3f dy = DotProduct(forawrd_,Vector3f{0.f,dis,0.f});
		Vector3f dz = DotProduct(forawrd_,Vector3f{0.f,0.f,dis });	
		Vector3f len = {dx[0],dy[0],dz[0]};
		position_ = len + position_;
		b_dirty_ = true;
		UpdateViewMatrix();
	}
	void SceneObjectCamera::MoveRight(float dis)
	{
		Vector3f dx = DotProduct(right_, Vector3f{ dis,0.f,0.f });
		Vector3f dy = DotProduct(right_, Vector3f{ 0.f,dis,0.f });
		Vector3f dz = DotProduct(right_, Vector3f{ 0.f,0.f,dis });
		Vector3f len = { dx[0],dy[0],dz[0] };
		position_ = len + position_;
		b_dirty_ = true;
		UpdateViewMatrix();
	}
	void SceneObjectCamera::RotatePitch(float angle)
	{
		Matrix4x4f m{};
		MatrixRotationAxis(m,right_,angle);
		TransformCoord(up_, m);
		TransformCoord(forawrd_, m);
		b_dirty_ = true;
		UpdateViewMatrix();
	}
	void SceneObjectCamera::RotateYaw(float angle)
	{
		Matrix4x4f m{};
		MatrixRotationY(m,angle);
		TransformCoord(right_,m);
		TransformCoord(up_,m);
		TransformCoord(forawrd_,m);
		b_dirty_ = true;
		UpdateViewMatrix();
	}
	void SceneObjectCamera::UpdateViewMatrix()
	{
		if(b_dirty_)
		{
			BuildViewMatrixLookToLH(view_,position_,forawrd_,up_);
			b_dirty_ = false;
		}
	}
}