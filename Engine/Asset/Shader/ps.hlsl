#include "type.hlsli"

float4 main(PSInput input) : SV_TARGET
{
	//normalize(input.normal);
	//return dot(input.normal, float3(0.f, 0.707f, 0.707f)) * float4(1.f, 1.f, 1.f, 1.f) + 0.2f;
	return float4(normalize(input.normal), 1.f);
}