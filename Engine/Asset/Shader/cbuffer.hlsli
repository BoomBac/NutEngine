#ifndef __CBUFFER_H__
#define __CBUFFER_H__

struct Light
{
	float3 light_pos_;
	float light_intensity_;//16
	float3 light_dir_;
	float inner_angle_;	//32
	float3 light_color_;
	float outer_angle_;	//48
	float falloff_begin_;	
	float falloff_end_;
	int light_type_;
	int shadow_map_index;	//64
	float4x4 light_vp;
};

cbuffer DrawFrameContext : register(b0)
{
	float4x4 g_world_matrix_;
	float4x4 g_view_matrix_;
	float4x4 g_projection_matrix_;
	float4 g_ambient_color_;
	float3 g_camera_position_;
	float padding;
	//first 0~5 use for defalut 6 face matrix for generate cube map
	float4x4 g_point_light_mat[18];
	Light lights[8];
};

cbuffer DrawBatchContext : register(b1)
{
	float4x4 g_object_matrix_;
	float4x4 g_normal_matrix_;
	//for material
	float3 g_base_color_;
	float g_metallic_;
	float3 g_emissive_color_;
	float g_roughness_;
	float g_ambient_oc_;
	int g_mat_flag_; //low 1 for base_color,low 2 for roughness,low 3 for metallic,low 4 for emissive,low 5 for normal,low 6 form ao
};

cbuffer LightIndex : register(b2)
{
	int g_light_mat_index;
	//use for calculate linear depth in ps_sm
	int g_cur_light_index;
};

Texture2D g_tex_base_color : register(t0);
Texture2D g_tex_roughness : register(t1);
Texture2D g_tex_metallic : register(t2);
Texture2D g_tex_emissive : register(t3);
Texture2D g_tex_normal : register(t4);
Texture2D g_tex_ambien_oc : register(t5);

Texture2D g_shadow_map[6] : register(t6);
TextureCube g_cube_shadow_map[2] : register(t12);

Texture2D g_spherical_map_env : register(t14);
TextureCube g_cube_map_env : register(t15);
TextureCube g_filtered_irridance_map : register(t16);


SamplerState g_sampler : register(s0);

#endif //__CBUFFER_H__