#ifndef __GFX_STRUCTURES_H__
#define __GFX_STRUCTURES_H__

#include "Scene.h"

namespace Engine
{
	constexpr int kMaxLightNum = 8;
	constexpr int kMaxPointLightNum = 2;
	struct Light
	{
		Vector3f    light_position;
		float		light_instensity;	//16
		Vector3f    light_direction;
		float		inner_angle;		//32
		Vector3f    light_color;
		float		outer_angle;		//48
		float		falloff_begin;	
		float		falloff_end;
		//low 2 bit for type 0 dir 1 point 2 spot,other for RTTInfo handle
		int			type;	
		int			shadow_map_index;	//64	
		Matrix4x4f	vp_matrix_;
		Light() : light_position(Vector3f{0.f,0.f,0.f}),light_instensity(0.f),light_direction(0.f,0.f,0.f),inner_angle(0.f),light_color(0.f,0.f,0.f),
			outer_angle(0.f),falloff_begin(0.f),falloff_end(0.f),type(-1),shadow_map_index(-1){}
	};
	struct DrawFrameContext
	{
		Matrix4x4f  world_matrix_;
		Matrix4x4f  vp_matrix_;
		Matrix4x4f  projection_matrix_;
		Vector4f    ambient_color_;
		Vector3f	camera_position_;
		float		padding;
		Matrix4x4f	point_light_vp_mat[kMaxPointLightNum * 6 + 6];
		Light lights_[kMaxLightNum];
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
