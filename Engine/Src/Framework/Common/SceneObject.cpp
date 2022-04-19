#include "pch.h"
#include "Framework/Common/SceneNode.h"
#include "Framework/Common/Log.h"

#include "Framework/Parser/JPEG.h"

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
	void SceneObjectMaterial::SetTexture(EMaterialProperty type, std::string textureName)
	{
		if (type == EMaterialProperty::kDiffuse) base_color_ = std::make_shared<SceneObjectTexture>(textureName);
		else if (type == EMaterialProperty::kSpecular) specular_ = std::make_shared<SceneObjectTexture>(textureName);
		else if (type == EMaterialProperty::kSpecularFactor) specular_power_ = std::make_shared<SceneObjectTexture>(textureName);
		else if (type == EMaterialProperty::kNormalMap) normal_map_ = std::make_shared<SceneObjectTexture>(textureName);
	}
	void SceneObjectMaterial::SetTexture(EMaterialProperty type, std::shared_ptr<SceneObjectTexture>& texture)
	{
		if (type == EMaterialProperty::kDiffuse) base_color_ = texture;
		else if (type == EMaterialProperty::kSpecular) specular_ = texture;
		else if (type == EMaterialProperty::kSpecularFactor) specular_power_ = texture;
		else if (type == EMaterialProperty::kNormalMap) normal_map_ = texture;
	}
	//-----------------------texture
	void SceneObjectTexture::GenerateCheckBoard(INT64 w, INT64 h)
	{
		//width_ = w;
		//height_ = h;
		//pitch_ = w * 4;
		//p_data_ = std::make_unique<UINT8[]>(w * h * 4);
		//for (INT64 i = 0; i < w; i++)
		//{
		//	for (INT64 j = 0; j < h ; j++)
		//	{
		//		p_data_[i * w  * 4 + j * 4] = 0xff;
		//		p_data_[i * w  * 4 + j * 4 + 1] = 0xff;
		//		p_data_[i * w  * 4 + j * 4 + 2] = 0x00;
		//		p_data_[i * w  * 4 + j * 4 + 3] = 0xff;
		//	}
		//}
	}

	void SceneObjectTexture::LoadTexture()
	{
		{
			if (!p_image_)
			{
				//jpeg handle all format temp
				JpegParser parser;
				p_image_ = std::make_shared<Image>(parser.Parse(name_.c_str()));
				width_ = p_image_->width;
				height_ = p_image_->height;
				pitch_ = p_image_->pitch;
			}
		}
	}

}