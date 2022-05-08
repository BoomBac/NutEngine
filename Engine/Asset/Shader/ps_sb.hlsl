#include "type.hlsli"
#include "cbuffer.hlsli"

struct ps2
{
	float4 fin_color : SV_TARGET;
	float fin_depth : SV_Depth;
};

float3 FilterCubeMap(float3 normal)
{
	float3 irradiance = { 0.f, 0.f, 0.f };
	float3 up = { 0.f, 1.f, 0.f };
	float3 right = normalize(cross(up, normal));
	up = normalize(cross(normal, right));
	float nrSamples = 0.0;
	float sampleDelta = 0.05f;
	for (float phi = 0.0; phi < TwoPI; phi += sampleDelta)
	{
		for (float theta = 0.0; theta < HalfPI; theta += sampleDelta)
		{
            // spherical to cartesian (in tangent space)
			float3 tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            // tangent space to world
			float3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;
			irradiance += g_cube_map_env.Sample(g_sampler, sampleVec).rgb * cos(theta) * sin(theta);
			nrSamples++;
		}
	}
	irradiance = PI * irradiance * (1.0 / float(nrSamples));
	return irradiance;
}

ps2 main(vs2_sm input)
{
	ps2 ret;
	//ret.fin_color = float4(FilterCubeMap(normalize(input.postionW)), 1.f);
	ret.fin_color = float4(g_filtered_irridance_map.Sample(g_sampler, normalize(input.postionW).xyz).xyz, 1.f);
	ret.fin_depth = 1.f;
	return ret;
}