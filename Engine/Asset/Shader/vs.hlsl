#include "type.hlsli"
#include "cbuffer.hlsli"


PSInput main(float3 position : POSITION, float3 normal : NORMAL,float2 uv : TEXCOORD) 
{
	PSInput res;
	float4x4 vp_matrix = mul(g_view_matrix_, g_projection_matrix_);
	float4x4 world_matrix = mul(g_world_matrix_, g_object_matrix_);
	
	res.positionW = mul(float4(position, 1.f), world_matrix);
	res.position = mul(float4(res.positionW, 1.f), vp_matrix);
	res.normal = mul(float4(normal, 1.f), g_normal_matrix_);
	res.uv = uv;
	return res;
}