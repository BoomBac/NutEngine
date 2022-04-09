#include "type.hlsli"

v2p main(a2v input)
{
	v2p output;
	output.Position = mul(mul(mul(float4(input.Position.xyz, 1.0f), m_worldMatrix), m_viewMatrix), m_projectionMatrix);
	//float3 vN = (mul(mul(float4(input.Normal, 0.0f), m_worldMatrix), m_viewMatrix)).xyz;
	output.vPosInView = (mul(mul(float4(input.Position.xyz, 1.0f), m_worldMatrix), m_viewMatrix)).xyz;
	//output.vNorm = vN;
	output.vNorm = float3(1.f,0.f,0.f);
	//output.TextureUV = input.TextureUV;
	return output;
}