#ifndef __PLANE_H__
#define __PLANE_H__

#include "Geometry.h"

namespace Engine
{
    class Plane : public Geometry
    {
    public:
        Plane(Vector3f normal, float intercept) : Geometry(EGeometryType::kSphere) {};
        void GetAabb(const Matrix4x4f& trans, Vector3f& aabbMin, Vector3f& aabbMax) const override;
        float GetIntercept() const { return intercept_; }
        Vector3f GetNormal() const { return normal_; }
    protected:
        Vector3f normal_;
        float    intercept_;
    };
}
#endif //__PLANE_H__