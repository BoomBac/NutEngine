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


struct vs2_sm
{
	float4 position : SV_POSITION;
	float3 postionW : POSITIONT;
};

static const float PI =			3.1415926535F;
static const float HalfPI =     1.57079632675F;
static const float TwoPI =		6.283185307F;
#endif //__TYPE_H__