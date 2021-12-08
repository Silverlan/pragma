/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/lua/classes/components/c_lentity_components.hpp"
#include "pragma/lua/libraries/lfile.h"
#include "pragma/lua/policies/file_policy.hpp"
#include "pragma/lua/policies/optional_policy.hpp"
#include "pragma/lua/policies/property_policy.hpp"
#include "pragma/entities/components/c_light_map_component.hpp"
#include "pragma/entities/components/c_light_map_receiver_component.hpp"
#include <pragma/lua/converters/optional_converter_t.hpp>
#include <pragma/lua/converters/property_converter_t.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/lua/lua_util_component.hpp>
#include <pragma/lua/lua_util_component_stream.hpp>
#include <util_image_buffer.hpp>
#include <prosper_command_buffer.hpp>
#include <texturemanager/texturemanager.h>
#include <cmaterialmanager.h>
#include <cmaterial_manager2.hpp>

extern DLLCLIENT CEngine *c_engine;

void Lua::Lightmap::register_class(lua_State *l,luabind::module_ &entsMod)
{
	auto defCLightMap = pragma::lua::create_entity_component_class<pragma::CLightMapComponent,pragma::BaseEntityComponent>("LightMapComponent");
	defCLightMap.scope[luabind::def("bake_lightmaps",&pragma::CLightMapComponent::BakeLightmaps)];
	defCLightMap.scope[luabind::def("import_lightmap_atlas",static_cast<bool(*)(const std::string&)>(&pragma::CLightMapComponent::ImportLightmapAtlas))];
	defCLightMap.scope[luabind::def("import_lightmap_atlas",static_cast<bool(*)(uimg::ImageBuffer&)>(&pragma::CLightMapComponent::ImportLightmapAtlas))];
	//defCLightMap.scope[luabind::def("import_lightmap_atlas",static_cast<bool(*)(VFilePtr)>(&pragma::CLightMapComponent::ImportLightmapAtlas),luabind::file_policy<1>{})];
	defCLightMap.def("GetLightmapTexture",static_cast<std::optional<std::shared_ptr<prosper::Texture>>(*)(lua_State*,pragma::CLightMapComponent&)>([](lua_State *l,pragma::CLightMapComponent &hLightMapC) -> std::optional<std::shared_ptr<prosper::Texture>> {
		auto lightMap = hLightMapC.GetLightMap();
		if(lightMap == nullptr)
			return {};
		return lightMap;
	}));
	defCLightMap.def("ConvertLightmapToBSPLuxelData",&pragma::CLightMapComponent::ConvertLightmapToBSPLuxelData);
	defCLightMap.def("UpdateLightmapUvBuffers",&pragma::CLightMapComponent::UpdateLightmapUvBuffers);
	defCLightMap.def("ReloadLightmapData",&pragma::CLightMapComponent::ReloadLightMapData);
	defCLightMap.def("SetLightmapAtlas",&pragma::CLightMapComponent::SetLightMapAtlas);
	defCLightMap.def("SetLightmapAtlas",static_cast<void(*)(lua_State*,pragma::CLightMapComponent&,const std::string &path)>([](lua_State *l,pragma::CLightMapComponent &hLightMapC,const std::string &path) {
		auto *nw = c_engine->GetNetworkState(l);

		auto &texManager = static_cast<msys::CMaterialManager&>(static_cast<ClientState*>(nw)->GetMaterialManager()).GetTextureManager();
		auto texture = texManager.LoadAsset(path);
		if(texture == nullptr)
			return;
		auto &vkTex = std::static_pointer_cast<Texture>(texture)->GetVkTexture();
		if(vkTex == nullptr)
			return;
		prosper::util::SamplerCreateInfo samplerCreateInfo {};
		auto sampler = c_engine->GetRenderContext().CreateSampler(samplerCreateInfo);
		vkTex->SetSampler(*sampler);
		hLightMapC.SetLightMapAtlas(vkTex);
	}));
	defCLightMap.def("SetExposure",&pragma::CLightMapComponent::SetLightMapExposure);
	defCLightMap.def("GetExposure",&pragma::CLightMapComponent::GetLightMapExposure);
	defCLightMap.def("GetExposureProperty",&pragma::CLightMapComponent::GetLightMapExposureProperty);

	auto defLightmapBakeSettings = luabind::class_<pragma::CLightMapComponent::LightmapBakeSettings>("BakeSettings");
	defLightmapBakeSettings.def(luabind::constructor<>());
	defLightmapBakeSettings.property("width",static_cast<luabind::object(*)(lua_State*,pragma::CLightMapComponent::LightmapBakeSettings&)>([](lua_State *l,pragma::CLightMapComponent::LightmapBakeSettings &bakeSettings) -> luabind::object {
		return bakeSettings.width.has_value() ? luabind::object{l,*bakeSettings.width} : luabind::object{};
	}),static_cast<void(*)(lua_State*,pragma::CLightMapComponent::LightmapBakeSettings&,luabind::object)>([](lua_State *l,pragma::CLightMapComponent::LightmapBakeSettings &bakeSettings,luabind::object o) {
		if(Lua::IsSet(l,2) == false)
		{
			bakeSettings.width = {};
			return;
		}
		bakeSettings.width = Lua::CheckNumber(l,2);
	}));
	defLightmapBakeSettings.property("height",static_cast<luabind::object(*)(lua_State*,pragma::CLightMapComponent::LightmapBakeSettings&)>([](lua_State *l,pragma::CLightMapComponent::LightmapBakeSettings &bakeSettings) -> luabind::object {
		return bakeSettings.height.has_value() ? luabind::object{l,*bakeSettings.height} : luabind::object{};
	}),static_cast<void(*)(lua_State*,pragma::CLightMapComponent::LightmapBakeSettings&,luabind::object)>([](lua_State *l,pragma::CLightMapComponent::LightmapBakeSettings &bakeSettings,luabind::object o) {
		if(Lua::IsSet(l,2) == false)
		{
			bakeSettings.height = {};
			return;
		}
		bakeSettings.height = Lua::CheckNumber(l,2);
	}));
	defLightmapBakeSettings.def_readwrite("samples",&pragma::CLightMapComponent::LightmapBakeSettings::samples);
	defLightmapBakeSettings.def_readwrite("globalLightIntensityFactor",&pragma::CLightMapComponent::LightmapBakeSettings::globalLightIntensityFactor);
	defLightmapBakeSettings.def_readwrite("denoise",&pragma::CLightMapComponent::LightmapBakeSettings::denoise);
	defLightmapBakeSettings.def_readwrite("createAsRenderJob",&pragma::CLightMapComponent::LightmapBakeSettings::createAsRenderJob);
	defLightmapBakeSettings.def_readwrite("rebuildUvAtlas",&pragma::CLightMapComponent::LightmapBakeSettings::rebuildUvAtlas);
	defLightmapBakeSettings.def_readwrite("exposure",&pragma::CLightMapComponent::LightmapBakeSettings::exposure);
	defLightmapBakeSettings.def("SetColorTransform",static_cast<void(*)(lua_State*,pragma::CLightMapComponent::LightmapBakeSettings&,const std::string&,const std::string&)>([](lua_State *l,pragma::CLightMapComponent::LightmapBakeSettings &bakeSettings,const std::string &config,const std::string &look) {
		bakeSettings.colorTransform = pragma::rendering::cycles::SceneInfo::ColorTransform{};
		bakeSettings.colorTransform->config = config;
		bakeSettings.colorTransform->look = look;
	}));
	defLightmapBakeSettings.def("ResetColorTransform",static_cast<void(*)(lua_State*,pragma::CLightMapComponent::LightmapBakeSettings&)>([](lua_State *l,pragma::CLightMapComponent::LightmapBakeSettings &bakeSettings) {
		bakeSettings.colorTransform = {};
	}));
	defCLightMap.scope[defLightmapBakeSettings];

	entsMod[defCLightMap];

	auto defCLightMapReceiver = pragma::lua::create_entity_component_class<pragma::CLightMapReceiverComponent,pragma::BaseEntityComponent>("LightMapReceiverComponent");
	defCLightMapReceiver.def("UpdateLightmapUvData",&pragma::CLightMapReceiverComponent::UpdateLightMapUvData);
	entsMod[defCLightMapReceiver];
}
