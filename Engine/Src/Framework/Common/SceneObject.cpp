#include "pch.h"
#include "Framework/Common/SceneNode.h"
#include "Framework/Common/Log.h"


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
	void SceneObjectCamera::SetPosition(const float& x, const float& y, const float& z)
	{
		position_.x = x;
		position_.y = y;
		position_.z = z;
		b_dirty_ = true;
		UpdateViewMatrix();
	}
	void SceneObjectCamera::SetPosition(const Vector3f& new_pos)
	{
		position_ = new_pos;
		b_dirty_ = true;
		UpdateViewMatrix();
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
	//------------------------Material
	void SceneObjectMaterial::SetColor(EMaterialProperty type, Vector4f color)
	{
		if(type == kDiffuse) base_color_ = color;
		else if (type == kSpecular)  specular_ = color;
	}

	void SceneObjectMaterial::SetParam(EMaterialProperty type, float param)
	{
		if (type == kSpecularFactor) specular_power_ = param;
	}

	Color SceneObjectMaterial::GetColor(EMaterialProperty type) const
	{
		if(type == kDiffuse) return base_color_;
		else if(type == kSpecular) return specular_;
		NE_LOG(ALL,kWarning,"{} is missing,will return 0.f",kPropertyStrArr[type])
		return Color();
	}

	Parameter SceneObjectMaterial::GetParameter(EMaterialProperty type) const
	{
		if(type == kSpecularFactor) return specular_power_;
		NE_LOG(ALL, kWarning, "{} is missing,will return 0.f", kPropertyStrArr[type])
		return 0.f;
	}

	//Color SceneObjectMaterial::GetColor(std::string name) const
	//{
	//	if (name == "diffuse") return base_color_;
	//	else if (name == "specular") return sepcular_;
	//	else if (name == "emission")  return emission_;
	//	else if (name == "opacity") return opacity_;
	//	else if (name == "transparency") return transparency_;
	//}

	//Parameter SceneObjectMaterial::GetParameter(std::string name) const
	//{
	//	if (name == "specular_power") return specular_power_;
	//	else if (name == "metallic")  return metallic_;
	//	else if (name == "roughness")  return roughness_;
	//	else if (name == "abient_occlusion")  return abient_occlusion_;
	//}

}