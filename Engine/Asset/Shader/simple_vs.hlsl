struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

cbuffer Constant : register(b0)
{
	float4x4 g_view_projection;
};

PSInput main (float4 position : POSITION, float4 color : COLOR)
{
	PSInput result;
	result.position = mul(position,g_view_projection);
	result.color =color;
	return result;
}