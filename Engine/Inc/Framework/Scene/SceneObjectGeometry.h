#ifndef __SCENE_OBJECT_GEOMETRY_H__
#define __SCENE_OBJECT_GEOMETRY_H__

#include "SceneObject.h"
#include "SceneObjectMesh.h"

namespace Engine
{
    class SceneObjectGeometry : public BaseSceneObject
    {
    public:
        SceneObjectGeometry() : BaseSceneObject(ESceneObjectType::kSceneObjectTypeGeometry), collision_type_(ESceneObjectCollisionType::kSceneObjectCollisionTypeNone) {};
        void SetVisibility(bool visible) { b_visible_ = visible; };
        const bool Visible() { return b_visible_; };
        void SetIfCastShadow(bool shadow) { b_shadow_ = shadow; };
        const bool CastShadow() { return b_shadow_; };
        void SetIfMotionBlur(bool motion_blur) { b_motion_blur_ = motion_blur; };
        const bool MotionBlur() { return b_motion_blur_; };
        void AddMesh(std::shared_ptr<SceneObjectMesh>& mesh) { mesh_.push_back(mesh); };
        const std::weak_ptr<SceneObjectMesh> GetMesh() { return (mesh_.empty() ? nullptr : mesh_[0]); };
        const std::weak_ptr<SceneObjectMesh> GetMeshLOD(size_t lod) { return (lod < mesh_.size() ? mesh_[lod] : nullptr); };
        ESceneObjectCollisionType GetConllisionType() const { return collision_type_; };
        const float* GetCollisionParam() const { return collision_param_; };
        void SetCollisionParam(const float* param, INT32 count)
        {
            assert(count > 0 && count < 10);
            memcpy(collision_param_, param, sizeof(float) * count);
        };
        void SetCollisionType(ESceneObjectCollisionType collision_type) { collision_type_ = collision_type; };
    protected:
        std::vector<std::shared_ptr<SceneObjectMesh>> mesh_;
        ESceneObjectCollisionType collision_type_;
        float collision_param_[10];
        bool        b_visible_;
        bool        b_shadow_;
        bool        b_motion_blur_;
    };
}

#endif // !__SCENE_OBJECT_GEOMETRY_H__