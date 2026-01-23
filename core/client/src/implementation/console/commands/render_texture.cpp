// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/console/helper.hpp"

module pragma.client;

import :console.commands;

static void get_filter_mode(UInt32 filter, prosper::Filter &minFilter, prosper::Filter &magFilter, prosper::SamplerMipmapMode &mipmapMode, UInt32 &anisotropy)
{
	anisotropy = 1; // 1 = off
	if(filter < 0)
		filter = 0;
	else if(filter > 6)
		filter = 6;
	if(filter == 0) {
		minFilter = prosper::Filter::Nearest;
		magFilter = prosper::Filter::Nearest;
		mipmapMode = prosper::SamplerMipmapMode::Nearest;
	}
	else if(filter == 1) {
		minFilter = prosper::Filter::Nearest;
		magFilter = prosper::Filter::Linear;
		mipmapMode = prosper::SamplerMipmapMode::Nearest;
	}
	else {
		minFilter = prosper::Filter::Linear;
		magFilter = prosper::Filter::Linear;
		mipmapMode = prosper::SamplerMipmapMode::Linear;
		if(filter >= 3) {
			filter -= 2;
			anisotropy = pragma::math::pow(static_cast<uint32_t>(2), filter);
		}
	}
}

static auto cvTextureQuality = pragma::console::get_client_con_var("cl_render_texture_quality");
static uint32_t get_quality_lod_offset()
{
	auto v = cvTextureQuality->GetInt();
	if(v >= 4)
		return 0;
	else if(v == 3)
		return 1;
	else if(v == 2)
		return 2;
	else if(v == 1)
		return 3;
	return 5;
}

static auto cvTextureFiltering = pragma::console::get_client_con_var("cl_render_texture_filtering");
static void CVAR_CALLBACK_cl_render_texture_quality(pragma::NetworkState *, const pragma::console::ConVar &, int, int)
{
	if(pragma::get_client_state() == nullptr)
		return;
	pragma::get_cengine()->GetRenderContext().WaitIdle();
	prosper::Filter minFilter, magFilter;
	prosper::SamplerMipmapMode mipmapMode;
	UInt32 anisotropy;
	get_filter_mode(cvTextureFiltering->GetInt(), minFilter, magFilter, mipmapMode, anisotropy);
	auto lodOffset = get_quality_lod_offset();
	auto &materialManager = static_cast<pragma::material::CMaterialManager &>(pragma::get_client_state()->GetMaterialManager());
	auto &textureManager = materialManager.GetTextureManager();
	auto &sampler = static_cast<pragma::material::TextureLoader &>(textureManager.GetLoader()).GetTextureSampler();
	//auto &customSamplers = textureManager.GetCustomSamplers();
	auto fUpdateSampler = [anisotropy, mipmapMode, minFilter, magFilter, lodOffset](prosper::ISampler &sampler) {
		sampler.SetMaxAnisotropy(static_cast<float>(anisotropy));
		sampler.SetMipmapMode(mipmapMode);
		sampler.SetMinFilter(minFilter);
		sampler.SetMagFilter(magFilter);
		sampler.SetMinLod(static_cast<float>(lodOffset));
		sampler.Update();
	};
	fUpdateSampler(*sampler);
	/*for(auto &sampler : customSamplers)
	{
		if(sampler.expired())
			continue;
		fUpdateSampler(*sampler.lock());
	}*/
	materialManager.ReloadMaterialShaders(); // Make sure to reload descriptor sets (So samplers are updated)
}

namespace {
	auto UVN = pragma::console::client::register_variable_listener<int>("cl_render_texture_filtering", &CVAR_CALLBACK_cl_render_texture_quality);
}
namespace {
	auto UVN = pragma::console::client::register_variable_listener<int>("cl_render_texture_quality", &CVAR_CALLBACK_cl_render_texture_quality);
}
