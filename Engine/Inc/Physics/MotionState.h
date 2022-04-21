#ifndef __MOTION_STATE__
#define __MOTION_STATE__
#include "Framework/Math/NutMath.hpp"

namespace Engine 
{
    class MotionState
    {
    public:
        MotionState(Matrix4x4f transition) : transition_(transition) {}
        void SetTransition(const Matrix4x4f& transition) { transition_ = transition; }
        const Matrix4x4f& GetTransition() const { return transition_; }
    private:
        Matrix4x4f transition_;
    };
}

#endif // !__MOTION_STATE__




