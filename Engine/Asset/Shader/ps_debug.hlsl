#include "type.hlsli"

float4 main(VSOutputDebug pin) : SV_TARGET
{
	return float4(pin.color,1.0f);
}