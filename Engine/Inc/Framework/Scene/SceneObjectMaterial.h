#ifndef __SCENE_OBJECT_MATERIAL_H__
#define __SCENE_OBJECT_MATERIAL_H__

#include "SceneObject.h"
#include "SceneObjectTexture.h"

namespace Engine
{
    class SceneObjectMaterial : public BaseSceneObject
    {
    public:
        inline const static Vector4f kMatColorMetallicDefault{1.f,1.f,1.f,0.f};
        inline const static Vector4f kMatMissiveRoughnessDefault{0.f,0.f,0.f,0.6f};
        enum EMaterialProperty
        {
            kDiffuse, // base_color or albedo
            kSpecular,
            kMetallic,
            kRoughness,
            kEmissiveColor,
            kSpecularFactor,
            kNormalMap
        };
        inline static const char* kPropertyStrArr[]{ "diffuse","specular","specular_factor" };

        SceneObjectMaterial(const std::string& name);
        SceneObjectMaterial(std::string&& name) : BaseSceneObject(ESceneObjectType::kSceneObjectTypeMaterial, name) {};

        //SceneObjectMaterial(const std::string& name = "default_material", Color&& base_color = Vector4f(1.0f), Parameter&& metallic = 0.0f, Parameter&& roughness = 0.0f,
        //    Normal&& normal = Vector3f(0.0f, 0.0f, 1.0f), Color&& specular = Vector4f(1.0f), Parameter&& specular_power = 2.f, Parameter&& ao = 0.0f, Color&& opacity = Vector4f(0.f), Color&& transparency = Vector4f(0.f),
        //    Color&& emission = Vector4f(0.f)) :
        //    BaseSceneObject(ESceneObjectType::kSceneObjectTypeMaterial, name), base_color_(std::move(base_color)), metallic_(std::move(metallic)),
        //    roughness_(std::move(roughness)), normal_map_(std::move(normal)), specular_(std::move(specular)), specular_power_(std::move(specular_power)), ambient_occlusion_(std::move(ao))
        //    , opacity_(std::move(opacity)), transparency_(std::move(transparency)), emission_(emission) {};
        void SetName(const std::string& name) { name_ = name; };
        void SetName(std::string&& name) { name_ = std::move(name); };
        //for pbr
        const Color& GetBaseColor() const{return base_color_;};
        const Parameter& GetMetallic() const {return metallic_;}
        const Parameter& GetRoughness() const {return roughness_;}
        const Color& GetEmission() const {return emission_;}
        const Parameter GetAmbientOC() const {return ambient_occlusion_;}
        const Normal& GetNormalMap() const { return normal_map_; };

        const Color& GetSpecularColor() const { return specular_; };
        const Parameter& GetSpecularPower() const { return specular_power_; };

        /// <param name="name">diffuse dispecular emission opacity transparency</param>
        /// <param name="color"></param>
        void SetColor(EMaterialProperty type, Vector4f color);
        /// <param name="name">metallic roughness specular_power abient_occlusion</param>
        /// <param name="param"></param>
        void SetParam(EMaterialProperty type, float param);
        Color GetColor(EMaterialProperty type);
        Parameter GetParameter(EMaterialProperty type) const;
        void SetTexture(EMaterialProperty type, std::string file_path);
        void SetTexture(EMaterialProperty type, std::shared_ptr<Engine::SceneObjectTexture>& texture);
        std::string GetTexture(EMaterialProperty type) const {}

    protected:
        Color       base_color_;
        Parameter   metallic_;
        Parameter   roughness_;
        Parameter   ambient_occlusion_;
        Color       emission_;
        Normal      normal_map_;

        Color       specular_;
        Parameter   specular_power_;
        Color       opacity_;
        Color       transparency_;
        std::string base_color_path_;
    };
}

#endif // !__SCENE_OBJECT_MATERIAL_H__