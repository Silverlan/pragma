/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_LUTIL_H__
#define __C_LUTIL_H__

#include "pragma/clientdefinitions.h"
#include "pragma/lua/c_ldefinitions.h"
#include <sharedutils/util_parallel_job.hpp>
#include <mathutil/uvec.h>

namespace pragma {
	struct LightmapDataCache;
};
namespace Lua {
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
			DLLCLIENT ::util::ParallelJob<std::shared_ptr<uimg::ImageBuffer>> bake_directional_lightmap_atlas(const std::vector<pragma::CLightComponent *> &lights, const std::vector<::ModelSubMesh *> &meshes, const std::vector<BaseEntity *> &entities, uint32_t width, uint32_t height,
			  ::pragma::LightmapDataCache *optLightmapDataCache = nullptr);
		};
	};
};

#endif
