#include "cbuffer.hlsli"
#include "type.hlsli"


float LinearInterpolate(float var,float begin,float end)
{
	if (var < begin)
		return 1.f;
	else if (var > end)
		return 0.f;
	else
		return (end - var) / (end - begin);
}

float AttenuationCurve(float dis, int type, float start, float end)
{
	float falloff = 1.f;
	if (type == 0)
		falloff = LinearInterpolate(dis, start, end);
	else if (type == 1)
	{
		float var = LinearInterpolate(dis, start, end);
		falloff = 3.f * pow(var, 2.f) - 2.f * pow(var, 3.f);
	}
	return falloff;
}

float CalculateDistance(float3 start, float3 end)
{
	return sqrt(pow(end.x - start.x, 2.f) + pow(end.y - start.y, 2.f) + pow(end.z - start.z, 2.f));
}

float3 ProjectionOnPlane(float3 p,float3 plane_normal,float3 plane_center)
{
	return p - dot(p - plane_center,plane_normal) * plane_normal;
}

bool IsAbovePlane(float3 p, float3 plane_normal, float3 plane_center)
{
	return dot(p - plane_center,plane_normal) > 0.f;
}

float3 linePlaneIntersect(float3 line_start, float3 line_dir, float3 plane_normal, float3 center_of_plane)
{
	return line_start + line_dir * (dot(center_of_plane - line_start, plane_normal) / dot(line_dir, plane_normal));
}

float3 CalculateLight(Light light, PSInput input)
{
		if (light.light_intensity_ == 0.f)
			return float3(0.f, 0.f, 0.f);
		float3 N = normalize(input.normal);
		float3 PtoL = normalize(light.light_pos_ - input.positionW);
		float dis = distance(light.light_pos_, input.positionW);
		float3 LDir = normalize(light.light_dir_);
		float LToPAngle = acos(dot(-PtoL, -LDir)) * 180.f / 3.14159f;
		float falloff = AttenuationCurve(dis, 1, light.falloff_begin_, light.falloff_end_);
		float3 R;
		//look to camera
		float3 V = normalize(g_camera_position_ - input.positionW.xyz);
		float3 linear_color = (0.f, 0.f, 0.f);
		switch (light.light_type_)
		{
			case 0: //direction light
		{
				R = normalize(reflect(-LDir,N));
					falloff = 1.f;
				}
				break;
			case 1: //point light
		{
					R = normalize(reflect(-PtoL, N));
					LDir = PtoL;
				}
				break;
			case 2: //spot light
			{
				R = normalize(reflect(-LDir, N));
			falloff *= AttenuationCurve(LToPAngle, 1, light.inner_angle_ * 0.5f, light.outer_angle_ * 0.5f);
			}
				break;
		}
		if (g_use_texture_ == 1.f)
		{
			linear_color = light.light_intensity_ * falloff * light.light_color_ * g_texture.Sample(g_sampler, input.uv).xyz * max(dot(N, -LDir), 0.f)
				+ g_specular_color_.rgb * pow(max(0.f, dot(R, V)), max(4.f, g_gloss_));
		}
		else
		{
		linear_color = falloff * light.light_intensity_ * (light.light_color_ * g_base_color_.rgb * max(dot(N, LDir), 0.f) +
			g_specular_color_.rgb * pow(max(0.f, dot(R, V)), max(4.f, g_gloss_)));
	}
	return linear_color;

}

float shadow_test(PSInput input,float cos_theta,int shadow_map_index)
{
	if (shadow_map_index == -1 || shadow_map_index > 7)
		return 1.f;
	float4 pos_light_space = mul(float4(input.positionW, 1.f), lights[shadow_map_index].light_vp);
	pos_light_space.xyz /= pos_light_space.w;
	float2 shadow_uv;
	shadow_uv.x = pos_light_space.x * 0.5f + 0.5f;
	shadow_uv.y = pos_light_space.y * -0.5f + 0.5f;
	//float bias = clamp(0.001f* tan(acos(cos_theta)), 0.f, 0.01f);
	float bias = 0.0000005f;
	float depth = g_shadow_map[shadow_map_index].Sample(g_sampler, shadow_uv).x;
	if (pos_light_space.z - depth > bias)
		return 0.3f;
	return 1.f;
}
	float4 main
	(PSInput input):
	SV_TARGET
{
	float3 linear_color = g_ambient_color_.xyz;

	for (int i = 0; i < 40; ++i)
	{
		float visbility = shadow_test(input, dot(normalize(input.normal),lights[i].light_dir_),lights[i].shadow_map_index);
		linear_color += CalculateLight(lights[i], input) * visbility;
	}
	return float4(pow(clamp(linear_color, float3(0.f, 0.f, 0.f), float3(1.f, 1.f, 1.f)), 1.f / 2.2f), 1.f);
}