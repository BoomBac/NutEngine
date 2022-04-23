#include "type.hlsli"

//VSOutputDebug main(float3 pos : POSITION, float3 color : COLOR)
//{
//	VSOutputDebug ret;
//	float4x4 vp_matrix = mul(g_view_matrix_, g_projection_matrix_);
//	float4x4 mvp_matrix = mul(g_world_matrix_, vp_matrix);
//	ret.position = mul(float4(pos, 1.f), mvp_matrix);
//	ret.color = color;
//	return ret;
//}


VSOutputDebug main(float3 pos : POSITION)
{
	VSOutputDebug ret;
	float4x4 vp_matrix = mul(g_view_matrix_, g_projection_matrix_);
	float4x4 mvp_matrix = mul(g_world_matrix_, vp_matrix);
	ret.position = mul(float4(pos, 1.f), mvp_matrix);
	ret.color = float3(1.f,1.f,1.f);
	return ret;
}