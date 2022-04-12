#include "type.hlsli"


float4 main(float3 position : POSITION) : SV_POSITION
{
	return mul(float4(position, 1.f), mul(g_view_matrix_, g_projection_matrix_));
}