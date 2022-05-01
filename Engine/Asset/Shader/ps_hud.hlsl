#include "cbuffer.hlsli"

struct Output
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

float4 main(Output input) : SV_TARGET
{
	
	float depth = g_shadow_map[0].Sample(g_sampler, input.uv).x;
	float linear_depth = (2.f * 10.f) / (10000.f + 10.f - depth * (10000.f - 10.f));
	//float linearDepth = near * far / (far - depth * (far - near)); get view-world z 
	return float4(linear_depth, linear_depth, linear_depth, 1.0f);
}