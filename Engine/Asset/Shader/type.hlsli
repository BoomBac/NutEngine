#ifndef __TYPE_H__
#define __TYPE_H__
struct PSInput
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float3 positionW : POSITION;
	float2 uv : TEXCOORD;
};
struct VSOutput
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
};

struct VSOutputDebug
{
	float4 position : SV_POSITION;
	float3 color : COLOR;
};

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);


#endif //__TYPE_H__