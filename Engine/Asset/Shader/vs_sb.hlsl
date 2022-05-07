#include "type.hlsli"
#include "cbuffer.hlsli"


vs2_sm main(float3 position : POSITION)
{
	vs2_sm res;
	float4x4 vp_matrix = mul(g_view_matrix_, g_projection_matrix_);
	res.position = mul(float4(position + g_camera_position_, 1.f), vp_matrix);
	//now use for normal,the origin is 0,0,0
	res.postionW = position;
	return res;
}