#ifndef __GFX_STRUCTURES_H__
#define __GFX_STRUCTURES_H__

#include "Scene.h"

namespace Engine
{
	struct Light
	{
		Vector3f    light_position;
		float		light_instensity;
		Vector3f    light_direction;
		float		inner_angle;
		Vector3f    light_color;
		float		outer_angle;
		float		falloff_begin;
		float		falloff_end;
		int			type;
		float		is_able;
	};
	struct DrawFrameContext
	{
		Matrix4x4f  world_matrix_;
		Matrix4x4f  view_matrix_;
		Matrix4x4f  projection_matrix_;
		Vector4f    ambient_color_;
		Vector3f	camera_position_;
		float		is_able;
		Light lights_[32];
	};
	struct DrawBatchContext
	{
		std::shared_ptr<SceneGeometryNode> node;
		std::shared_ptr<SceneObjectMaterial> material;
		Matrix4x4f transform;
		INT32 batch_index;
		virtual ~DrawBatchContext() = default;
	};
	struct Frame
	{
		DrawFrameContext frame_context;
		std::vector<std::shared_ptr<DrawBatchContext>> batch_contexts;
		long long shadow_map;
		UINT32 shadow_map_count;
		Frame() : shadow_map(-1),shadow_map_count(0) {}
	};
}
#endif //__GFX_STRUCTURES_H__
