/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/lua/classes/components/c_lentity_components.hpp"
#include "pragma/lua/libraries/lfile.h"
#include <util_image_buffer.hpp>
#include <prosper_command_buffer.hpp>

void Lua::Lightmap::register_class(lua_State *l,luabind::module_ &entsMod)
{
	auto defCLightMap = luabind::class_<CLightMapHandle,BaseEntityComponentHandle>("LightMapComponent");
	defCLightMap.scope[luabind::def("bake_lightmaps",static_cast<bool(*)(lua_State*,const pragma::CLightMapComponent::LightmapBakeSettings&)>([](lua_State *l,const pragma::CLightMapComponent::LightmapBakeSettings &bakeSettings) -> bool {
		return pragma::CLightMapComponent::BakeLightmaps(bakeSettings);
	}))];
	defCLightMap.scope[luabind::def("import_lightmap_atlas",static_cast<bool(*)(lua_State*,const std::string&)>([](lua_State *l,const std::string &path) -> bool {
		return pragma::CLightMapComponent::ImportLightmapAtlas(path);
	}))];
	defCLightMap.scope[luabind::def("import_lightmap_atlas",static_cast<bool(*)(lua_State*,uimg::ImageBuffer&)>([](lua_State *l,uimg::ImageBuffer &imgBuf) -> bool {
		return pragma::CLightMapComponent::ImportLightmapAtlas(imgBuf);
	}))];
	defCLightMap.scope[luabind::def("import_lightmap_atlas",static_cast<bool(*)(lua_State*,LFile&)>([](lua_State *l,LFile &f) -> bool {
		return pragma::CLightMapComponent::ImportLightmapAtlas(f.GetHandle());
	}))];
	defCLightMap.def("GetLightmapTexture",static_cast<void(*)(lua_State*,CLightMapHandle&)>([](lua_State *l,CLightMapHandle &hLightMapC) {
		pragma::Lua::check_component(l,hLightMapC);
		auto lightMap = hLightMapC->GetLightMap();
		if(lightMap == nullptr)
			return;
		Lua::Push<std::shared_ptr<prosper::Texture>>(l,lightMap);
		}));
	defCLightMap.def("ConvertLightmapToBSPLuxelData",static_cast<void(*)(lua_State*,CLightMapHandle&)>([](lua_State *l,CLightMapHandle &hLightMapC) {
		pragma::Lua::check_component(l,hLightMapC);
		hLightMapC->ConvertLightmapToBSPLuxelData();
		}));
	defCLightMap.def("UpdateLightmapUvBuffers",static_cast<void(*)(lua_State*,CLightMapHandle&)>([](lua_State *l,CLightMapHandle &hLightMapC) {
		pragma::Lua::check_component(l,hLightMapC);
		hLightMapC->UpdateLightmapUvBuffers();
	}));;
	defCLightMap.def("ReloadLightmapData",static_cast<void(*)(lua_State*,CLightMapHandle&)>([](lua_State *l,CLightMapHandle &hLightMapC) {
		pragma::Lua::check_component(l,hLightMapC);
		hLightMapC->ReloadLightMapData();
	}));
	defCLightMap.def("SetLightmapAtlas",static_cast<void(*)(lua_State*,CLightMapHandle&,prosper::Texture&)>([](lua_State *l,CLightMapHandle &hLightMapC,prosper::Texture &texture) {
		pragma::Lua::check_component(l,hLightMapC);
		hLightMapC->SetLightMapAtlas(texture.shared_from_this());
	}));
	defCLightMap.def("SetExposure",static_cast<void(*)(lua_State*,CLightMapHandle&,float)>([](lua_State *l,CLightMapHandle &hLightMapC,float exposure) {
		pragma::Lua::check_component(l,hLightMapC);
		hLightMapC->SetLightMapExposure(exposure);
	}));
	defCLightMap.def("GetExposure",static_cast<float(*)(lua_State*,CLightMapHandle&)>([](lua_State *l,CLightMapHandle &hLightMapC) -> float {
		pragma::Lua::check_component(l,hLightMapC);
		return hLightMapC->GetLightMapExposure();
	}));
	defCLightMap.def("GetExposureProperty",static_cast<void(*)(lua_State*,CLightMapHandle&)>([](lua_State *l,CLightMapHandle &hLightMapC) {
		pragma::Lua::check_component(l,hLightMapC);
		Lua::Property::push(l,*hLightMapC->GetLightMapExposureProperty());
	}));

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

	auto defCLightMapReceiver = luabind::class_<CLightMapReceiverHandle,BaseEntityComponentHandle>("LightMapReceiverComponent");
	defCLightMapReceiver.def("UpdateLightmapUvData",static_cast<void(*)(lua_State*,CLightMapReceiverHandle&)>([](lua_State *l,CLightMapReceiverHandle &hLightMapReceiverC) {
		pragma::Lua::check_component(l,hLightMapReceiverC);
		hLightMapReceiverC->UpdateLightMapUvData();
	}));
	entsMod[defCLightMapReceiver];
}
