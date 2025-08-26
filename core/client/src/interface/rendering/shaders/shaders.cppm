// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

export module pragma.client.rendering.shaders;
export import :brdf_convolution;
export import :calc_image_color;
export import :combine_image_channels;
export import :compute_irradiance_map_roughness;
export import :convolute_cubemap_lighting;
export import :cubemap_to_equirectangular;
export import :depth_to_rgb;
export import :equirectangular_to_cubemap;
export import :extract_diffuse_ambinet_occlusion;
export import :forwardp_light_culling;
export import :hdr;
export import :pp_bloom_blur;
export import :pp_dof;
export import :pp_fog;
export import :pp_fxaa;
export import :pp_hdr;
export import :pp_light_cone;
export import :pp_motion_blur;
export import :pp_water;
export import :specular_glossiness_to_metalness_roughness;
export import :specular_to_roughness;
export import :ssao;
export import :ssao_blur;
