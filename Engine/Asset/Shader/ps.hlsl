#include "type.hlsli"

float4 main(PSInput input) : SV_TARGET
{
	return float4(input.normal,1.f);
}