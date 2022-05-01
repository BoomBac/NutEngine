#include "type.hlsli"
#include "cbuffer.hlsli"


float4 main(float3 position : POSITION) : SV_POSITION
{
	float4 ret;
	float4x4 vp_matrix = mul(lights[g_light_mat_index].light_vp, g_projection_matrix_);
	float4x4 world_matrix = mul(g_world_matrix_, g_object_matrix_);
	float4x4 mvp_matrix = mul(world_matrix, vp_matrix);
	float3 world_pos = mul(float4(position, 1.f),world_matrix);
	ret = mul(float4(world_pos, 1.f), lights[g_light_mat_index].light_vp);
	return ret;
}