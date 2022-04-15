#ifndef __TYPE_H__
#define __TYPE_H__
struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};
struct VSInput
{
	float4 position : POSITIONT;
};
cbuffer DrawFrameContext : register(b0)
{
	float4x4 g_world_matrix_;
	float4x4 g_view_matrix_;
	float4x4 g_projection_matrix_;
	float3 g_light_position_;
	float3 g_light_color_;
};
cbuffer DrawBatchContext : register(b1)
{
	float4x4 g_object_matrix_;
	int g_count_;
	float g_color_;
};


#endif //__TYPE_H__