#include "type.hlsli"
#include "cbuffer.hlsli"

struct ps2
{
	float4 fin_color : SV_TARGET;
	float fin_depth : SV_Depth;
};

ps2 main(vs2_sm input)
{
	ps2 ret;
	ret.fin_color = float4(g_sky_box.SampleLevel(g_sampler, normalize(input.postionW),0).xyz, 1.f);

	ret.fin_depth = 1.f;
	return ret;
}