#ifndef __SPHERE_H__
#define __SPHERE_H__

#include "Geometry.h"

namespace Engine
{
    class Sphere : public Geometry
    {
    public:
        Sphere(float radius) : Geometry(EGeometryType::kSphere),radius_(radius) {};
        void GetAabb(const Matrix4x4f& trans, Vector3f& aabbMin, Vector3f& aabbMax) const override;
        float GetRadius() const {return radius_;}
    protected:
       float radius_;
    };
}
#endif //__SPHERE_H__