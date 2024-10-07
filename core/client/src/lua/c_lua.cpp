/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"

namespace pragma::string {
	class Utf8String;
	class Utf8StringView;
	class Utf8StringArg;
};

#include "pragma/clientstate/clientstate.h"
#include "pragma/game/c_game.h"
#include "pragma/lua/libraries/c_lengine.h"
#include "pragma/lua/libraries/c_lgame.h"
#include "pragma/lua/libraries/lents.h"
#include "pragma/lua/libraries/c_lnetmessages.h"
#include "pragma/lua/libraries/c_lents.h"
#include "pragma/lua/libraries/c_lrender.h"
#include "pragma/lua/libraries/lengine.h"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include "pragma/rendering/c_rendermode.h"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include <pragma/lua/converters/optional_converter_t.hpp>
#include <pragma/lua/converters/pair_converter_t.hpp>
#include <pragma/lua/classes/lentity.h>
#include <pragma/lua/classes/ldef_entity.h>
#include <mathutil/glmutil.h>
#include "luasystem.h"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/model/c_modelmesh.h"
#include "cmaterialmanager.h"
#include <wgui/wgui.h>
#include <wgui/wibase.h>
#include "pragma/lua/libraries/lray.h"
#include "pragma/audio/c_alsound.h"
#include "pragma/lua/libraries/c_lglobal.h"
#include "pragma/lua/libraries/c_llocale.h"
#include "pragma/lua/libraries/lgame.h"
#include <sharedutils/util_string.h>
#include "pragma/lua/classes/c_lnetpacket.h"
#include "pragma/lua/classes/c_lcamera.h"
#include "pragma/lua/classes/components/c_lentity_components.hpp"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/entities/components/c_lua_component.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/renderers/raytracing_renderer.hpp"
#include "pragma/lua/classes/c_lentity.h"
#include "pragma/lua/classes/c_lua_entity.h"
#include <pragma/math/e_frustum.h>
#include <pragma/lua/lua_util_component.hpp>
#include <pragma/lua/classes/lproperty.hpp>
#include <pragma/lua/lua_entity_component.hpp>
#include <pragma/lua/sh_lua_component_wrapper.hpp>
#include <pragma/lua/lentity_component_lua.hpp>
#include <pragma/lua/sh_lua_component.hpp>
#include <pragma/lua/sh_lua_component_t.hpp>
#include <pragma/networking/resources.h>
#include <pragma/input/inputhelper.h>
#include <pragma/lua/libraries/lnet.hpp>
#include <luainterface.hpp>
#include <pragma/lua/lua_component_event.hpp>
#include <pragma/model/model.h>
#include <prosper_descriptor_set_group.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_render_pass.hpp>
#include <image/prosper_render_target.hpp>
#include <pragma/lua/libraries/lfile.h>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/localization.h>
#include <wgui/fontmanager.h>
#include <udm.hpp>

#undef LEFT
#undef RIGHT
#undef TOP
#undef BOTTOM
#undef NEAR
#undef FAR

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

namespace pragma {
	// Has to be in same namespace as class, otherwise luabind can't locate it
	static std::ostream &operator<<(std::ostream &stream, const pragma::CLuaBaseEntityComponent &component) { return ::operator<<(stream, static_cast<const pragma::BaseEntityComponent &>(component)); }
};

void Lua::register_shared_client_state(lua_State *l)
{
	Lua::RegisterLibrary(l, "locale", {{"get_text", Lua::Locale::get_text}, {"get_languages", Lua::Locale::get_languages}});
	auto modLocale = luabind::module_(l, "locale");
	modLocale[luabind::def("load", Lua::Locale::load), luabind::def("get_language", Lua::Locale::get_language), luabind::def("change_language", Lua::Locale::change_language), luabind::def("set_text", Lua::Locale::set_localization), luabind::def("localize", Lua::Locale::localize)];
	modLocale[luabind::def(
	  "get_used_characters", +[]() -> std::string { return ::Locale::GetUsedCharacters().cpp_str(); })];
	modLocale[luabind::def(
	  "load_all", +[]() { ::Locale::LoadAll(); })];
	modLocale[luabind::def("clear", Lua::Locale::clear)];
	modLocale[luabind::def("get_texts", Lua::Locale::get_texts)];
	modLocale[luabind::def(
	  "get_raw_text", +[](const std::string &id) -> std::optional<std::string> {
		  std::string text;
		  if(::Locale::GetRawText(id, text))
			  return text;
		  return {};
	  })];
	modLocale[luabind::def("parse", static_cast<Lua::opt<Lua::map<std::string, std::string>> (*)(lua_State *, const std::string &, const std::string &)>(Lua::Locale::parse))];
	modLocale[luabind::def("parse", static_cast<Lua::opt<Lua::map<std::string, std::string>> (*)(lua_State *, const std::string &)>(Lua::Locale::parse))];
}

void CGame::RegisterLua()
{
	GetLuaInterface().SetIdentifier("cl");

	Lua::engine::register_library(GetLuaState());
	auto modEngine = luabind::module_(GetLuaState(), "engine");
	modEngine[luabind::def("get_text_size", static_cast<Vector2i (*)(lua_State *, const std::string &, const std::string &)>(Lua::engine::get_text_size)),
	  luabind::def("get_text_size", static_cast<Vector2i (*)(lua_State *, const std::string &, const FontInfo &)>(Lua::engine::get_text_size)),

	  luabind::def("poll_console_output", Lua::engine::poll_console_output), luabind::def("library_exists", Lua::engine::LibraryExists), luabind::def("load_library", Lua::engine::LoadLibrary), luabind::def("unload_library", Lua::engine::UnloadLibrary),
	  luabind::def("is_library_loaded", Lua::engine::IsLibraryLoaded), luabind::def("get_info", Lua::engine::get_info)];

	Lua::RegisterLibrary(GetLuaState(), "game",
	  {
	    //{"create_light",Lua::engine::CreateLight},
	    //{"remove_lights",Lua::engine::RemoveLights},
	    //{"create_sprite",Lua::engine::CreateSprite},
	    {"create_particle_system", &Lua::engine::create_particle_system}, {"save_particle_system", &Lua::engine::save_particle_system},

	    {"open_dropped_file", Lua::game::Client::open_dropped_file}, {"set_gravity", Lua::game::Client::set_gravity}, {"get_gravity", Lua::game::Client::get_gravity}, {"load_model", Lua::game::Client::load_model}, {"create_model", Lua::game::Client::create_model},
	    {"get_action_input", Lua::game::Client::get_action_input}, {"set_action_input", Lua::game::Client::set_action_input}, {"queue_scene_for_rendering", Lua::game::Client::queue_scene_for_rendering}, {"render_scenes", Lua::game::Client::render_scenes},
	    {"set_debug_render_filter", Lua::game::Client::set_debug_render_filter}, {"update_render_buffers", Lua::game::Client::update_render_buffers}, {"get_render_scene", Lua::game::Client::get_render_scene}, {"get_render_scene_camera", Lua::game::Client::get_render_scene_camera},
	    {"get_scene", Lua::game::Client::get_scene}, {"get_scene_by_index", Lua::game::Client::get_scene_by_index}, {"get_primary_camera", Lua::game::Client::get_scene_camera}, {"get_draw_command_buffer", Lua::game::Client::get_draw_command_buffer},
	    {"get_setup_command_buffer", Lua::game::Client::get_setup_command_buffer}, {"flush_setup_command_buffer", Lua::game::Client::flush_setup_command_buffer}, {"get_camera_transform", Lua::game::Client::get_camera_position},
	    {"get_render_clip_plane", Lua::game::Client::get_render_clip_plane}, {"set_render_clip_plane", Lua::game::Client::set_render_clip_plane}, {"build_reflection_probes", Lua::game::Client::build_reflection_probes},

	    {"set_render_stats_enabled", Lua::game::Client::set_render_stats_enabled}, {"get_queued_render_scenes", Lua::game::Client::get_queued_render_scenes},

	    {"get_debug_buffer", Lua::game::Client::get_debug_buffer}, {"get_time_buffer", Lua::game::Client::get_time_buffer}, {"get_csm_buffer", Lua::game::Client::get_csm_buffer}, {"get_render_settings_descriptor_set", Lua::game::Client::get_render_settings_descriptor_set},
	    {"set_default_game_render_enabled", static_cast<int32_t (*)(lua_State *)>([](lua_State *l) -> int32_t {
		     c_game->SetDefaultGameRenderEnabled(Lua::CheckBool(l, 1));
		     return 0;
	     })},
	    {"is_default_game_render_enabled", static_cast<int32_t (*)(lua_State *)>([](lua_State *l) -> int32_t {
		     Lua::PushBool(l, c_game->IsDefaultGameRenderEnabled());
		     return 1;
	     })},

	    {"get_render_mask", static_cast<int32_t (*)(lua_State *)>([](lua_State *l) -> int32_t {
		     auto mask = c_game->GetRenderMask(Lua::CheckString(l, 1));
		     Lua::Push(l, mask);
		     return 1;
	     })},
	    {"find_render_mask_name", static_cast<int32_t (*)(lua_State *)>([](lua_State *l) -> int32_t {
		     auto *name = c_game->FindRenderMaskName(Lua::Check<pragma::rendering::RenderMask>(l, 1));
		     if(!name)
			     return 0;
		     Lua::PushString(l, *name);
		     return 1;
	     })},
	    {"register_render_mask", static_cast<int32_t (*)(lua_State *)>([](lua_State *l) -> int32_t {
		     auto inclusiveByDefault = true;
		     if(Lua::IsSet(l, 2))
			     inclusiveByDefault = Lua::CheckBool(l, 2);
		     auto mask = c_game->RegisterRenderMask(Lua::CheckString(l, 1), inclusiveByDefault);
		     Lua::Push(l, mask);
		     return 1;
	     })},
	    {"register_particle_system",
	      +[](lua_State *l) -> int32_t {
		      std::string ptName = Lua::CheckString(l, 1);
		      auto &udmData = Lua::Check<udm::LinkedPropertyWrapper>(l, 2);
		      std::string err;
		      auto res = pragma::CParticleSystemComponent::InitializeFromAssetData(ptName, udmData, err);
		      Lua::PushBool(l, res);
		      return 1;
	      }},
	    {"wait_for_frames",
	      +[](lua_State *l) -> int32_t {
		      size_t numFrames = Lua::CheckInt(l, 1);
		      if(numFrames < 1)
			      numFrames = 1;
		      Lua::CheckFunction(l, 2);
		      auto waitForThink = false;
		      if(Lua::IsSet(l, 3))
			      waitForThink = Lua::CheckBool(l, 3);
		      luabind::object func {luabind::from_stack(l, 2)};
		      auto cb = FunctionCallback<void>::Create(nullptr);
		      static_cast<Callback<void> *>(cb.get())->SetFunction([l, cb, numFrames, func, waitForThink]() mutable {
			      --numFrames;
			      if(numFrames == 0) {
				      if(waitForThink) {
					      auto cbThink = FunctionCallback<void>::Create(nullptr);
					      static_cast<Callback<void> *>(cbThink.get())->SetFunction([l, cbThink, func]() mutable {
						      Lua::CallFunction(l, [&func](lua_State *l) -> Lua::StatusCode {
							      func.push(l);
							      return Lua::StatusCode::Ok;
						      });
						      if(cbThink.IsValid())
							      cbThink.Remove();
					      });
					      c_game->AddCallback("Think", cbThink);
				      }
				      else {
					      Lua::CallFunction(l, [&func](lua_State *l) -> Lua::StatusCode {
						      func.push(l);
						      return Lua::StatusCode::Ok;
					      });
				      }
				      if(cb.IsValid())
					      cb.Remove();
			      }
		      });
		      cb = c_game->AddCallback("PostRenderScenes", cb);
		      Lua::Push<CallbackHandle>(l, cb);
		      return 1;
	      }}

	    /*{"debug_vehicle",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) -> int32_t {
			Con::cout<<"Creating vehicle..."<<Con::endl;
			static auto vhc = std::make_shared<debug::Vehicle>();
			return 0;
		})}*/
	  });
	auto modGame = luabind::module_(GetLuaState(), "game");
	Lua::game::register_shared_functions(GetLuaState(), modGame);
	modGame[luabind::def("load_material", static_cast<Material *(*)(lua_State *, const std::string &, bool, bool)>(Lua::engine::load_material)), luabind::def("load_material", static_cast<Material *(*)(lua_State *, const std::string &, bool)>(Lua::engine::load_material)),
	  luabind::def("load_material", static_cast<Material *(*)(lua_State *, const std::string &)>(Lua::engine::load_material)),
	  luabind::def("load_texture", static_cast<std::shared_ptr<prosper::Texture> (*)(lua_State *, const std::string &, util::AssetLoadFlags)>(Lua::engine::load_texture)),
	  luabind::def("load_texture", static_cast<std::shared_ptr<prosper::Texture> (*)(lua_State *, const std::string &)>(Lua::engine::load_texture)),
	  luabind::def("load_texture", static_cast<std::shared_ptr<prosper::Texture> (*)(lua_State *, const LFile &, const std::string &, util::AssetLoadFlags loadFlags)>(Lua::engine::load_texture)),
	  luabind::def("load_texture", static_cast<std::shared_ptr<prosper::Texture> (*)(lua_State *, const LFile &, const std::string &)>(Lua::engine::load_texture)),
	  luabind::def("load_texture", static_cast<std::shared_ptr<prosper::Texture> (*)(lua_State *, const LFile &, util::AssetLoadFlags)>(Lua::engine::load_texture)),
	  luabind::def("load_texture", static_cast<std::shared_ptr<prosper::Texture> (*)(lua_State *, const LFile &)>(Lua::engine::load_texture)),
	  luabind::def("create_material", static_cast<std::shared_ptr<Material> (*)(const std::string &, const std::string &)>(Lua::engine::create_material)), luabind::def("create_material", static_cast<std::shared_ptr<Material> (*)(const std::string &)>(Lua::engine::create_material)),
	  luabind::def("get_material", static_cast<Material *(*)(const std::string &)>(Lua::engine::get_material)), luabind::def("precache_model", static_cast<void (*)(lua_State *, const std::string &)>(Lua::engine::precache_model)),
	  luabind::def("precache_material", static_cast<void (*)(lua_State *, const std::string &)>(Lua::engine::precache_material)), luabind::def("get_error_material", Lua::engine::get_error_material), luabind::def("clear_unused_materials", Lua::engine::clear_unused_materials),
	  luabind::def("precache_particle_system", static_cast<bool (*)(lua_State *, const std::string &, bool)>(Lua::engine::precache_particle_system)), luabind::def("precache_particle_system", static_cast<bool (*)(lua_State *, const std::string &)>(Lua::engine::precache_particle_system)),
	  luabind::def("load_sound_scripts", static_cast<void (*)(lua_State *, const std::string &, bool)>(Lua::engine::LoadSoundScripts)), luabind::def("load_sound_scripts", static_cast<void (*)(lua_State *, const std::string &)>(Lua::engine::LoadSoundScripts)),
	  luabind::def("get_model", Lua::engine::get_model), luabind::def("get_number_of_scenes_queued_for_rendering", &CGame::GetNumberOfScenesQueuedForRendering), luabind::def("get_queued_scene_render_info", &CGame::GetQueuedSceneRenderInfo),

	  luabind::def("set_gameplay_control_camera", &CGame::SetGameplayControlCamera), luabind::def("reset_gameplay_control_camera", &CGame::ResetGameplayControlCamera), luabind::def("get_gameplay_control_camera", &CGame::GetGameplayControlCamera),
	  luabind::def("clear_gameplay_control_camera", &CGame::ClearGameplayControlCamera),
	  luabind::def(
	    "get_primary_camera_render_mask", +[]() -> std::pair<::pragma::rendering::RenderMask, ::pragma::rendering::RenderMask> {
		    auto inclusionMask = ::pragma::rendering::RenderMask::None;
		    auto exclusionMask = ::pragma::rendering::RenderMask::None;
		    c_game->GetPrimaryCameraRenderMask(inclusionMask, exclusionMask);
		    return {inclusionMask, exclusionMask};
	    })];

	Lua::ents::register_library(GetLuaState());
	auto &modEnts = GetLuaInterface().RegisterLibrary("ents",
	  {{"get_local_player", Lua::ents::Client::get_local_player}, {"get_listener", Lua::ents::Client::get_listener}, {"get_view_body", Lua::ents::Client::get_view_body}, {"get_view_model", Lua::ents::Client::get_view_model}, {"get_instance_buffer", Lua::ents::Client::get_instance_buffer},
	    {"get_instance_bone_buffer", Lua::ents::Client::get_instance_bone_buffer}, {"register_component", Lua::ents::register_component<pragma::CLuaBaseEntityComponent>}, {"create_camera", static_cast<int32_t (*)(lua_State *)>([](lua_State *l) -> int32_t {
		                                                                                                                                                                        auto aspectRatio = Lua::CheckNumber(l, 1);
		                                                                                                                                                                        auto fov = Lua::CheckNumber(l, 2);
		                                                                                                                                                                        auto nearZ = Lua::CheckNumber(l, 3);
		                                                                                                                                                                        auto farZ = Lua::CheckNumber(l, 4);
		                                                                                                                                                                        auto *cam = c_game->CreateCamera(aspectRatio, fov, nearZ, farZ);
		                                                                                                                                                                        if(cam == nullptr)
			                                                                                                                                                                        return 0;
		                                                                                                                                                                        cam->PushLuaObject(l);
		                                                                                                                                                                        return 1;
	                                                                                                                                                                        })},
	    {"create_scene", Lua::game::Client::create_scene}});
	modEnts[luabind::def("register_component_event", &Lua::ents::register_component_event)];

	auto entityClassDef = luabind::class_<BaseEntity>("BaseEntityBase");
	Lua::Entity::register_class(entityClassDef);
	modEnts[entityClassDef];

	auto cEntityClassDef = luabind::class_<CBaseEntity, BaseEntity>("Entity");
	Lua::Entity::Client::register_class(cEntityClassDef);
	modEnts[cEntityClassDef];

	//auto tmp = luabind::class_<EntityHandle>("EntityOld");
	//modEnts[tmp];

	auto classDefBase = luabind::class_<CLuaEntity, luabind::bases<CBaseEntity>, pragma::lua::CLuaEntityHolder>("BaseEntity");
	classDefBase.def(luabind::constructor<>());
	// classDefBase.def(luabind::tostring(luabind::self));
	//classDefBase.def(luabind::constructor<>());
	classDefBase.def("Initialize", &CLuaEntity::LuaInitialize, &CLuaEntity::default_Initialize);
	//classDefBase.def("ReceiveNetEvent",&SLuaEntityWrapper::ReceiveNetEvent,&SLuaBaseEntityWrapper::default_ReceiveNetEvent);
	modEnts[classDefBase];

	auto modNet = luabind::module(GetLuaState(), "net");
	modNet[luabind::def("send", &Lua::net::client::send), luabind::def("receive", &Lua::net::client::receive), luabind::def("register_event", &Lua::net::register_event)];

	auto netPacketClassDef = luabind::class_<NetPacket>("Packet");
	Lua::NetPacket::Client::register_class(netPacketClassDef);
	netPacketClassDef.def("WritePlayer", static_cast<void (*)(lua_State *, ::NetPacket &, util::WeakHandle<pragma::CPlayerComponent> &)>([](lua_State *l, ::NetPacket &packet, util::WeakHandle<pragma::CPlayerComponent> &pl) { nwm::write_player(packet, pl.get()); }));
	netPacketClassDef.def("WritePlayer", static_cast<void (*)(lua_State *, ::NetPacket &, EntityHandle &)>([](lua_State *l, ::NetPacket &packet, EntityHandle &hEnt) { nwm::write_player(packet, hEnt.get()); }));
	netPacketClassDef.def("ReadPlayer", static_cast<void (*)(lua_State *, ::NetPacket &)>([](lua_State *l, ::NetPacket &packet) {
		auto *pl = static_cast<pragma::CPlayerComponent *>(nwm::read_player(packet));
		if(pl == nullptr)
			return;
		pl->PushLuaObject(l);
	}));
	modNet[netPacketClassDef];
	Lua::net::RegisterLibraryEnums(GetLuaState());

	Lua::register_shared_client_state(GetLuaState());

	Game::RegisterLua();
	/*lua_bind(
		luabind::class_<ListenerHandle,EntityHandle>("Listener")
		.def("SetGain",&Lua_Listener_SetGain)
		.def("GetGain",&Lua_Listener_GetGain)
	);*/

	lua_pushboolean(GetLuaState(), 1);
	lua_setglobal(GetLuaState(), "CLIENT");

	lua_pushboolean(GetLuaState(), 0);
	lua_setglobal(GetLuaState(), "SERVER");

	auto modTime = luabind::module_(GetLuaState(), "time");
	modTime[luabind::def("server_time", Lua::ServerTime), luabind::def("frame_time", Lua::FrameTime)];

	Lua::RegisterLibraryEnums(GetLuaState(), "sound",
	  {{"CHANNEL_CONFIG_MONO", umath::to_integral(al::ChannelConfig::Mono)}, {"CHANNEL_CONFIG_STEREO", umath::to_integral(al::ChannelConfig::Stereo)}, {"CHANNEL_CONFIG_REAR", umath::to_integral(al::ChannelConfig::Rear)}, {"CHANNEL_CONFIG_QUAD", umath::to_integral(al::ChannelConfig::Quad)},
	    {"CHANNEL_CONFIG_X51", umath::to_integral(al::ChannelConfig::X51)}, {"CHANNEL_CONFIG_X61", umath::to_integral(al::ChannelConfig::X61)}, {"CHANNEL_CONFIG_X71", umath::to_integral(al::ChannelConfig::X71)},
	    {"CHANNEL_CONFIG_BFORMAT_2D", umath::to_integral(al::ChannelConfig::BFormat2D)}, {"CHANNEL_CONFIG_BFORMAT_3D", umath::to_integral(al::ChannelConfig::BFormat3D)},

	    {"SAMPLE_TYPE_UINT8", umath::to_integral(al::SampleType::UInt8)}, {"SAMPLE_TYPE_INT16", umath::to_integral(al::SampleType::Int16)}, {"SAMPLE_TYPE_FLOAT32", umath::to_integral(al::SampleType::Float32)}, {"SAMPLE_TYPE_MULAW", umath::to_integral(al::SampleType::Mulaw)}});

	Lua::RegisterLibraryEnums(GetLuaState(), "time", {{"TIMER_TYPE_SERVERTIME", umath::to_integral(TimerType::ServerTime)}});

	Lua::RegisterLibraryEnums(GetLuaState(), "file", {{"SEARCH_RESOURCES", FSYS_SEARCH_RESOURCES}});

	Lua::RegisterLibraryEnums(GetLuaState(), "geometry",
	  {{"FrustumPlane_LEFT", umath::to_integral(FrustumPlane::Left)}, {"FrustumPlane_RIGHT", umath::to_integral(FrustumPlane::Right)}, {"FrustumPlane_TOP", umath::to_integral(FrustumPlane::Top)}, {"FrustumPlane_BOTTOM", umath::to_integral(FrustumPlane::Bottom)},
	    {"FrustumPlane_NEAR", umath::to_integral(FrustumPlane::Near)}, {"FrustumPlane_FAR", umath::to_integral(FrustumPlane::Far)}, {"FrustumPlane_COUNT", umath::to_integral(FrustumPlane::Count)},

	    {"FrustumPoint_FarBottomLeft", umath::to_integral(FrustumPoint::FarBottomLeft)}, {"FrustumPoint_FAR_TOP_LEFT", umath::to_integral(FrustumPoint::FarTopLeft)}, {"FrustumPoint_FAR_TOP_RIGHT", umath::to_integral(FrustumPoint::FarTopRight)},
	    {"FrustumPoint_FAR_BOTTOM_RIGHT", umath::to_integral(FrustumPoint::FarBottomRight)}, {"FrustumPoint_NearBottomLeft", umath::to_integral(FrustumPoint::NearBottomLeft)}, {"FrustumPoint_NearTopLeft", umath::to_integral(FrustumPoint::NearTopLeft)},
	    {"FrustumPoint_NearTopRight", umath::to_integral(FrustumPoint::NearTopRight)}, {"FrustumPoint_NearBottomRight", umath::to_integral(FrustumPoint::NearBottomRight)}});

	Lua::RegisterLibraryEnums(GetLuaState(), "gui",
	  {{"CURSOR_SHAPE_DEFAULT", umath::to_integral(GLFW::Cursor::Shape::Default)}, {"CURSOR_SHAPE_HIDDEN", umath::to_integral(GLFW::Cursor::Shape::Hidden)}, {"CURSOR_SHAPE_ARROW", umath::to_integral(GLFW::Cursor::Shape::Arrow)},
	    {"CURSOR_SHAPE_IBEAM", umath::to_integral(GLFW::Cursor::Shape::IBeam)}, {"CURSOR_SHAPE_CROSSHAIR", umath::to_integral(GLFW::Cursor::Shape::Crosshair)}, {"CURSOR_SHAPE_HAND", umath::to_integral(GLFW::Cursor::Shape::Hand)},
	    {"CURSOR_SHAPE_HRESIZE", umath::to_integral(GLFW::Cursor::Shape::HResize)}, {"CURSOR_SHAPE_VRESIZE", umath::to_integral(GLFW::Cursor::Shape::VResize)},

	    {"CURSOR_MODE_DISABLED", umath::to_integral(GLFW::CursorMode::Disabled)}, {"CURSOR_MODE_HIDDEN", umath::to_integral(GLFW::CursorMode::Hidden)}, {"CURSOR_MODE_NORMAL", umath::to_integral(GLFW::CursorMode::Normal)}});

	Lua::RegisterLibraryEnums(GetLuaState(), "game",
	  {{"RENDER_FLAG_NONE", 0}, {"RENDER_FLAG_BIT_WORLD", umath::to_integral(RenderFlags::World)}, {"RENDER_FLAG_BIT_VIEW", umath::to_integral(RenderFlags::View)}, {"RENDER_FLAG_BIT_SKYBOX", umath::to_integral(RenderFlags::Skybox)},
	    {"RENDER_FLAG_BIT_SHADOWS", umath::to_integral(RenderFlags::Shadows)}, {"RENDER_FLAG_BIT_PARTICLES", umath::to_integral(RenderFlags::Particles)}, {"RENDER_FLAG_BIT_GLOW", umath::to_integral(RenderFlags::Glow)}, {"RENDER_FLAG_BIT_DEBUG", umath::to_integral(RenderFlags::Debug)},
	    {"RENDER_FLAG_ALL", umath::to_integral(RenderFlags::All)}, {"RENDER_FLAG_REFLECTION_BIT", umath::to_integral(RenderFlags::Reflection)}, {"RENDER_FLAG_WATER_BIT", umath::to_integral(RenderFlags::Water)}, {"RENDER_FLAG_STATIC_BIT", umath::to_integral(RenderFlags::Static)},
	    {"RENDER_FLAG_DYNAMIC_BIT", umath::to_integral(RenderFlags::Dynamic)}, {"RENDER_FLAG_TRANSLUCENT_BIT", umath::to_integral(RenderFlags::Translucent)}, {"RENDER_FLAG_HDR_BIT", umath::to_integral(RenderFlags::HDR)},
	    {"RENDER_FLAG_PARTICLE_DEPTH_BIT", umath::to_integral(RenderFlags::ParticleDepth)},

	    {"ASSET_LOAD_FLAG_NONE", umath::to_integral(util::AssetLoadFlags::None)}, {"ASSET_LOAD_FLAG_DONT_CACHE", umath::to_integral(util::AssetLoadFlags::DontCache)}, {"ASSET_LOAD_FLAG_IGNORE_CACHE", umath::to_integral(util::AssetLoadFlags::IgnoreCache)}});

	auto gameMod = luabind::module(GetLuaState(), "game");
	RegisterLuaGameClasses(gameMod);

	// Needs to be registered AFTER RegisterLuaGameClasses has been called!
	Lua::register_base_entity_component(modEnts);
	auto defEntCmp = pragma::lua::create_entity_component_class<pragma::CLuaBaseEntityComponent, luabind::bases<pragma::BaseLuaBaseEntityComponent, pragma::BaseEntityComponent>, pragma::lua::CLuaBaseEntityComponentHolder>("BaseEntityComponent");
	defEntCmp.def(luabind::constructor<CBaseEntity &>());
	defEntCmp.def("ReceiveData", static_cast<void (*)(lua_State *, pragma::CLuaBaseEntityComponent &, NetPacket)>([](lua_State *l, pragma::CLuaBaseEntityComponent &hComponent, NetPacket packet) {

	}));
	defEntCmp.def("ReceiveNetEvent", static_cast<void (*)(lua_State *, pragma::CLuaBaseEntityComponent &, uint32_t, NetPacket)>([](lua_State *l, pragma::CLuaBaseEntityComponent &hComponent, uint32_t evId, NetPacket packet) {

	}));
	defEntCmp.def("ReceiveSnapshotData", static_cast<void (*)(lua_State *, pragma::CLuaBaseEntityComponent &, NetPacket)>([](lua_State *l, pragma::CLuaBaseEntityComponent &hComponent, NetPacket packet) {

	}));
	register_shared_lua_component_methods<pragma::CLuaBaseEntityComponent>(defEntCmp);
	modEnts[defEntCmp];

	auto _G = luabind::globals(GetLuaState());
	_G["BaseEntityComponent"] = _G["ents"]["BaseEntityComponent"];

	Lua::RegisterLibraryEnums(GetLuaState(), "input",
	  {{"STATE_PRESS", GLFW_PRESS}, {"STATE_RELEASE", GLFW_RELEASE}, {"STATE_REPEAT", GLFW_REPEAT},

	    {"MOUSE_BUTTON_1", GLFW_MOUSE_BUTTON_1}, {"MOUSE_BUTTON_2", GLFW_MOUSE_BUTTON_2}, {"MOUSE_BUTTON_3", GLFW_MOUSE_BUTTON_3}, {"MOUSE_BUTTON_4", GLFW_MOUSE_BUTTON_4}, {"MOUSE_BUTTON_5", GLFW_MOUSE_BUTTON_5}, {"MOUSE_BUTTON_6", GLFW_MOUSE_BUTTON_6},
	    {"MOUSE_BUTTON_7", GLFW_MOUSE_BUTTON_7}, {"MOUSE_BUTTON_8", GLFW_MOUSE_BUTTON_8}, {"MOUSE_BUTTON_LEFT", GLFW_MOUSE_BUTTON_LEFT}, {"MOUSE_BUTTON_RIGHT", GLFW_MOUSE_BUTTON_RIGHT}, {"MOUSE_BUTTON_MIDDLE", GLFW_MOUSE_BUTTON_MIDDLE},

	    {"MOD_NONE", 0}, {"MOD_SHIFT", GLFW_MOD_SHIFT}, {"MOD_CONTROL", GLFW_MOD_CONTROL}, {"MOD_ALT", GLFW_MOD_ALT}, {"MOD_SUPER", GLFW_MOD_SUPER},

	    {"KEY_UNKNOWN", GLFW_KEY_UNKNOWN}, {"KEY_SPACE", GLFW_KEY_SPACE}, {"KEY_APOSTROPHE", GLFW_KEY_APOSTROPHE}, {"KEY_COMMA", GLFW_KEY_COMMA}, {"KEY_MINUS", GLFW_KEY_MINUS}, {"KEY_PERIOD", GLFW_KEY_PERIOD}, {"KEY_SLASH", GLFW_KEY_SLASH}, {"KEY_0", GLFW_KEY_0}, {"KEY_1", GLFW_KEY_1},
	    {"KEY_2", GLFW_KEY_2}, {"KEY_3", GLFW_KEY_3}, {"KEY_4", GLFW_KEY_4}, {"KEY_5", GLFW_KEY_5}, {"KEY_6", GLFW_KEY_6}, {"KEY_7", GLFW_KEY_7}, {"KEY_8", GLFW_KEY_8}, {"KEY_9", GLFW_KEY_9}, {"KEY_SEMICOLON", GLFW_KEY_SEMICOLON}, {"KEY_EQUAL", GLFW_KEY_EQUAL}, {"KEY_A", GLFW_KEY_A},
	    {"KEY_B", GLFW_KEY_B}, {"KEY_C", GLFW_KEY_C}, {"KEY_D", GLFW_KEY_D}, {"KEY_E", GLFW_KEY_E}, {"KEY_F", GLFW_KEY_F}, {"KEY_G", GLFW_KEY_G}, {"KEY_H", GLFW_KEY_H}, {"KEY_I", GLFW_KEY_I}, {"KEY_J", GLFW_KEY_J}, {"KEY_K", GLFW_KEY_K}, {"KEY_L", GLFW_KEY_L}, {"KEY_M", GLFW_KEY_M},
	    {"KEY_N", GLFW_KEY_N}, {"KEY_O", GLFW_KEY_O}, {"KEY_P", GLFW_KEY_P}, {"KEY_Q", GLFW_KEY_Q}, {"KEY_R", GLFW_KEY_R}, {"KEY_S", GLFW_KEY_S}, {"KEY_T", GLFW_KEY_T}, {"KEY_U", GLFW_KEY_U}, {"KEY_V", GLFW_KEY_V}, {"KEY_W", GLFW_KEY_W}, {"KEY_X", GLFW_KEY_X}, {"KEY_Y", GLFW_KEY_Y},
	    {"KEY_Z", GLFW_KEY_Z}, {"KEY_LEFT_BRACKET", GLFW_KEY_LEFT_BRACKET}, {"KEY_BACKSLASH", GLFW_KEY_BACKSLASH}, {"KEY_RIGHT_BRACKET", GLFW_KEY_RIGHT_BRACKET}, {"KEY_GRAVE_ACCENT", GLFW_KEY_GRAVE_ACCENT}, {"KEY_WORLD_1", GLFW_KEY_WORLD_1}, {"KEY_WORLD_2", GLFW_KEY_WORLD_2},
	    {"KEY_ESCAPE", GLFW_KEY_ESCAPE}, {"KEY_ENTER", GLFW_KEY_ENTER}, {"KEY_TAB", GLFW_KEY_TAB}, {"KEY_BACKSPACE", GLFW_KEY_BACKSPACE}, {"KEY_INSERT", GLFW_KEY_INSERT}, {"KEY_DELETE", GLFW_KEY_DELETE}, {"KEY_RIGHT", GLFW_KEY_RIGHT}, {"KEY_LEFT", GLFW_KEY_LEFT},
	    {"KEY_DOWN", GLFW_KEY_DOWN}, {"KEY_UP", GLFW_KEY_UP}, {"KEY_PAGE_UP", GLFW_KEY_PAGE_UP}, {"KEY_PAGE_DOWN", GLFW_KEY_PAGE_DOWN}, {"KEY_HOME", GLFW_KEY_HOME}, {"KEY_END", GLFW_KEY_END}, {"KEY_CAPS_LOCK", GLFW_KEY_CAPS_LOCK}, {"KEY_SCROLL_LOCK", GLFW_KEY_SCROLL_LOCK},
	    {"KEY_NUM_LOCK", GLFW_KEY_NUM_LOCK}, {"KEY_PRINT_SCREEN", GLFW_KEY_PRINT_SCREEN}, {"KEY_PAUSE", GLFW_KEY_PAUSE}, {"KEY_F1", GLFW_KEY_F1}, {"KEY_F2", GLFW_KEY_F2}, {"KEY_F3", GLFW_KEY_F3}, {"KEY_F4", GLFW_KEY_F4}, {"KEY_F5", GLFW_KEY_F5}, {"KEY_F6", GLFW_KEY_F6},
	    {"KEY_F7", GLFW_KEY_F7}, {"KEY_F8", GLFW_KEY_F8}, {"KEY_F9", GLFW_KEY_F9}, {"KEY_F10", GLFW_KEY_F10}, {"KEY_F11", GLFW_KEY_F11}, {"KEY_F12", GLFW_KEY_F12}, {"KEY_F13", GLFW_KEY_F13}, {"KEY_F14", GLFW_KEY_F14}, {"KEY_F15", GLFW_KEY_F15}, {"KEY_F16", GLFW_KEY_F16},
	    {"KEY_F17", GLFW_KEY_F17}, {"KEY_F18", GLFW_KEY_F18}, {"KEY_F19", GLFW_KEY_F19}, {"KEY_F20", GLFW_KEY_F20}, {"KEY_F21", GLFW_KEY_F21}, {"KEY_F22", GLFW_KEY_F22}, {"KEY_F23", GLFW_KEY_F23}, {"KEY_F24", GLFW_KEY_F24}, {"KEY_F25", GLFW_KEY_F25}, {"KEY_KP_0", GLFW_KEY_KP_0},
	    {"KEY_KP_1", GLFW_KEY_KP_1}, {"KEY_KP_2", GLFW_KEY_KP_2}, {"KEY_KP_3", GLFW_KEY_KP_3}, {"KEY_KP_4", GLFW_KEY_KP_4}, {"KEY_KP_5", GLFW_KEY_KP_5}, {"KEY_KP_6", GLFW_KEY_KP_6}, {"KEY_KP_7", GLFW_KEY_KP_7}, {"KEY_KP_8", GLFW_KEY_KP_8}, {"KEY_KP_9", GLFW_KEY_KP_9},
	    {"KEY_KP_DECIMAL", GLFW_KEY_KP_DECIMAL}, {"KEY_KP_DIVIDE", GLFW_KEY_KP_DIVIDE}, {"KEY_KP_MULTIPLY", GLFW_KEY_KP_MULTIPLY}, {"KEY_KP_SUBTRACT", GLFW_KEY_KP_SUBTRACT}, {"KEY_KP_ADD", GLFW_KEY_KP_ADD}, {"KEY_KP_ENTER", GLFW_KEY_KP_ENTER}, {"KEY_KP_EQUAL", GLFW_KEY_KP_EQUAL},
	    {"KEY_LEFT_SHIFT", GLFW_KEY_LEFT_SHIFT}, {"KEY_LEFT_CONTROL", GLFW_KEY_LEFT_CONTROL}, {"KEY_LEFT_ALT", GLFW_KEY_LEFT_ALT}, {"KEY_LEFT_SUPER", GLFW_KEY_LEFT_SUPER}, {"KEY_RIGHT_SHIFT", GLFW_KEY_RIGHT_SHIFT}, {"KEY_RIGHT_CONTROL", GLFW_KEY_RIGHT_CONTROL},
	    {"KEY_RIGHT_ALT", GLFW_KEY_RIGHT_ALT}, {"KEY_RIGHT_SUPER", GLFW_KEY_RIGHT_SUPER}, {"KEY_MENU", GLFW_KEY_MENU},

	    {"KEY_SCRL_UP", GLFW_CUSTOM_KEY_SCRL_UP}, {"KEY_SCRL_DOWN", GLFW_CUSTOM_KEY_SCRL_DOWN}, {"KEY_SCRL_DOWN", GLFW_CUSTOM_KEY_SCRL_DOWN},

	    {"KEY_MOUSE_BUTTON_1", GLFW_KEY_SPECIAL_MOUSE_BUTTON_1}, {"KEY_MOUSE_BUTTON_2", GLFW_KEY_SPECIAL_MOUSE_BUTTON_2}, {"KEY_MOUSE_BUTTON_3", GLFW_KEY_SPECIAL_MOUSE_BUTTON_3}, {"KEY_MOUSE_BUTTON_4", GLFW_KEY_SPECIAL_MOUSE_BUTTON_4},
	    {"KEY_MOUSE_BUTTON_5", GLFW_KEY_SPECIAL_MOUSE_BUTTON_5}, {"KEY_MOUSE_BUTTON_6", GLFW_KEY_SPECIAL_MOUSE_BUTTON_6}, {"KEY_MOUSE_BUTTON_7", GLFW_KEY_SPECIAL_MOUSE_BUTTON_7}, {"KEY_MOUSE_BUTTON_8", GLFW_KEY_SPECIAL_MOUSE_BUTTON_8},
	    {"KEY_MOUSE_BUTTON_9", GLFW_KEY_SPECIAL_MOUSE_BUTTON_9},

	    {"CONTROLLER_0_KEY_START", GLFW_CUSTOM_KEY_JOYSTICK_0_KEY_START}, {"CONTROLLER_0_AXIS_START", GLFW_CUSTOM_KEY_JOYSTICK_0_AXIS_START}, {"CONTROLLER_1_KEY_START", GLFW_CUSTOM_KEY_JOYSTICK_1_KEY_START}, {"CONTROLLER_1_AXIS_START", GLFW_CUSTOM_KEY_JOYSTICK_1_AXIS_START},
	    {"CONTROLLER_2_KEY_START", GLFW_CUSTOM_KEY_JOYSTICK_2_KEY_START}, {"CONTROLLER_2_AXIS_START", GLFW_CUSTOM_KEY_JOYSTICK_2_AXIS_START}, {"CONTROLLER_3_KEY_START", GLFW_CUSTOM_KEY_JOYSTICK_3_KEY_START}, {"CONTROLLER_3_AXIS_START", GLFW_CUSTOM_KEY_JOYSTICK_3_AXIS_START},
	    {"CONTROLLER_4_KEY_START", GLFW_CUSTOM_KEY_JOYSTICK_4_KEY_START}, {"CONTROLLER_4_AXIS_START", GLFW_CUSTOM_KEY_JOYSTICK_4_AXIS_START}, {"CONTROLLER_5_KEY_START", GLFW_CUSTOM_KEY_JOYSTICK_5_KEY_START}, {"CONTROLLER_5_AXIS_START", GLFW_CUSTOM_KEY_JOYSTICK_5_AXIS_START},
	    {"CONTROLLER_6_KEY_START", GLFW_CUSTOM_KEY_JOYSTICK_6_KEY_START}, {"CONTROLLER_6_AXIS_START", GLFW_CUSTOM_KEY_JOYSTICK_6_AXIS_START}, {"CONTROLLER_7_KEY_START", GLFW_CUSTOM_KEY_JOYSTICK_7_KEY_START}, {"CONTROLLER_7_AXIS_START", GLFW_CUSTOM_KEY_JOYSTICK_7_AXIS_START},
	    {"CONTROLLER_8_KEY_START", GLFW_CUSTOM_KEY_JOYSTICK_8_KEY_START}, {"CONTROLLER_8_AXIS_START", GLFW_CUSTOM_KEY_JOYSTICK_8_AXIS_START}, {"CONTROLLER_9_KEY_START", GLFW_CUSTOM_KEY_JOYSTICK_9_KEY_START}, {"CONTROLLER_9_AXIS_START", GLFW_CUSTOM_KEY_JOYSTICK_9_AXIS_START}});

	/*lua_registerglobalint(SHADER_VERTEX_BUFFER_LOCATION);
	lua_registerglobalint(SHADER_UV_BUFFER_LOCATION);
	lua_registerglobalint(SHADER_NORMAL_BUFFER_LOCATION);*/ // Vulkan TODO
}

void CGame::InitializeLua()
{
	Game::InitializeLua();
	CallCallbacks<void, lua_State *>("OnLuaInitialized", GetLuaState());
}

void CGame::SetupLua()
{
	Game::SetupLua();
	RunLuaFiles("autorun/");
	RunLuaFiles("autorun/client/");
	InitializeGameMode();
	//for(auto &subDir : GetLuaEntityDirectories())
	//	LoadLuaEntities(subDir);

	LoadLuaShaders();
}

void CGame::LoadLuaShaders()
{
	std::vector<std::string> files;
	//FileManager::FindFiles(Lua::SCRIPT_DIRECTORY_SLASH +"shaders\\*.lua",&files,NULL); // Deprecated; Shaders have to be explicitely included now
	for(unsigned int i = 0; i < files.size(); i++)
		LoadLuaShader(files[i]);
}

void CGame::LoadLuaShader(std::string file)
{
	StringToLower(file);
	std::string identifier = file.substr(0, file.length() - 4);
	file = "shaders/" + file;
	ExecuteLuaFile(file);
}

std::string CGame::GetLuaNetworkDirectoryName() const { return "client"; }
std::string CGame::GetLuaNetworkFileName() const { return "cl_init" + Lua::DOT_FILE_EXTENSION; }

//////////////////////////////////////////////

void ClientState::RegisterSharedLuaGlobals(Lua::Interface &lua) {}
