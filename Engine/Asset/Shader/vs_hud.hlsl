struct Output
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

Output main(float3 position : POSITION, float2 uv : TEXCOORD)
{
	Output ret;
	ret.position = float4(position, 1.f);
	ret.uv = uv;
	return ret;
}