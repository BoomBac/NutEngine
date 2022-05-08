#ifndef __SCENE_OBJECT_CAMERA_H__
#define __SCENE_OBJECT_CAMERA_H__

#include "SceneObject.h"

namespace Engine
{
    class SceneObjectCamera : public BaseSceneObject
    {
    public:
        SceneObjectCamera() : BaseSceneObject(ESceneObjectType::kSceneObjectTypeCamera)
        {
            UpdateViewMatrix();
        };
        SceneObjectCamera(float aspect, float near_clip = 10.0f, float far_clip = 10000.0f) : BaseSceneObject(ESceneObjectType::kSceneObjectTypeCamera),
            aspect_(aspect), near_clip_distance_(near_clip), far_clip_distance_(far_clip)
        {
            UpdateViewMatrix();
        };
        /// <summary>
        /// 
        /// </summary>
        /// <param name="name">near,far,aspect,fov</param>
        /// <param name="param"></param>
        void SetParam(std::string name, float param)
        {
            if (name == "near") near_clip_distance_ = param;
            else if (name == "far") far_clip_distance_ = param;
            else if (name == "aspect") aspect_ = param;
        }

        void SetLens(float fovy, float aspect, float nz, float fz);
        float GetNearZ() const { return near_clip_distance_; };
        float GetFarZ() const { return far_clip_distance_; };
        float GetAspect() const { return aspect_; };
        float GetFovX() const;
        float GetFovY() const;
        float GetNearPlaneWidth() const;
        float GetFarPlaneWidth() const;
        float GetNearPlaneHeight() const;
        float GetFarPlaneHeight() const;
        const Matrix4x4f& GetProjection() const { return projection_; }
        const Matrix4x4f& GetView() const { return view_; }
        void SetPosition(const float& x, const float& y, const float& z);
        void SetPosition(const Vector3f& new_pos);
        const Vector3f& GetPosition() const { return position_; };
        const Vector3f& GetForward() const { return forawrd_; };
        const Vector3f& GetRight() const { return right_; };
        const Vector3f& GetUp() const { return up_; };
        void MoveForward(float dis);
        void MoveRight(float dis);
        void RotatePitch(float angle);
        void RotateYaw(float angle);
    protected:
        void UpdateViewMatrix();
        Vector3f position_{ 0.f, 0.0f, -1000.f };
        Vector3f forawrd_{ 0.0f, 0.0f, 1.f };
        Vector3f up_{ 0.0f, 1.0f, 0.0f };
        Vector3f right_{ 1.f,0.f,0.f };

        bool b_dirty_ = true;
        float aspect_;
        float near_clip_distance_;
        float far_clip_distance_;
        float fov_;
        float near_plane_height_;
        float far_plane_height_;
        Matrix4x4f view_{};
        Matrix4x4f projection_{};
    };
    class SceneObjectOrthogonalCamera : public SceneObjectCamera
    {
    public:
        using SceneObjectCamera::SceneObjectCamera;
    };
    class SceneObjectPerspectiveCamera : public SceneObjectCamera
    {
    protected:
    public:
        SceneObjectPerspectiveCamera(float aspect = 16.0f / 9.0f, float near_clip = 10.0f, float far_clip = 10000.0f,
            float fov = 1.57F) : SceneObjectCamera(aspect, near_clip, far_clip)
        {
            fov_ = fov;
        };
        void SetParam(std::string name, float param)
        {
            if (name == "fov")
                fov_ = param;
            else
                SceneObjectCamera::SetParam(name, param);
        }
        float GetFov() const { return fov_; }
    };
}

#endif // !__SCENE_OBJECT_CAMERA_H__