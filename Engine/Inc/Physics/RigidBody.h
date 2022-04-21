#pragma once
#include <memory>
#include "Framework/Geometries/Geometry.h"
#include "MotionState.h"

namespace Engine 
{
    class RigidBody 
    {
    public:
        RigidBody(std::shared_ptr<Geometry> collisionShape, std::shared_ptr<MotionState> state) : p_collision_shape_(collisionShape), p_motion_state_(state) {}
        std::shared_ptr<MotionState> GetMotionState() { return p_motion_state_; }
        std::shared_ptr<Geometry>    GetCollisionShape() { return p_collision_shape_; }
    private:
        std::shared_ptr<Geometry>       p_collision_shape_;
        std::shared_ptr<MotionState>    p_motion_state_;
    };
}