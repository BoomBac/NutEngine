#include "type.hlsli"
#include "cbuffer.hlsli"

float3 FilterCubeMap(float3 normal)
{
	float3 irradiance = { 0.f, 0.f, 0.f };
	float3 up = { 0.f, 1.f, 0.f };
	float3 right = normalize(cross(up, normal));
	up = normalize(cross(normal, right));
	float nrSamples = 0.0;
	float sampleDelta = 0.03f;
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

float4 main (vs2_sm input) :
	SV_TARGET
{
	return float4(FilterCubeMap(normalize(input.postionW)), 1.f);
}