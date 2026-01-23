// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:scripting.lua.libraries.util;

export import :entities.components.camera;
export import :entities.components.lights.light;
export import :rendering.lightmap_data_cache;

export namespace Lua {
	namespace util {
		namespace Client {
			DLLCLIENT int calc_world_direction_from_2d_coordinates(lua::State *l, pragma::CCameraComponent &hCam, const ::Vector2 &uv);
			DLLCLIENT int create_particle_tracer(lua::State *l);
			DLLCLIENT int create_muzzle_flash(lua::State *l);
			DLLCLIENT luabind::object create_giblet(GibletCreateInfo &createInfo);
			DLLCLIENT int export_map(lua::State *l);
			DLLCLIENT int import_model(lua::State *l);
			DLLCLIENT int import_gltf(lua::State *l);
			DLLCLIENT int export_texture(lua::State *l);
			DLLCLIENT int export_material(lua::State *l);
			DLLCLIENT std::string get_clipboard_string();
			DLLCLIENT void set_clipboard_string(const std::string &str);
			DLLCLIENT pragma::util::ParallelJob<std::shared_ptr<pragma::image::ImageBuffer>> bake_directional_lightmap_atlas(const std::vector<pragma::CLightComponent *> &lights, const std::vector<pragma::geometry::ModelSubMesh *> &meshes,
			  const std::vector<pragma::ecs::BaseEntity *> &entities, uint32_t width, uint32_t height, pragma::rendering::LightmapDataCache *optLightmapDataCache = nullptr);
		};
	};
};
