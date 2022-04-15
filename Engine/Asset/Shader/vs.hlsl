#include "type.hlsli"


float4 main(float3 position : POSITION) : SV_POSITION
{
	float4x4 vp_matrix = mul(g_view_matrix_, g_projection_matrix_);
	float4x4 world_matrix = mul(g_world_matrix_, g_object_matrix_);
	float4x4 mvp_matrix = mul(world_matrix, vp_matrix);
	return mul(float4(position, 1.f), mvp_matrix);
}