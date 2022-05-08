#include "type.hlsli"
#include "cbuffer.hlsli"

//struct ps2
//{
//	float4 fin_color : SV_TARGET;
//	float fin_depth : SV_Depth;
//};

static const float2 inv_atan = {0.1591f,0.3183f};
float2 SampleSphericalMap(float3 v)
{
	float2 uv = float2(atan2(v.x, v.z), asin(v.y));
	uv *= inv_atan;
	uv += 0.5;
	return uv;
}

	float4 main

	(vs2_sm input):
	SV_TARGET
{
	float2 uv = SampleSphericalMap(normalize(-input.postionW));	
	return float4(pow(g_spherical_map_env.Sample(g_sampler, uv).xyz,1.f / 2.2f), 1.f);
}