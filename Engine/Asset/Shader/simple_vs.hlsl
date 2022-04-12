#include "type.hlsli"



PSInput main (float4 position : POSITION, float4 color : COLOR)
{
	PSInput result;
	result.position = mul(position, g_projection_matrix_ * g_view_matrix_);
	result.color =color;
	return result;
}