#include "pch.h"
#include "Framework/Scene/SceneObjectMesh.h"

namespace Engine
{
	SceneObjectMesh::SceneObjectMesh(INT64 num_polygon, bool visible, bool shadow, bool motion_blur) : BaseSceneObject(ESceneObjectType::kSceneObjectTypeMesh),
        num_polygon_(num_polygon),b_visible_(visible), b_shadow_(shadow), b_motion_blur_(motion_blur)
	{
        //postion normal uv tangant
        vertex_arr_.resize(4);
	}

    SceneObjectMesh::SceneObjectMesh(SceneObjectMesh&& mesh) : BaseSceneObject(ESceneObjectType::kSceneObjectTypeMesh),
        index_arr_(std::move(mesh.index_arr_)),
        vertex_arr_(std::move(mesh.vertex_arr_)),
        prim_type_(mesh.prim_type_),
        b_visible_(mesh.b_visible_),
        b_shadow_(mesh.b_shadow_),
        b_motion_blur_(mesh.b_motion_blur_)
	{

	}

    void SceneObjectMesh::AddIndexArray(SceneObjectIndexArray&& array)
    {
        index_arr_.push_back(std::move(array));
    }

    void SceneObjectMesh::AddVertexArray(SceneObjectVertexArray&& array, int id)
    {
        vertex_arr_[id] = std::move(array);
    }

    void SceneObjectMesh::AddVertexArray(SceneObjectVertexArray&& array)
    {
        vertex_arr_.push_back(std::move(array));
    }

    const Vector3f* SceneObjectMesh::GetVertexPosition(INT64 polygon_id) const
    {    
        void* ptr = const_cast<void*>(vertex_arr_[0].GetData());
        float* fptr = reinterpret_cast<float*>(ptr);
        return reinterpret_cast<Vector3f*>(&fptr[polygon_id * 9]);
    }

    const Vector2f* SceneObjectMesh::GetVertexUV(INT64 polygon_id) const
    {
        void* ptr = const_cast<void*>(vertex_arr_[2].GetData());
        float* fptr = reinterpret_cast<float*>(ptr);
        return reinterpret_cast<Vector2f*>(&fptr[polygon_id * 6]);
    }

}