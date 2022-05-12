#ifndef __SCENE_OBJECT_MESH_H__
#define __SCENE_OBJECT_MESH_H__

#include "SceneObject.h"

namespace Engine
{
    class SceneObjectMesh : public BaseSceneObject
    {
    public:
        SceneObjectMesh(INT64 num_polygon, bool visible = true, bool shadow = true, bool motion_blur = true);
        SceneObjectMesh(SceneObjectMesh&& mesh);
        void AddIndexArray(SceneObjectIndexArray&& array);
        void AddVertexArray(SceneObjectVertexArray&& array,int id);
        void AddVertexArray(SceneObjectVertexArray&& array);
        void SetPrimitiveType(EPrimitiveType type) { prim_type_ = type; };
        size_t GetIndexGroupCount() const { return index_arr_.size(); };
        size_t GetIndexCount(const size_t index) const { return (index_arr_.empty() ? 0 : index_arr_[index].GetIndexCount()); };
        size_t GetVertexCount() const { return (vertex_arr_.empty() ? 0 : vertex_arr_[0].GetVertexCount()); };
        size_t GetVertexPropertiesCount() const { return vertex_arr_.size(); };
        size_t GetPolygonNum() const {return num_polygon_;};

        const Vector3f* GetVertexPosition(INT64 polygon_id) const;
        const SceneObjectVertexArray& GetVertexNormal() const {return vertex_arr_[1];};
        const Vector2f* GetVertexUV(INT64 polygon_id) const;
        const SceneObjectVertexArray& GetVertexTangent() const {return vertex_arr_[3];};

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
        INT64      num_polygon_;
    };
}

#endif // !__SCENE_OBJECT_MESH_H__


