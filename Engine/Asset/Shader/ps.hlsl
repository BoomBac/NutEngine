#include "type.hlsli"

float4 main(PSInput input) : SV_TARGET
{
	float3 ambient = g_ambient_color_.xyz;
	float3 light_dir = (0.f, 0.707f, 0.707f);
	float3 normal_dir = normalize(input.normal);
	float3 base_color_tex = g_texture.Sample(g_sampler, input.uv).xyz;
	float3 diffuse = max(0.f, dot(light_dir, normal_dir)) * g_light_color_.xyz * g_base_color_.xyz + base_color_tex * g_use_texture_;
	float3 reflect_dir = normalize(reflect(light_dir, normal_dir));
	float3 view_dir = normalize(input.positionW - g_camera_position_.xyz);
	float3 specular = pow(max(0.f, dot(reflect_dir, view_dir)), max(16.f, g_gloss_)) * g_specular_color_.xyz;
	float3 color = diffuse + specular + ambient;
	return float4( + color,1.f);
}