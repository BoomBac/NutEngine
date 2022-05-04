#include "type.hlsli"
#include "cbuffer.hlsli"


vs2_sm main(float3 position : POSITION)
{
	vs2_sm ret;
	float4x4 vp_matrix;
	float4x4 world_matrix = mul(g_world_matrix_, g_object_matrix_);
	ret.postionW = mul(float4(position, 1.f), world_matrix).xyz;
	if (g_light_mat_index < 100)
		vp_matrix = lights[g_light_mat_index].light_vp;
	else
		vp_matrix =g_point_light_mat[g_light_mat_index - 100];
	ret.position = mul(float4(ret.postionW, 1.f), vp_matrix);
	return ret;
}