#ifndef __SCENE_OBJECT_MESH_H__
#define __SCENE_OBJECT_MESH_H__

#include "SceneObject.h"

namespace Engine
{
    class SceneObjectMesh : public BaseSceneObject
    {
    public:
        SceneObjectMesh(bool visible = true, bool shadow = true, bool motion_blur = true) : BaseSceneObject(ESceneObjectType::kSceneObjectTypeMesh),
            b_visible_(visible), b_shadow_(shadow), b_motion_blur_(motion_blur) {};
        SceneObjectMesh(SceneObjectMesh&& mesh)
            : BaseSceneObject(ESceneObjectType::kSceneObjectTypeMesh),
            index_arr_(std::move(mesh.index_arr_)),
            vertex_arr_(std::move(mesh.vertex_arr_)),
            prim_type_(mesh.prim_type_),
            b_visible_(mesh.b_visible_),
            b_shadow_(mesh.b_shadow_),
            b_motion_blur_(mesh.b_motion_blur_) {};
        void AddIndexArray(SceneObjectIndexArray&& array) { index_arr_.push_back(std::move(array)); };
        void AddVertexArray(SceneObjectVertexArray&& array) { vertex_arr_.push_back(std::move(array)); };
        void SetPrimitiveType(EPrimitiveType type) { prim_type_ = type; };
        size_t GetIndexGroupCount() const { return index_arr_.size(); };
        size_t GetIndexCount(const size_t index) const { return (index_arr_.empty() ? 0 : index_arr_[index].GetIndexCount()); };
        size_t GetVertexCount() const { return (vertex_arr_.empty() ? 0 : vertex_arr_[0].GetVertexCount()); };
        size_t GetVertexPropertiesCount() const { return vertex_arr_.size(); };
        const SceneObjectVertexArray& GetVertexPropertyArray(const size_t index) const { return vertex_arr_[index]; };
        const SceneObjectIndexArray& GetIndexArray(const size_t index) const { return index_arr_[index]; };
        const EPrimitiveType& GetPrimitiveType() { return prim_type_; };
    protected:
        std::vector<SceneObjectIndexArray>  index_arr_;
        std::vector<SceneObjectVertexArray> vertex_arr_;
        EPrimitiveType prim_type_;
        bool        b_visible_ = true;
        bool        b_shadow_ = false;
        bool        b_motion_blur_ = false;

    };
}

#endif // !__SCENE_OBJECT_MESH_H__


