#ifndef __CBUFFER_H__
#define __CBUFFER_H__

struct Light
{
	float3 light_pos_;
	float light_intensity_;//16
	float3 light_dir_;
	float inner_angle_;	//32
	float3 light_color_;
	float outer_angle_;	//48
	float falloff_begin_;	
	float falloff_end_;
	int light_type_;
	int shadow_map_index;	//64
	float4x4 light_vp;
};

cbuffer DrawFrameContext : register(b0)
{
	float4x4 g_world_matrix_;
	float4x4 g_view_matrix_;
	float4x4 g_projection_matrix_;
	float4 g_ambient_color_;
	float3 g_camera_position_;
	float padding;
	float4x4 g_point_light_mat[12];
	Light lights[8];
};

cbuffer DrawBatchContext : register(b1)
{
	float4x4 g_object_matrix_;
	float4x4 g_normal_matrix_;
	float4 g_base_color_;
	float4 g_specular_color_;
	float g_gloss_;
	float g_use_texture_;
};

cbuffer LightIndex : register(b2)
{
	int g_light_mat_index;
	int g_cur_light_index;
};


Texture2D g_texture : register(t0);
Texture2D g_shadow_map[6] : register(t1);
TextureCube g_cube_shadow_map[2] : register(t7);

SamplerState g_sampler : register(s0);

#endif //__CBUFFER_H__