/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/lua/classes/components/c_lentity_components.hpp"
#include "pragma/lua/libraries/lfile.h"
#include "pragma/lua/policies/file_policy.hpp"
#include "pragma/lua/policies/optional_policy.hpp"
#include "pragma/lua/policies/property_policy.hpp"
#include "pragma/entities/components/c_light_map_component.hpp"
#include "pragma/entities/components/c_light_map_receiver_component.hpp"
#include "pragma/entities/components/c_light_map_data_cache_component.hpp"
#include "pragma/entities/components/lightmap_data_cache.hpp"
#include <pragma/lua/converters/optional_converter_t.hpp>
#include <pragma/lua/converters/property_converter_t.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/lua/converters/pair_converter_t.hpp>
#include <pragma/lua/converters/vector_converter_t.hpp>
#include <pragma/lua/lua_util_component.hpp>
#include <pragma/lua/lua_util_component_stream.hpp>
#include <pragma/lua/custom_constructor.hpp>
#include <util_image_buffer.hpp>
#include <prosper_command_buffer.hpp>
#include <luabind/copy_policy.hpp>
#include <texturemanager/texturemanager.h>
#include <cmaterialmanager.h>
#include <cmaterial_manager2.hpp>

extern DLLCLIENT CEngine *c_engine;

static void set_lightmap_texture(lua_State *l, pragma::CLightMapComponent &hLightMapC, const std::string &path, bool directional)
{
	auto *nw = c_engine->GetNetworkState(l);

	auto &texManager = static_cast<msys::CMaterialManager &>(static_cast<ClientState *>(nw)->GetMaterialManager()).GetTextureManager();
	auto texture = texManager.LoadAsset(path);
	if(texture == nullptr)
		return;
	auto &vkTex = std::static_pointer_cast<Texture>(texture)->GetVkTexture();
	if(vkTex == nullptr)
		return;
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	auto sampler = c_engine->GetRenderContext().CreateSampler(samplerCreateInfo);
	vkTex->SetSampler(*sampler);
	if(directional)
		hLightMapC.SetDirectionalLightMapAtlas(vkTex);
	else
		hLightMapC.SetLightMapAtlas(vkTex);
}

void Lua::Lightmap::register_class(lua_State *l, luabind::module_ &entsMod)
{
	auto defCLightMap = pragma::lua::create_entity_component_class<pragma::CLightMapComponent, pragma::BaseEntityComponent>("LightMapComponent");
	defCLightMap.add_static_constant("TEXTURE_DIFFUSE", umath::to_integral(pragma::CLightMapComponent::Texture::DiffuseMap));
	defCLightMap.add_static_constant("TEXTURE_DIFFUSE_DIRECT", umath::to_integral(pragma::CLightMapComponent::Texture::DiffuseDirectMap));
	defCLightMap.add_static_constant("TEXTURE_DIFFUSE_INDIRECT", umath::to_integral(pragma::CLightMapComponent::Texture::DiffuseIndirectMap));
	defCLightMap.add_static_constant("TEXTURE_DOMINANT_DIRECTION", umath::to_integral(pragma::CLightMapComponent::Texture::DominantDirectionMap));
	defCLightMap.add_static_constant("TEXTURE_COUNT", umath::to_integral(pragma::CLightMapComponent::Texture::Count));

	defCLightMap.scope[luabind::def("bake_lightmaps", &pragma::CLightMapComponent::BakeLightmaps)];
	defCLightMap.scope[luabind::def("import_lightmap_atlas", static_cast<bool (*)(const std::string &)>(&pragma::CLightMapComponent::ImportLightmapAtlas))];
	defCLightMap.scope[luabind::def("import_lightmap_atlas", static_cast<bool (*)(uimg::ImageBuffer &)>(&pragma::CLightMapComponent::ImportLightmapAtlas))];
	//defCLightMap.scope[luabind::def("import_lightmap_atlas",static_cast<bool(*)(VFilePtr)>(&pragma::CLightMapComponent::ImportLightmapAtlas),luabind::file_policy<1>{})];
	defCLightMap.def("GetLightmapTexture", static_cast<std::optional<std::shared_ptr<prosper::Texture>> (*)(lua_State *, pragma::CLightMapComponent &)>([](lua_State *l, pragma::CLightMapComponent &hLightMapC) -> std::optional<std::shared_ptr<prosper::Texture>> {
		auto lightMap = hLightMapC.GetLightMap();
		if(lightMap == nullptr)
			return {};
		return lightMap;
	}));
	defCLightMap.def(
	  "GetDirectionalLightmapTexture", +[](lua_State *l, pragma::CLightMapComponent &hLightMapC) -> std::optional<std::shared_ptr<prosper::Texture>> {
		  auto lightMap = hLightMapC.GetDirectionalLightMap();
		  if(lightMap == nullptr)
			  return {};
		  return lightMap;
	  });
	defCLightMap.def("GetLightmapMaterialName", &pragma::CLightMapComponent::GetLightMapMaterialName);
	defCLightMap.def("SetLightmapMaterial", &pragma::CLightMapComponent::SetLightMapMaterial);
	defCLightMap.def("ConvertLightmapToBSPLuxelData", &pragma::CLightMapComponent::ConvertLightmapToBSPLuxelData);
	defCLightMap.def("UpdateLightmapUvBuffers", &pragma::CLightMapComponent::UpdateLightmapUvBuffers);
	defCLightMap.def("ReloadLightmapData", &pragma::CLightMapComponent::ReloadLightMapData);
	defCLightMap.def("GetLightmapAtlas", &pragma::CLightMapComponent::GetLightMapAtlas);
	defCLightMap.def("GetDirectionalLightmapAtlas", &pragma::CLightMapComponent::GetDirectionalLightMapAtlas);
	defCLightMap.def("GetLightmapTexture", &pragma::CLightMapComponent::GetTexture, luabind::copy_policy<0> {});
	defCLightMap.def("SetLightmapAtlas", &pragma::CLightMapComponent::SetLightMapAtlas);
	defCLightMap.def(
	  "SetLightmapAtlas", +[](lua_State *l, pragma::CLightMapComponent &hLightMapC, const std::string &path) { set_lightmap_texture(l, hLightMapC, path, false); });
	defCLightMap.def("SetDirectionalLightmapAtlas", &pragma::CLightMapComponent::SetDirectionalLightMapAtlas);
	defCLightMap.def(
	  "SetDirectionalLightmapAtlas", +[](lua_State *l, pragma::CLightMapComponent &hLightMapC, const std::string &path) { set_lightmap_texture(l, hLightMapC, path, true); });
	defCLightMap.def("SetExposure", &pragma::CLightMapComponent::SetLightMapExposure);
	defCLightMap.def("GetExposure", &pragma::CLightMapComponent::GetLightMapExposure);
	defCLightMap.def("GetExposureProperty", &pragma::CLightMapComponent::GetLightMapExposureProperty);

	auto defLightmapBakeSettings = luabind::class_<pragma::CLightMapComponent::LightmapBakeSettings>("BakeSettings");
	defLightmapBakeSettings.def(luabind::constructor<>());
	defLightmapBakeSettings.property("width", static_cast<luabind::object (*)(lua_State *, pragma::CLightMapComponent::LightmapBakeSettings &)>([](lua_State *l, pragma::CLightMapComponent::LightmapBakeSettings &bakeSettings) -> luabind::object {
		return bakeSettings.width.has_value() ? luabind::object {l, *bakeSettings.width} : luabind::object {};
	}),
	  static_cast<void (*)(lua_State *, pragma::CLightMapComponent::LightmapBakeSettings &, luabind::object)>([](lua_State *l, pragma::CLightMapComponent::LightmapBakeSettings &bakeSettings, luabind::object o) {
		  if(Lua::IsSet(l, 2) == false) {
			  bakeSettings.width = {};
			  return;
		  }
		  bakeSettings.width = Lua::CheckNumber(l, 2);
	  }));
	defLightmapBakeSettings.property("height", static_cast<luabind::object (*)(lua_State *, pragma::CLightMapComponent::LightmapBakeSettings &)>([](lua_State *l, pragma::CLightMapComponent::LightmapBakeSettings &bakeSettings) -> luabind::object {
		return bakeSettings.height.has_value() ? luabind::object {l, *bakeSettings.height} : luabind::object {};
	}),
	  static_cast<void (*)(lua_State *, pragma::CLightMapComponent::LightmapBakeSettings &, luabind::object)>([](lua_State *l, pragma::CLightMapComponent::LightmapBakeSettings &bakeSettings, luabind::object o) {
		  if(Lua::IsSet(l, 2) == false) {
			  bakeSettings.height = {};
			  return;
		  }
		  bakeSettings.height = Lua::CheckNumber(l, 2);
	  }));
	defLightmapBakeSettings.def_readwrite("samples", &pragma::CLightMapComponent::LightmapBakeSettings::samples);
	defLightmapBakeSettings.def_readwrite("globalLightIntensityFactor", &pragma::CLightMapComponent::LightmapBakeSettings::globalLightIntensityFactor);
	defLightmapBakeSettings.def_readwrite("denoise", &pragma::CLightMapComponent::LightmapBakeSettings::denoise);
	defLightmapBakeSettings.def_readwrite("createAsRenderJob", &pragma::CLightMapComponent::LightmapBakeSettings::createAsRenderJob);
	defLightmapBakeSettings.def_readwrite("rebuildUvAtlas", &pragma::CLightMapComponent::LightmapBakeSettings::rebuildUvAtlas);
	defLightmapBakeSettings.def_readwrite("exposure", &pragma::CLightMapComponent::LightmapBakeSettings::exposure);
	defLightmapBakeSettings.def("SetColorTransform",
	  static_cast<void (*)(lua_State *, pragma::CLightMapComponent::LightmapBakeSettings &, const std::string &, const std::string &)>([](lua_State *l, pragma::CLightMapComponent::LightmapBakeSettings &bakeSettings, const std::string &config, const std::string &look) {
		  bakeSettings.colorTransform = pragma::rendering::cycles::SceneInfo::ColorTransform {};
		  bakeSettings.colorTransform->config = config;
		  bakeSettings.colorTransform->look = look;
	  }));
	defLightmapBakeSettings.def("ResetColorTransform", static_cast<void (*)(lua_State *, pragma::CLightMapComponent::LightmapBakeSettings &)>([](lua_State *l, pragma::CLightMapComponent::LightmapBakeSettings &bakeSettings) { bakeSettings.colorTransform = {}; }));
	defCLightMap.scope[defLightmapBakeSettings];

	auto defCache = luabind::class_<pragma::LightmapDataCache>("DataCache");
	defCache.scope[luabind::def(
	  "load", +[](lua_State *l, const std::string &path) -> Lua::var<pragma::LightmapDataCache, std::pair<bool, std::string>> {
		  auto cache = std::make_shared<pragma::LightmapDataCache>();
		  std::string err;
		  if(!pragma::LightmapDataCache::Load(path, *cache, err))
			  return luabind::object {l, std::pair<bool, std::string> {false, err}};
		  return luabind::object {l, cache};
	  })];
	defCache.def(
	  "AddInstanceData", +[](pragma::LightmapDataCache &cache, const std::string &entUuid, const std::string &model, const umath::Transform &pose, const std::string &meshUuid, const std::vector<Vector2> &uvs) {
		  auto tmpUvs = uvs;
		  cache.AddInstanceData(util::uuid_string_to_bytes(entUuid), model, pose, util::uuid_string_to_bytes(meshUuid), std::move(tmpUvs));
	  });
	defCache.def(
	  "GetInstanceIds", +[](pragma::LightmapDataCache &cache) -> std::vector<std::string> {
		  std::vector<std::string> uuids;
		  uuids.reserve(cache.cacheData.size());
		  for(auto &pair : cache.cacheData)
			  uuids.push_back(util::uuid_to_string(pair.first.uuid));
		  return uuids;
	  });
	defCache.def(
	  "GetInstancePose", +[](pragma::LightmapDataCache &cache, const std::string &uuid) -> std::optional<umath::Transform> {
		  auto it = cache.cacheData.find(pragma::LmUuid {util::uuid_string_to_bytes(uuid)});
		  if(it == cache.cacheData.end())
			  return {};
		  return it->second.pose;
	  });
	defCache.def(
	  "FindLightmapUvs", +[](pragma::LightmapDataCache &cache, const std::string &entUuid, const std::string &meshUuid) -> std::optional<std::vector<Vector2>> {
		  auto *uvs = cache.FindLightmapUvs(util::uuid_string_to_bytes(entUuid), util::uuid_string_to_bytes(meshUuid));
		  if(!uvs)
			  return {};
		  return *uvs;
	  });
	defCache.def(
	  "SaveAs", +[](lua_State *l, pragma::LightmapDataCache &cache, const std::string &path) -> Lua::var<bool, std::pair<bool, std::string>> {
		  std::string err;
		  auto res = cache.SaveAs(path, err);
		  if(res)
			  return luabind::object {l, res};
		  return luabind::object {l, std::pair<bool, std::string> {res, err}};
	  });
	defCache.def(
	  "SetLightmapEntity", +[](lua_State *l, pragma::LightmapDataCache &cache, const std::string &uuid) { cache.lightmapEntityId = util::uuid_string_to_bytes(uuid); });
	defCache.def(
	  "GetLightmapEntity", +[](lua_State *l, pragma::LightmapDataCache &cache) -> std::string { return util::uuid_to_string(cache.lightmapEntityId); });
	defCLightMap.scope[defCache];
	entsMod[defCLightMap];
	pragma::lua::define_custom_constructor<pragma::LightmapDataCache, +[]() -> std::shared_ptr<pragma::LightmapDataCache> { return std::make_shared<pragma::LightmapDataCache>(); }>(l);

	auto defCLightMapReceiver = pragma::lua::create_entity_component_class<pragma::CLightMapReceiverComponent, pragma::BaseEntityComponent>("LightMapReceiverComponent");
	defCLightMapReceiver.def("UpdateLightmapUvData", &pragma::CLightMapReceiverComponent::UpdateLightMapUvData);
	entsMod[defCLightMapReceiver];

	auto defCLmCache = pragma::lua::create_entity_component_class<pragma::CLightMapDataCacheComponent, pragma::BaseEntityComponent>("LightMapDataCacheComponent");
	defCLmCache.def("SetLightMapDataCachePath", &pragma::CLightMapDataCacheComponent::SetLightMapDataCachePath);
	defCLmCache.def("GetLightMapDataCachePath", &pragma::CLightMapDataCacheComponent::GetLightMapDataCachePath);
	defCLmCache.def(
	  "GetLightMapDataCacheFilePath", +[](const pragma::CLightMapDataCacheComponent &component) -> std::string { return pragma::LightmapDataCache::GetCacheFileName(component.GetLightMapDataCachePath());
	  });
	defCLmCache.def("GetLightMapDataCache", &pragma::CLightMapDataCacheComponent::GetLightMapDataCache);
	defCLmCache.def("ReloadCache", &pragma::CLightMapDataCacheComponent::ReloadCache);
	entsMod[defCLmCache];
}
