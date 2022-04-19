#ifndef __TYPE_H__
#define __TYPE_H__
struct PSInput
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float3 positionW : POSITION;
};
struct VSOutput
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
};
cbuffer DrawFrameContext : register(b0)
{
	float4x4 g_world_matrix_;
	float4x4 g_view_matrix_;
	float4x4 g_projection_matrix_;
	float4	g_ambient_color_;
	float4	g_light_color_;
	float4 g_light_position_;
	float4 g_camera_position_;
};

cbuffer DrawBatchContext : register(b1)
{
	float4x4 g_object_matrix_;
	float4x4 g_normal_matrix_;
	float4 g_base_color_;
	float4 g_specular_color_;
	float g_gloss_;
	int g_count_;
};


#endif //__TYPE_H__