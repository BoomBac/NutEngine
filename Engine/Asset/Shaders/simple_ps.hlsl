#include "type.hlsli"

//float4 main(v2p input) : SV_TARGET
//{
//	return float4(1.0f, 1.0f, 1.0f, 1.0f);
//}

float4 main(v2p input) : SV_TARGET
{
	return float4(input.Color,1.f);
}