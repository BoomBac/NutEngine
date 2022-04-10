#ifndef __TYPE_H__
#define __TYPE_H__

struct a2v
{
	float3 Position		: POSITION;
	float3 Color		: COLOR;
	//float3 Normal		: NORMAL;
	//float2 TextureUV	: TEXCOORD;
};
struct v2p
{
	float4 Position     : SV_POSITION;
	float3 Color		: COLOR;
	//float2 TextureUV    : TEXCOORD0;
	//float3 vNorm		: TEXCOORD1;
	//float3 vPosInView	: TEXCOORD3;
};

cbuffer PerFrameConstants : register(b0)
{
	float4x4 m_worldMatrix;
	float4x4 m_viewMatrix;
	float4x4 m_projectionMatrix;
	float4   m_lightPosition;
	float4   m_lightColor;
};

cbuffer PerBatchConstants : register(b1)
{
	float3 ambientColor;
	float3 diffuseColor;
	float3 specularColor;
	float specularPower;
};

#endif // !__TYPE_H__