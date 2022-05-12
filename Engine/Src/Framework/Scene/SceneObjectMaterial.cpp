#include "pch.h"
#include "Framework/Scene/SceneObjectMaterial.h"

namespace Engine
{
	SceneObjectMaterial::SceneObjectMaterial(const std::string& name) : BaseSceneObject(ESceneObjectType::kSceneObjectTypeMaterial, name)
	{
		base_color_.value_ = {1.F,1.F,1.F,1.F};
		metallic_.value_ = 0.f;
		roughness_ = 0.6f;
		ambient_occlusion_ = 1.f;
		emission_.value_ = { 0.F,0.F,0.F,0.F };
	}
	void SceneObjectMaterial::SetColor(EMaterialProperty type, Vector4f color)
	{
		if (type == kDiffuse) base_color_ = color;
		else if (type == kSpecular)  specular_ = color;
	}
	void SceneObjectMaterial::SetParam(EMaterialProperty type, float param)
	{
		if (type == kSpecularFactor) specular_power_ = param;
	}

	Color SceneObjectMaterial::GetColor(EMaterialProperty type)
	{
		if (type == kDiffuse) return base_color_;
		else if (type == kSpecular) return specular_;
		NE_LOG(ALL, kWarning, "{} is missing,will return 0.f", kPropertyStrArr[type])
			return Color();
	}

	Parameter SceneObjectMaterial::GetParameter(EMaterialProperty type) const
	{
		if (type == kSpecularFactor) return specular_power_;
		NE_LOG(ALL, kWarning, "{} is missing,will return 0.f", kPropertyStrArr[type])
		return 0.f;
	}
	void SceneObjectMaterial::SetTexture(EMaterialProperty type, std::string file_path)
	{
		std::string name{};
		for (int i = file_path.size() - 1; i >= 0; --i)
		{
			if (file_path[i] == '\\' || file_path[i] == '/')
			{
				name = file_path.substr(i + 1); break;
			}
		}
		if (type == EMaterialProperty::kDiffuse) base_color_ = std::make_shared<SceneObjectTexture>(file_path, name);
		else if (type == EMaterialProperty::kEmissiveColor) emission_ = std::make_shared<SceneObjectTexture>(file_path, name);
		else if (type == EMaterialProperty::kMetallic) metallic_ = std::make_shared<SceneObjectTexture>(file_path, name);
		else if (type == EMaterialProperty::kRoughness) roughness_ = std::make_shared<SceneObjectTexture>(file_path, name);
		else if (type == EMaterialProperty::kNormalMap) normal_map_ = std::make_shared<SceneObjectTexture>(file_path, name);
	}
	void SceneObjectMaterial::SetTexture(EMaterialProperty type, std::shared_ptr<SceneObjectTexture>& texture)
	{
		if (type == EMaterialProperty::kDiffuse) base_color_ = texture;
		else if (type == EMaterialProperty::kEmissiveColor) emission_ = texture;
		else if (type == EMaterialProperty::kMetallic) metallic_ = texture;
		else if (type == EMaterialProperty::kRoughness) roughness_ = texture;
		else if (type == EMaterialProperty::kNormalMap) normal_map_ = texture;
	}
}