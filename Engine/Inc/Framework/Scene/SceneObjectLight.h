#ifndef __SCENE_OBJECT_LIGHT_H__
#define __SCENE_OBJECT_LIGHT_H__

#include "SceneObject.h"

namespace Engine
{
    using AttenFunc = float (*)(float /* Intensity */, float /* Distance */);

    class SceneObjectLight : public BaseSceneObject
    {
    protected:
        // can only be used as base class of delivered lighting objects
        SceneObjectLight(ELightType type) : type_(type), BaseSceneObject(ESceneObjectType::kSceneObjectTypeLight, "default_light") {};
    protected:
        Color       light_color_;
        float       intensity_;
        AttenFunc   light_attenuation_;
        float       near_clip_distance_;
        float       far_clip_distance_;
        bool        b_cast_shadows_;
        std::string texture_;
        ELightType  type_;
    public:
        void SetIfCastShadow(bool shadow) { b_cast_shadows_ = shadow; };
        void SetColor(std::string& attrib, Vector4f& color)
        {
            if (attrib == "color") {
                light_color_ = Color(color);
            }
        };
        void SetParam(std::string& attrib, float param)
        {
            if (attrib == "intensity") {
                intensity_ = param;
            }
        };
        void SetTexture(std::string& attrib, std::string& textureName)
        {
            if (attrib == "projection") {
                texture_ = textureName;
            }
        };
        void SetAttenuation(AttenFunc func)
        {
            light_attenuation_ = func;
        }
        Color GetColor() const { return light_color_; };
        float GetIntensity() { return intensity_; };
        ELightType GetType() const { return type_; };
        void SetType(ELightType type) { type_ = type; };
    };

    class SceneObjectDirectonalLight : public SceneObjectLight
    {
    public:
        SceneObjectDirectonalLight() : SceneObjectLight(ELightType::kDirectional) {}
    };

    class SceneObjectPointLight : public SceneObjectLight
    {
    public:
        SceneObjectPointLight() : SceneObjectLight(ELightType::kPoint) {}
    };

    class SceneObjectSpotLight : public SceneObjectLight
    {
    public:
        SceneObjectSpotLight() : SceneObjectLight(ELightType::kSpot) {}
        float   inner_angle_;
        float   outer_angle_;
    };
}

#endif // !__SCENE_OBJECT_LIGHT_H__