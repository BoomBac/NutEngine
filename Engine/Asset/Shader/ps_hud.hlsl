#include "cbuffer.hlsli"

struct Output
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

float4 main(Output input) : SV_TARGET
{
	
	float2 shadow_uv;
	shadow_uv.x = input.uv.x * 0.5f + 0.5f;
	shadow_uv.y = input.uv.y * -0.5f + 0.5f;
	///float depth = g_shadow_map[0].Sample(g_sampler, input.uv).x;
	float depth = g_cube_shadow_map[0].Sample(g_sampler, float3(shadow_uv.xy,1.f)).x;
	float linear_depth = (2.f * 10.f) / (10000.f + 10.f - depth * (10000.f - 10.f));
	//float linearDepth = near * far / (far - depth * (far - near)); get view-world z 
	return float4(linear_depth, linear_depth, linear_depth, 1.0f);
}