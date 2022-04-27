#ifndef __CBUFFER_H__
#define __CBUFFER_H__

struct Light
{
	float3 light_pos_;
	float light_intensity_;
	float3 light_dir_;
	float inner_angle_;
	float3 light_color_;
	float outer_angle_;
	float falloff_begin_;
	float falloff_end_;
	int light_type_;
	float is_able;
	//float2 light_size_;
	//float2 padding; 
};

cbuffer DrawFrameContext : register(b0)
{
	float4x4 g_world_matrix_;
	float4x4 g_view_matrix_;
	float4x4 g_projection_matrix_;
	float4 g_ambient_color_;
	float3 g_camera_position_;
	float padding;
	Light lights[32];
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

#endif //__CBUFFER_H__