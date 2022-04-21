#ifndef __BOX_H__
#define __BOX_H__

#include "Geometry.h"

namespace Engine
{
	class Box : public Geometry
	{
    public:
        Box() : Geometry(EGeometryType::kBox) {};
        Box(Vector3f half_extent) : Geometry(EGeometryType::kBox), half_extent_(half_extent) {};
        void GetAabb(const Matrix4x4f& trans,Vector3f& aabbMin,Vector3f& aabbMax) const override;

        Vector3f GetDimension() const { return 2.f * half_extent_; };
        Vector3f GetDimensionWithMargin() const { return  margin_ + 2.f * half_extent_; };
        Vector3f GetHalfExtents() const {return half_extent_;};
        Vector3f GetHalfExtentWithMargin() const{return margin_ + half_extent_;};
    protected:
        Vector3f half_extent_;
	};
}
#endif //__BOX_H__