#include <iostream>
#include "Framework/Math/NutMath.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../External/stb-image/stb_image_write.h"

#include <directxmath.h>


using std::cout;
using std::endl;

using namespace Engine;

const float PI = 3.14159265358979323846264338327950288;

float RadicalInverse_VdC(unsigned int bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10;
}

Vector2f Hammersley(unsigned int i, unsigned int N)
{
	return Vector2f(float(i) / float(N), RadicalInverse_VdC(i));
}

Vector3f ImportanceSampleGGX(Vector2f Xi, float roughness, Vector3f N)
{
	float a = roughness * roughness;

	float phi = 2.0 * PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

	// from spherical coordinates to cartesian coordinates
	Vector3f H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;

	// from tangent-space vector to world-space sample vector
	Vector3f up = abs(N.z) < 0.999 ? Vector3f(0.0, 0.0, 1.0) : Vector3f(1.0, 0.0, 0.0);
	auto tmp = CrossProduct(up, N);
	Vector3f tangent = Normalize(tmp);
	Vector3f bitangent = CrossProduct(N, tangent);

	Vector3f sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return Normalize(sampleVec);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float a = roughness;
	float k = (a * a) / 2.0;

	float nom = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;
}

float GeometrySmith(float roughness, float NoV, float NoL)
{
	float ggx2 = GeometrySchlickGGX(NoV, roughness);
	float ggx1 = GeometrySchlickGGX(NoL, roughness);

	return ggx1 * ggx2;
}

Vector2f IntegrateBRDF(float NdotV, float roughness, unsigned int samples)
{
	Vector3f V;
	V.x = sqrt(1.0 - NdotV * NdotV);
	V.y = 0.0;
	V.z = NdotV;

	float A = 0.0;
	float B = 0.0;

	Vector3f N = Vector3f(0.0, 0.0, 1.0);

	for (unsigned int i = 0u; i < samples; ++i)
	{
		Vector2f Xi = Hammersley(i, samples);
		auto H = ImportanceSampleGGX(Xi, roughness, N);
		auto tmp = 2.0f * DotProduct(V, H) * H - V;
		auto L = Normalize(tmp);

		float NoL = fmax(L.z, 0.0f);
		float NoH = fmax(H.z, 0.0f);
		float VoH = fmax(DotProduct(V, H), 0.0f);
		float NoV = fmax(DotProduct(N, V), 0.0f);

		if (NoL > 0.0)
		{
			float G = GeometrySmith(roughness, NoV, NoL);

			float G_Vis = (G * VoH) / (NoH * NoV);
			float Fc = pow(1.0 - VoH, 5.0);

			A += (1.0 - Fc) * G_Vis;
			B += Fc * G_Vis;
		}
	}

	return Vector2f(A / float(samples), B / float(samples));
}


int main(int argc,char** argv)
{	
    const int size = 64;
    float* data = new float[size * size * 3];
	for (int y = 0; y < size; y++)
	{
		for (int x = 0; x < size; x++)
		{
            float NoV = y * (1.f / size);
            float roughness = x * (1.f / size);
            int id = y * size * 3 + x * 3;
            //auto res = IntegrateBRDF(NoV, roughness,1024);
            auto res = IntegrateBRDF(roughness, NoV,1024);
            //data[id] = res.x * 255;
            //data[id+1] = res.y * 255;
            data[id] = res.x;
            data[id + 1] = res.y;
            data[id+2] = 0;
           // cout << data[id] << " " << data[id+1] << endl;
        }
    }
    stbi_write_hdr("lut.hdr", size, size, 3, data);
    delete[] data;
	return 0;
}

