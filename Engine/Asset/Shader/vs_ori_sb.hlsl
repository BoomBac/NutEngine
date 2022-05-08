#include "type.hlsli"
#include "cbuffer.hlsli"


vs2_sm main(float3 position : POSITION)
{
	vs2_sm res;
	res.position = mul(float4(position, 1.f), g_point_light_mat[g_light_mat_index]);
	//now use for normal,the origin is 0,0,0
	res.postionW = position;
	return res;
}