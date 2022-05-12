#include "cbuffer.hlsli"
#include "type.hlsli"


float LinearInterpolate(float var, float begin, float end)
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

float3 ProjectionOnPlane(float3 p, float3 plane_normal, float3 plane_center)
{
	return p - dot(p - plane_center, plane_normal) * plane_normal;
}

bool IsAbovePlane(float3 p, float3 plane_normal, float3 plane_center)
{
	return dot(p - plane_center, plane_normal) > 0.f;
}

float3 linePlaneIntersect(float3 line_start, float3 line_dir, float3 plane_normal, float3 center_of_plane)
{
	return line_start + line_dir * (dot(center_of_plane - line_start, plane_normal) / dot(line_dir, plane_normal));
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float num = NdotV;
	float denom = NdotV * (1.0 - k) + k;
	
	return num / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);
	
	return ggx1 * ggx2;
}

float DistributionGGX(float3 N, float3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.f);
	float NdotH2 = NdotH * NdotH;
	float up = a2;
	float bottom = NdotH2 * (a2 - 1.f) + 1.f;
	bottom = PI * bottom * bottom;
	return up / bottom;
}

float3 SchlickFresnel(float3 H, float3 V, float3 f)
{
	return f + (float3(1.f, 1.f, 1.f) - f) * pow(1.f - dot(H, V), 5.f);
}
float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
	float3 f = max(float3(1.0 - roughness, 1.0 - roughness, 1.0 - roughness),F0);
	return F0 + (f, - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}



float shadow_test(int light_id,int shadow_map_id, float3 sp_pos_w, float3 light_pos_w)
{
	if (shadow_map_id == -1)
		return 1.f;
	float closest_depth;
	float4 pos_light_space = mul(float4(sp_pos_w, 1.f), lights[light_id].light_vp);
	pos_light_space.xyz /= pos_light_space.w;
	float2 shadow_uv;
	shadow_uv.x = pos_light_space.x * 0.5f + 0.5f;
	shadow_uv.y = pos_light_space.y * -0.5f + 0.5f;
	closest_depth = g_shadow_map[shadow_map_id].Sample(g_sampler, shadow_uv).x * 10000.f;
	//float bias = clamp(0.001f* tan(acos(cos_theta)), 0.f, 0.01f);
	float bias = 2.f;
	float cur_depth = length(sp_pos_w - light_pos_w);
	if (cur_depth - closest_depth > bias)
		return 0.0f;
	return 1.f;
}

float shadow_test_point_light(int cube_shadow_map_index, float3 sp_pos_w, float3 light_pos_w)
{
	if (cube_shadow_map_index == -1)
		return 1.f;
	float closest_depth = g_cube_shadow_map[cube_shadow_map_index].Sample(g_sampler, normalize(sp_pos_w - light_pos_w)).x * 10000.f;
	float cur_depth = length(sp_pos_w - light_pos_w);
	float bias = 0.5f;
	if (cur_depth - closest_depth > bias)
		return 0.3f;
	return 1.f;
}

float3 NormalSampleToWorldSpace(float3 normal_sample, float3 normal_w, float3 tangentW)
{
	float3 normalT = 2.f * normal_sample - 1.f;
	float3 N = normalize(normal_w);
	float3 T = normalize(tangentW - dot(tangentW, N) * N);
	float3 B = cross(N, T);
	float3x3 TBN = float3x3(T, B, N);
	float3 normal_sampleW = mul(normalT, TBN);
	return normal_sampleW;
}

float3 CookTorranceBRDF(PSInput input, float3 light_dir)
{
	//low 1 for base_color,low 2 for roughness,low 3 for metallic,low 4 for emissive,low 5 for normal,low 6 form ao
	float3 base_color = (g_mat_flag_ & 0x01) == 0x01 ? g_base_color_ : g_tex_base_color.Sample(g_sampler, input.uv).xyz;
	float roughness = (g_mat_flag_ & 0x02) == 0x02 ? g_roughness_ : g_tex_roughness.Sample(g_sampler, input.uv).x;
	float metallic = (g_mat_flag_ & 0x04) == 0x04 ? g_metallic_ : g_tex_metallic.Sample(g_sampler, input.uv).x;
	//float3 base_color = g_tex_base_color.Sample(g_sampler, input.uv).xyz;
	float3 N = normalize(input.normal);
	float3 L = normalize(light_dir);
	float3 V = normalize(g_camera_position_ - input.positionW);
	float G = GeometrySmith(N, V, L, roughness);
	float3 H = normalize(L + V);
	float NDF = DistributionGGX(N, H, roughness);
	float3 F = SchlickFresnel(H, N, float3(0.4, 0.4, 0.4));
	float3 up = NDF * G * F;
	float a = max(dot(N, V), 0.f);
	float b = max(dot(N, L), 0.f);
	float3 bottom = 4.f * a * b + 0.0000001f;
	float3 specular = up / bottom;
	float3 KS = F;
	float3 kD = float3(1.f, 1.f, 1.f) - KS;
	kD *= 1.f - metallic;
	return kD * base_color / PI + KS * specular;
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
	float3 linear_color = { 0.f, 0.f, 0.f };
	int type = light.light_type_ & 0x03;
	switch (type)
	{
		case 0: //direction light
		{
				R = normalize(reflect(-LDir, N));
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
	linear_color = falloff * light.light_intensity_ * light.light_color_ * CookTorranceBRDF(input, LDir);
	return linear_color;

}

float3 CalculateAmbientLight(PSInput input)
{
	float3 ambient = { 0.f, 0.f, 0.f };
	float metallic = (g_mat_flag_ & 0x04) == 0x04 ? g_metallic_ : g_tex_metallic.Sample(g_sampler, input.uv).x;
	float3 N = normalize(input.normal);
	float3 V = normalize(g_camera_position_ - input.positionW);
	float3 KS = FresnelSchlickRoughness(max(0.f, dot(N, V)), float3(0.04, 0.04, 0.04), 0.1f);
	float3 KD = float3(1.f, 1.f, 1.f) - KS;
	KD *= 1.F - metallic;
	float3 irridance = g_filtered_irridance_map.Sample(g_sampler, normalize(N)).xyz;
	ambient = KD * irridance;
	return ambient;
}

	float4 main

	(PSInput input):
	SV_TARGET
{
		if ((g_mat_flag_ & 0x10) != 0x10)
		{
			float3 normal_sample = g_tex_normal.Sample(g_sampler, input.uv).xyz;
			input.normal = NormalSampleToWorldSpace(normal_sample, input.normal, input.tangentW);
		}
		float3 emissive = (g_mat_flag_ & 0x08) == 0x08 ? g_emissive_color_ : g_tex_emissive.Sample(g_sampler, input.uv).xyz;
		float3 linear_color = emissive + CalculateAmbientLight(input) * 0.08f;
		for (int i = 0; i < 40; ++i)
		{
			int type = lights[i].light_type_ & 0x03;
			float visbility = 0;
			if (type == 1)
				visbility = shadow_test_point_light(lights[i].shadow_map_index, input.positionW, lights[i].light_pos_);
			else
				visbility = shadow_test(i, lights[i].shadow_map_index, input.positionW, lights[i].light_pos_);
			linear_color += CalculateLight(lights[i], input) * visbility;
		}
		return float4(pow(clamp(linear_color, float3(0.f, 0.f, 0.f), float3(1.f, 1.f, 1.f)), 1.f / 2.2f), 1.f);
	}