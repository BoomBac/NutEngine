#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__
#include "Framework/Math/NutMath.hpp"

namespace Engine
{
    enum class EGeometryType
    {
        kBox,
        kSphere,
        kCylinder,
        kCone,
        kPlane,
        kCapsule,
        kTriangle
    };
    class Geometry
    {
    public:
        Geometry(EGeometryType geometry_type) : geometry_type_(geometry_type) {};
        Geometry() = delete;
        virtual ~Geometry() = default;

        // GetAabb returns the axis aligned bounding box in the coordinate frame of the given transform trans.
        virtual void GetAabb(const Matrix4x4f& trans, Vector3f& aabbMin, Vector3f& aabbMax) const = 0;
        virtual void GetBoundingSphere(Vector3f& center, float& radius) const;
        virtual float GetAngularMotionDisc() const;
        void CalculateTemporalAabb(const Matrix4x4f& curTrans, const Vector3f& linvel, const Vector3f& angvel, float timeStep,
            Vector3f& temporalAabbMin, Vector3f& temporalAabbMax) const;
        EGeometryType GetGeometryType() const { return geometry_type_; };
    protected:
        EGeometryType geometry_type_;
        float margin_;
    };
}
#endif // !__GEOMETRY_H__

