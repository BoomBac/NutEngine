cbuffer cbSettings
{
	
}

Texture2D gInputA;
Texture2D gInputB;
RWTexture2D<float4> gOutput;

[numthreads(16, 16, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	gOutput[DTid.xy] = gInputA[DTid.xy] + gInputB[DTid.xy];
}