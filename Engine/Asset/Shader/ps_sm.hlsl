#include "type.hlsli"
#include "cbuffer.hlsli"

float main(vs2_sm input) : SV_Depth
{
	float linear_length = length(input.postionW - lights[g_cur_light_index].light_pos_);
	return linear_length * 0.0001f;
}