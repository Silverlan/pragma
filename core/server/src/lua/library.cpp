/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include <pragma/engine.h>
#include "pragma/game/s_game.h"
#include "pragma/lua/classes/lresource.h"
#include "pragma/lua/libraries/s_ldebugoverlay.h"
#include "pragma/lua/libraries/s_lai.h"
#include <pragma/lua/libraries/lutil.hpp>
#include "pragma/lua/libraries/s_lutil.h"
#include "pragma/lua/libraries/s_lsound.h"
#include "pragma/lua/libraries/s_ldebug.h"
#include "pragma/lua/libraries/lfile.h"
#include "pragma/model/model.h"
#include <pragma/asset/util_asset.hpp>
#include <pragma/debug/debug_render_info.hpp>
#include <pragma/util/util_splash_damage_info.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/lua/converters/vector_converter_t.hpp>
#include <pragma/util/giblet_create_info.hpp>
#include <pragma/util/bulletinfo.h>
#include <pragma/lua/libraries/lasset.hpp>
#include <pragma/lua/luaapi.h>
#include <pragma/lua/classes/lalsound.h>
#include <sharedutils/asset_loader/file_asset_manager.hpp>
#include <luainterface.hpp>
#include <fsys/ifile.hpp>

void SGame::RegisterLuaLibraries()
{
	Lua::util::register_library(GetLuaState());

	auto osMod = luabind::module(GetLuaState(), "os");
	Lua::util::register_os(GetLuaState(), osMod);

	auto utilMod = luabind::module(GetLuaState(), "util");
	Lua::util::register_shared(GetLuaState(), utilMod);
	utilMod[luabind::def("fire_bullets", static_cast<luabind::object (*)(lua_State *, const BulletInfo &, bool)>(Lua::util::Server::fire_bullets)), luabind::def("fire_bullets", static_cast<luabind::object (*)(lua_State *, const BulletInfo &)>(Lua::util::Server::fire_bullets)),
	  luabind::def("create_giblet", Lua::util::Server::create_giblet), luabind::def("create_explosion", Lua::util::Server::create_explosion), luabind::def("calc_world_direction_from_2d_coordinates", Lua::util::calc_world_direction_from_2d_coordinates)];

	LFile;
	Game::RegisterLuaLibraries();
	auto modAsset = luabind::module_(GetLuaState(), "asset");
	modAsset[luabind::def(
	           "load",
	           +[](lua_State *l, LFile &f, pragma::asset::Type type) -> Lua::var<bool, luabind::object> {
		           // See also core/client/src/lua/c_library.cpp
		           auto *manager = pragma::get_engine()->GetNetworkState(l)->GetAssetManager(type);
		           if(!manager)
			           return luabind::object {l, false};
		           auto fh = f.GetHandle();
		           if(!fh)
			           return luabind::object {l, false};
		           auto fp = std::make_unique<ufile::FileWrapper>(fh);
		           auto fileName = fp->GetFileName();
		           if(!fileName.has_value())
			           return luabind::object {l, false};
		           std::string ext;
		           if(ufile::get_extension(*fileName, &ext) == false)
			           return luabind::object {l, false};
		           auto loadInfo = manager->CreateDefaultLoadInfo();
		           loadInfo->flags |= util::AssetLoadFlags::DontCache | util::AssetLoadFlags::IgnoreCache;
		           auto asset = manager->LoadAsset(ufile::get_file_from_filename(*fileName), std::move(fp), ext, std::move(loadInfo));
		           switch(type) {
		           case pragma::asset::Type::Model:
			           return luabind::object {l, std::static_pointer_cast<Model>(asset)};
		           case pragma::asset::Type::Material:
			           return luabind::object {l, std::static_pointer_cast<Material>(asset)};
		           }
		           return luabind::object {};
	           }),
	  luabind::def(
	    "load",
	    +[](lua_State *l, const std::string &name, pragma::asset::Type type) -> Lua::var<bool, luabind::object> {
		    // See also core/client/src/lua/c_library.cpp
		    auto *manager = pragma::get_engine()->GetNetworkState(l)->GetAssetManager(type);
		    if(!manager)
			    return luabind::object {l, false};
		    auto asset = manager->LoadAsset(name);
		    switch(type) {
		    case pragma::asset::Type::Model:
			    return luabind::object {l, std::static_pointer_cast<Model>(asset)};
		    case pragma::asset::Type::Material:
			    return luabind::object {l, std::static_pointer_cast<Material>(asset)};
		    }
		    return luabind::object {};
	    }),
	  luabind::def(
	    "reload", +[](lua_State *l, const std::string &name, pragma::asset::Type type) -> Lua::var<bool, luabind::object> {
		    auto *manager = pragma::get_engine()->GetNetworkState(l)->GetAssetManager(type);
		    if(!manager)
			    return luabind::object {l, false};
		    auto asset = manager->ReloadAsset(name);
		    switch(type) {
		    case pragma::asset::Type::Model:
			    return luabind::object {l, std::static_pointer_cast<Model>(asset)};
		    case pragma::asset::Type::Material:
			    return luabind::object {l, std::static_pointer_cast<Material>(asset)};
		    }
		    return luabind::object {};
	    })];

	Lua::asset::register_library(GetLuaInterface(), true);

	auto resMod = luabind::module(GetLuaState(), "resource");
	resMod[luabind::def("add_file", static_cast<bool (*)(const std::string &, bool)>(Lua::resource::add_file)), luabind::def("add_file", static_cast<bool (*)(const std::string &)>(Lua::resource::add_file)), luabind::def("add_lua_file", Lua::resource::add_lua_file),
	  luabind::def("get_list", Lua::resource::get_list)];

	auto utilDebug = luabind::module(GetLuaState(), "debug");
	utilDebug[luabind::def("draw_point", Lua::DebugRenderer::Server::DrawPoint), luabind::def("draw_line", Lua::DebugRenderer::Server::DrawLine), luabind::def("draw_box", &Lua::DebugRenderer::Server::DrawBox),
	  luabind::def("draw_sphere", static_cast<void (*)(float, const DebugRenderInfo &)>(Lua::DebugRenderer::Server::DrawSphere)), luabind::def("draw_sphere", static_cast<void (*)(float, const DebugRenderInfo &, uint32_t)>(Lua::DebugRenderer::Server::DrawSphere)),
	  luabind::def("draw_cone", &Lua::DebugRenderer::Server::DrawCone), luabind::def("draw_truncated_cone", &Lua::DebugRenderer::Server::DrawTruncatedCone), luabind::def("draw_cylinder", &Lua::DebugRenderer::Server::DrawCylinder),
	  luabind::def("draw_pose", &Lua::DebugRenderer::Server::DrawAxis), luabind::def("draw_text", static_cast<void (*)(const std::string &, const Vector2 &, const DebugRenderInfo &)>(Lua::DebugRenderer::Server::DrawText)),
	  luabind::def("draw_text", static_cast<void (*)(const std::string &, float, const DebugRenderInfo &)>(Lua::DebugRenderer::Server::DrawText)), luabind::def("draw_path", &Lua::DebugRenderer::Server::DrawPath),
	  luabind::def("draw_spline", static_cast<void (*)(lua_State *, luabind::table<>, uint32_t, float, const DebugRenderInfo &)>(Lua::DebugRenderer::Server::DrawSpline)),
	  luabind::def("draw_spline", static_cast<void (*)(lua_State *, luabind::table<>, uint32_t, const DebugRenderInfo &)>(Lua::DebugRenderer::Server::DrawSpline)), luabind::def("draw_plane", &Lua::DebugRenderer::Server::DrawPlane), luabind::def("draw_mesh", &SGame::DrawMesh)];

	Lua::ai::server::register_library(GetLuaInterface());

	Lua::RegisterLibrary(GetLuaState(), "sound", {{"create", Lua::sound::Server::create}, LUA_LIB_SOUND_SHARED});
	Lua::sound::register_enums(GetLuaState());

	auto alSoundClassDef = luabind::class_<ALSound>("Source");
	Lua::ALSound::register_class(alSoundClassDef);

	auto soundMod = luabind::module(GetLuaState(), "sound");
	soundMod[alSoundClassDef];

	utilDebug[luabind::def("behavior_selector_type_to_string", Lua::debug::Server::behavior_selector_type_to_string), luabind::def("behavior_task_decorator_type_to_string", Lua::debug::Server::behavior_task_decorator_type_to_string),
	  luabind::def("behavior_task_result_to_string", Lua::debug::Server::behavior_task_result_to_string), luabind::def("behavior_task_type_to_string", Lua::debug::Server::behavior_task_type_to_string), luabind::def("disposition_to_string", Lua::debug::Server::disposition_to_string),
	  luabind::def("memory_type_to_string", Lua::debug::Server::memory_type_to_string), luabind::def("npc_state_to_string", Lua::debug::Server::npc_state_to_string), luabind::def("task_to_string", Lua::debug::Server::task_to_string)];
}
