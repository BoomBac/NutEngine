#include "cbuffer.hlsli"

struct Output
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

#define PI 3.141592653589793

float saturate(float x)
{
	return clamp(x, 0.0, 1.0);
}

float2 hammersley(uint i, uint N)
{
	uint bits = (i << 16u) | (i >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	float rdi = float(bits) * 2.3283064365386963e-10;
	return float2(float(i) / float(N), rdi);
}

float G_Smith(float NoV, float NoL, float roughness)
{
	float k = (roughness * roughness) / 2.0;
	float GGXL = NoL / (NoL * (1.0 - k) + k);
	float GGXV = NoV / (NoV * (1.0 - k) + k);
	return GGXL * GGXV;
}

float V_SmithGGXCorrelated(float NoV, float NoL, float roughness)
{
	float a2 = pow(roughness, 4.0);
	float GGXV = NoL * sqrt(NoV * NoV * (1.0 - a2) + a2);
	float GGXL = NoV * sqrt(NoL * NoL * (1.0 - a2) + a2);
	return 0.5 / (GGXV + GGXL);
}


float3 importanceSampleGGX(float2 Xi, float roughness, float3 N)
{
	float a = roughness * roughness;
	float Phi = 2.0 * PI * Xi.x;
	float CosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
	float SinTheta = sqrt(1.0 - CosTheta * CosTheta);
	float3 H;
	H.x = SinTheta * cos(Phi);
	H.y = SinTheta * sin(Phi);
	H.z = CosTheta;
	float3 UpVector = abs(N.z) < 0.999 ? float3(0., 0., 1.0) : float3(1.0, 0., 0.);
	float3 TangentX = normalize(cross(UpVector, N));
	float3 TangentY = cross(N, TangentX);
	return TangentX * H.x + TangentY * H.y + N * H.z;
}


float2 integrateBRDF(float roughness, float NoV)
{
	float3 V;
	V.x = sqrt(1.0 - NoV * NoV); // sin
	V.y = 0.0;
	V.z = NoV; // cos
	const float3 N = float3(0.0, 0.0, 1.0);
    
	float A = 0.0;
	float B = 0.0;
	const int numSamples = 1024u;
    
	for (int i = 0u; i < numSamples; i++)
	{
		float2 Xi = hammersley(i, numSamples);
         // Sample microfacet direction
		float3 H = importanceSampleGGX(Xi, roughness, N);
        
         // Get the light direction
		float3 L = 2.0 * dot(V, H) * H - V;
        
		float NoL = saturate(dot(N, L));
		float NoH = saturate(dot(N, H));
		float VoH = saturate(dot(V, H));
        
		if (NoL > 0.0)
		{
			float V_pdf = V_SmithGGXCorrelated(NoV, NoL, roughness) * VoH * NoL / NoH;
			float Fc = pow(1.0 - VoH, 5.0);
			A += (1.0 - Fc) * V_pdf;
			B += Fc * V_pdf;
		}
	}

	return 4.0 * float2(A, B) / float(numSamples);
}


float4 main(Output input) : SV_TARGET
{
	//float2 res = integrateBRDF(input.uv.y, input.uv.x);
	//return float4(res, 0.f, 1.0f);
	
	
	float2 shadow_uv;
	shadow_uv.x = input.uv.x * 0.5f + 0.5f;
	shadow_uv.y = input.uv.y * -0.5f + 0.5f;
	float depth = g_shadow_map[0].Sample(g_sampler, input.uv).x;
	//float depth = g_cube_shadow_map[0].Sample(g_sampler, float3(shadow_uv.xy,1.f)).x;
	//float linear_depth = (2.f * 100.f) / (100000.f + 100.f - depth * (100000.f - 100.f));
	//float linearDepth = near * far / (far - depth * (far - near)); get view-world z 
	//return float4(linear_depth, linear_depth, linear_depth, 1.0f);
	return float4(depth, depth, depth, 1.0f);
}