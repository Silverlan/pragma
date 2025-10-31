// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/lua/core.hpp"


export module pragma.client:scripting.lua.libraries.util;

import :entities.components.camera;
import :entities.components.lights.light;
import :rendering.lightmap_data_cache;

export namespace Lua {
	namespace util {
		namespace Client {
			DLLCLIENT int calc_world_direction_from_2d_coordinates(lua_State *l, pragma::CCameraComponent &hCam, const ::Vector2 &uv);
			DLLCLIENT int create_particle_tracer(lua_State *l);
			DLLCLIENT int create_muzzle_flash(lua_State *l);
			DLLCLIENT luabind::object create_giblet(GibletCreateInfo &createInfo);
			DLLCLIENT int export_map(lua_State *l);
			DLLCLIENT int import_model(lua_State *l);
			DLLCLIENT int import_gltf(lua_State *l);
			DLLCLIENT int export_texture(lua_State *l);
			DLLCLIENT int export_material(lua_State *l);
			DLLCLIENT std::string get_clipboard_string();
			DLLCLIENT void set_clipboard_string(const std::string &str);
			DLLCLIENT ::util::ParallelJob<std::shared_ptr<uimg::ImageBuffer>> bake_directional_lightmap_atlas(const std::vector<pragma::CLightComponent *> &lights, const std::vector<pragma::ModelSubMesh *> &meshes, const std::vector<pragma::ecs::BaseEntity *> &entities, uint32_t width, uint32_t height,
			  ::pragma::LightmapDataCache *optLightmapDataCache = nullptr);
		};
	};
};
