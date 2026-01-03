// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :scripting.lua.libraries.net_messages;
import pragma.shared;
import pragma.string.unicode;

#undef LoadLibrary
#undef LEFT
#undef RIGHT
#undef TOP
#undef BOTTOM
#undef NEAR
#undef FAR

void Lua::register_shared_client_state(lua::State *l)
{
	RegisterLibrary(l, "locale", {{"get_text", Locale::get_text}, {"get_languages", Locale::get_languages}});
	auto modLocale = luabind::module_(l, "locale");
	modLocale[(luabind::def("load", Locale::load), luabind::def("get_language", Locale::get_language), luabind::def("change_language", Locale::change_language), luabind::def("set_text", Locale::set_localization), luabind::def("localize", Locale::localize),
	  luabind::def("relocalize", Locale::relocalize))];
	modLocale[luabind::def("get_used_characters", +[]() -> std::string { return pragma::locale::get_used_characters().cpp_str(); })];
	modLocale[luabind::def("load_all", +[]() { pragma::locale::load_all(); })];
	modLocale[luabind::def("clear", Locale::clear)];
	modLocale[luabind::def("get_texts", Locale::get_texts)];
	modLocale[luabind::def(
	  "get_raw_text", +[](const std::string &id) -> std::optional<std::string> {
		  std::string text;
		  if(pragma::locale::get_raw_text(id, text))
			  return text;
		  return {};
	  })];
	modLocale[luabind::def("parse", static_cast<opt<map<std::string, std::string>> (*)(lua::State *, const std::string &, const std::string &)>(Locale::parse))];
	modLocale[luabind::def("parse", static_cast<opt<map<std::string, std::string>> (*)(lua::State *, const std::string &)>(Locale::parse))];
}

void pragma::CGame::RegisterLua()
{
	GetLuaInterface().SetIdentifier("cl");

	Lua::engine::register_library(GetLuaState());
	auto modEngine = luabind::module_(GetLuaState(), "engine");
	modEngine[(luabind::def("get_text_size", static_cast<Vector2i (*)(lua::State *, const std::string &, const std::string &)>(Lua::engine::get_text_size)),
	  luabind::def("get_text_size", static_cast<Vector2i (*)(lua::State *, const std::string &, const gui::FontInfo &)>(Lua::engine::get_text_size)),

	  luabind::def("get_truncated_text_length", static_cast<std::pair<size_t, size_t> (*)(lua::State *, const std::string &, const std::string &, uint32_t)>(Lua::engine::get_truncated_text_length)),
	  luabind::def("get_truncated_text_length", static_cast<std::pair<size_t, size_t> (*)(lua::State *, const std::string &, const gui::FontInfo &, uint32_t)>(Lua::engine::get_truncated_text_length)),

	  luabind::def("poll_console_output", Lua::engine::poll_console_output), luabind::def("library_exists", Lua::engine::LibraryExists), luabind::def("load_library", Lua::engine::LoadLibrary), luabind::def("unload_library", Lua::engine::UnloadLibrary),
	  luabind::def("is_library_loaded", Lua::engine::IsLibraryLoaded), luabind::def("get_info", Lua::engine::get_info), luabind::def("get_user_data_dir", util::get_user_data_dir), luabind::def("get_resource_dirs", util::get_resource_dirs),
	  luabind::def(
	    "open_user_data_dir_in_explorer", +[]() {
		    auto &dir = fs::get_absolute_primary_root_path();
		    util::open_path_in_explorer(dir.GetString());
	    }))];

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
	    {"set_default_game_render_enabled", static_cast<int32_t (*)(lua::State *)>([](lua::State *l) -> int32_t {
		     get_cgame()->SetDefaultGameRenderEnabled(Lua::CheckBool(l, 1));
		     return 0;
	     })},
	    {"is_default_game_render_enabled", static_cast<int32_t (*)(lua::State *)>([](lua::State *l) -> int32_t {
		     Lua::PushBool(l, get_cgame()->IsDefaultGameRenderEnabled());
		     return 1;
	     })},

	    {"get_render_mask", static_cast<int32_t (*)(lua::State *)>([](lua::State *l) -> int32_t {
		     auto mask = get_cgame()->GetRenderMask(Lua::CheckString(l, 1));
		     Lua::Push(l, mask);
		     return 1;
	     })},
	    {"find_render_mask_name", static_cast<int32_t (*)(lua::State *)>([](lua::State *l) -> int32_t {
		     auto *name = get_cgame()->FindRenderMaskName(Lua::Check<rendering::RenderMask>(l, 1));
		     if(!name)
			     return 0;
		     Lua::PushString(l, *name);
		     return 1;
	     })},
	    {"register_render_mask", static_cast<int32_t (*)(lua::State *)>([](lua::State *l) -> int32_t {
		     auto inclusiveByDefault = true;
		     if(Lua::IsSet(l, 2))
			     inclusiveByDefault = Lua::CheckBool(l, 2);
		     auto mask = get_cgame()->RegisterRenderMask(Lua::CheckString(l, 1), inclusiveByDefault);
		     Lua::Push(l, mask);
		     return 1;
	     })},
	    {"register_particle_system",
	      +[](lua::State *l) -> int32_t {
		      std::string ptName = Lua::CheckString(l, 1);
		      auto &udmData = Lua::Check<udm::LinkedPropertyWrapper>(l, 2);
		      std::string err;
		      auto res = ecs::CParticleSystemComponent::InitializeFromAssetData(ptName, udmData, err);
		      Lua::PushBool(l, res);
		      return 1;
	      }},
	    {"wait_for_frames",
	      +[](lua::State *l) -> int32_t {
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
						      Lua::CallFunction(l, [&func](lua::State *l) -> Lua::StatusCode {
							      func.push(l);
							      return Lua::StatusCode::Ok;
						      });
						      if(cbThink.IsValid())
							      cbThink.Remove();
					      });
					      get_cgame()->AddCallback("Think", cbThink);
				      }
				      else {
					      Lua::CallFunction(l, [&func](lua::State *l) -> Lua::StatusCode {
						      func.push(l);
						      return Lua::StatusCode::Ok;
					      });
				      }
				      if(cb.IsValid())
					      cb.Remove();
			      }
		      });
		      cb = get_cgame()->AddCallback("PostRenderScenes", cb);
		      Lua::Push<CallbackHandle>(l, cb);
		      return 1;
	      }}

	    /*{"debug_vehicle",static_cast<int32_t(*)(lua::State*)>([](lua::State *l) -> int32_t {
			Con::COUT<<"Creating vehicle..."<<Con::endl;
			static auto vhc = pragma::util::make_shared<debug::Vehicle>();
			return 0;
		})}*/
	  });
	auto modGame = luabind::module_(GetLuaState(), "game");
	Lua::game::register_shared_functions(GetLuaState(), modGame);
	modGame[(luabind::def("load_material", static_cast<material::Material *(*)(lua::State *, const std::string &, bool, bool)>(Lua::engine::load_material)),
	  luabind::def("load_material", static_cast<material::Material *(*)(lua::State *, const std::string &, bool)>(Lua::engine::load_material)), luabind::def("load_material", static_cast<material::Material *(*)(lua::State *, const std::string &)>(Lua::engine::load_material)),
	  luabind::def("load_texture", static_cast<std::shared_ptr<prosper::Texture> (*)(lua::State *, const std::string &, util::AssetLoadFlags)>(Lua::engine::load_texture)),
	  luabind::def("load_texture", static_cast<std::shared_ptr<prosper::Texture> (*)(lua::State *, const std::string &)>(Lua::engine::load_texture)),
	  luabind::def("load_texture", static_cast<std::shared_ptr<prosper::Texture> (*)(lua::State *, const LFile &, const std::string &, util::AssetLoadFlags loadFlags)>(Lua::engine::load_texture)),
	  luabind::def("load_texture", static_cast<std::shared_ptr<prosper::Texture> (*)(lua::State *, const LFile &, const std::string &)>(Lua::engine::load_texture)),
	  luabind::def("load_texture", static_cast<std::shared_ptr<prosper::Texture> (*)(lua::State *, const LFile &, util::AssetLoadFlags)>(Lua::engine::load_texture)),
	  luabind::def("load_texture", static_cast<std::shared_ptr<prosper::Texture> (*)(lua::State *, const LFile &)>(Lua::engine::load_texture)), luabind::def("precache_particle_system", static_cast<bool (*)(lua::State *, const std::string &, bool)>(Lua::engine::precache_particle_system)),
	  luabind::def("precache_particle_system", static_cast<bool (*)(lua::State *, const std::string &)>(Lua::engine::precache_particle_system)), luabind::def("load_sound_scripts", static_cast<void (*)(lua::State *, const std::string &, bool)>(Lua::engine::LoadSoundScripts)),
	  luabind::def("load_sound_scripts", static_cast<void (*)(lua::State *, const std::string &)>(Lua::engine::LoadSoundScripts)), luabind::def("get_model", Lua::engine::get_model), luabind::def("get_number_of_scenes_queued_for_rendering", &CGame::GetNumberOfScenesQueuedForRendering),
	  luabind::def("get_queued_scene_render_info", &CGame::GetQueuedSceneRenderInfo),

	  luabind::def("set_gameplay_control_camera", &CGame::SetGameplayControlCamera<CCameraComponent>), luabind::def("reset_gameplay_control_camera", &CGame::ResetGameplayControlCamera), luabind::def("get_gameplay_control_camera", &CGame::GetGameplayControlCamera<CCameraComponent>),
	  luabind::def("clear_gameplay_control_camera", &CGame::ClearGameplayControlCamera),
	  luabind::def(
	    "get_primary_camera_render_mask", +[]() -> std::pair<rendering::RenderMask, rendering::RenderMask> {
		    auto inclusionMask = rendering::RenderMask::None;
		    auto exclusionMask = rendering::RenderMask::None;
		    get_cgame()->GetPrimaryCameraRenderMask(inclusionMask, exclusionMask);
		    return {inclusionMask, exclusionMask};
	    }))];

	Lua::ents::register_library(GetLuaState());
	auto &modEnts = GetLuaInterface().RegisterLibrary("ents",
	  {{"get_local_player", Lua::ents::Client::get_local_player}, {"get_listener", Lua::ents::Client::get_listener}, {"get_view_body", Lua::ents::Client::get_view_body}, {"get_view_model", Lua::ents::Client::get_view_model}, {"get_instance_buffer", Lua::ents::Client::get_instance_buffer},
	    {"get_instance_bone_buffer", Lua::ents::Client::get_instance_bone_buffer}, {"register_component", Lua::ents::register_component<CLuaBaseEntityComponent>}, {"create_camera", static_cast<int32_t (*)(lua::State *)>([](lua::State *l) -> int32_t {
		                                                                                                                                                                auto aspectRatio = Lua::CheckNumber(l, 1);
		                                                                                                                                                                auto fov = Lua::CheckNumber(l, 2);
		                                                                                                                                                                auto nearZ = Lua::CheckNumber(l, 3);
		                                                                                                                                                                auto farZ = Lua::CheckNumber(l, 4);
		                                                                                                                                                                auto *cam = get_cgame()->CreateCamera<CCameraComponent>(aspectRatio, fov, nearZ, farZ);
		                                                                                                                                                                if(cam == nullptr)
			                                                                                                                                                                return 0;
		                                                                                                                                                                cam->PushLuaObject(l);
		                                                                                                                                                                return 1;
	                                                                                                                                                                })},
	    {"create_scene", Lua::game::Client::create_scene}});
	modEnts[luabind::def("register_component_event", &Lua::ents::register_component_event)];

	auto entityClassDef = luabind::class_<ecs::BaseEntity>("BaseEntityBase");
	Lua::Entity::register_class(entityClassDef);
	modEnts[entityClassDef];

	auto cEntityClassDef = luabind::class_<ecs::CBaseEntity, ecs::BaseEntity>("Entity");
	Lua::Entity::Client::register_class(cEntityClassDef);
	modEnts[cEntityClassDef];

	//auto tmp = luabind::class_<EntityHandle>("EntityOld");
	//modEnts[tmp];

	auto classDefBase = luabind::class_<CLuaEntity, luabind::bases<ecs::CBaseEntity>, LuaCore::HandleHolder<CLuaEntity>>("BaseEntity");
	classDefBase.def(luabind::constructor<>());
	// classDefBase.def(luabind::tostring(luabind::self));
	//classDefBase.def(luabind::constructor<>());
	classDefBase.def("Initialize", &CLuaEntity::LuaInitialize, &CLuaEntity::default_Initialize);
	//classDefBase.def("ReceiveNetEvent",&SLuaEntityWrapper::ReceiveNetEvent,&SLuaBaseEntityWrapper::default_ReceiveNetEvent);
	modEnts[classDefBase];

	auto modNet = luabind::module(GetLuaState(), "net");
	modNet[(luabind::def("send", &Lua::net::client::send), luabind::def("receive", &Lua::net::client::receive), luabind::def("register_event", &Lua::net::register_event))];

	auto netPacketClassDef = luabind::class_<NetPacket>("Packet");
	Lua::NetPacket::Client::register_class(netPacketClassDef);
	netPacketClassDef.def("WritePlayer", static_cast<void (*)(lua::State *, NetPacket &, util::WeakHandle<CPlayerComponent> &)>([](lua::State *l, NetPacket &packet, util::WeakHandle<CPlayerComponent> &pl) { networking::write_player(packet, pl.get()); }));
	netPacketClassDef.def("WritePlayer", static_cast<void (*)(lua::State *, NetPacket &, EntityHandle &)>([](lua::State *l, NetPacket &packet, EntityHandle &hEnt) { networking::write_player(packet, hEnt.get()); }));
	netPacketClassDef.def("ReadPlayer", static_cast<void (*)(lua::State *, NetPacket &)>([](lua::State *l, NetPacket &packet) {
		auto *pl = static_cast<CPlayerComponent *>(networking::read_player(packet));
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

	lua::push_boolean(GetLuaState(), 1);
	Lua::SetGlobal(GetLuaState(), "CLIENT");

	lua::push_boolean(GetLuaState(), 0);
	Lua::SetGlobal(GetLuaState(), "SERVER");

	auto modTime = luabind::module_(GetLuaState(), "time");
	modTime[(luabind::def("server_time", Lua::ServerTime), luabind::def("frame_time", Lua::FrameTime))];

	Lua::RegisterLibraryEnums(GetLuaState(), "sound",
	  {{"CHANNEL_CONFIG_MONO", math::to_integral(audio::ChannelConfig::Mono)}, {"CHANNEL_CONFIG_STEREO", math::to_integral(audio::ChannelConfig::Stereo)}, {"CHANNEL_CONFIG_REAR", math::to_integral(audio::ChannelConfig::Rear)},
	    {"CHANNEL_CONFIG_QUAD", math::to_integral(audio::ChannelConfig::Quad)}, {"CHANNEL_CONFIG_X51", math::to_integral(audio::ChannelConfig::X51)}, {"CHANNEL_CONFIG_X61", math::to_integral(audio::ChannelConfig::X61)}, {"CHANNEL_CONFIG_X71", math::to_integral(audio::ChannelConfig::X71)},
	    {"CHANNEL_CONFIG_BFORMAT_2D", math::to_integral(audio::ChannelConfig::BFormat2D)}, {"CHANNEL_CONFIG_BFORMAT_3D", math::to_integral(audio::ChannelConfig::BFormat3D)},

	    {"SAMPLE_TYPE_UINT8", math::to_integral(audio::SampleType::UInt8)}, {"SAMPLE_TYPE_INT16", math::to_integral(audio::SampleType::Int16)}, {"SAMPLE_TYPE_FLOAT32", math::to_integral(audio::SampleType::Float32)}, {"SAMPLE_TYPE_MULAW", math::to_integral(audio::SampleType::Mulaw)}});

	Lua::RegisterLibraryEnums(GetLuaState(), "time", {{"TIMER_TYPE_SERVERTIME", math::to_integral(TimerType::ServerTime)}});

	Lua::RegisterLibraryEnums(GetLuaState(), "file", {{"SEARCH_RESOURCES", networking::FSYS_SEARCH_RESOURCES}});

	Lua::RegisterLibraryEnums(GetLuaState(), "geometry",
	  {{"FrustumPlane_LEFT", pragma::math::to_integral(math::FrustumPlane::Left)}, {"FrustumPlane_RIGHT", pragma::math::to_integral(math::FrustumPlane::Right)}, {"FrustumPlane_TOP", pragma::math::to_integral(math::FrustumPlane::Top)},
	    {"FrustumPlane_BOTTOM", pragma::math::to_integral(math::FrustumPlane::Bottom)}, {"FrustumPlane_NEAR", pragma::math::to_integral(math::FrustumPlane::Near)}, {"FrustumPlane_FAR", pragma::math::to_integral(math::FrustumPlane::Far)},
	    {"FrustumPlane_COUNT", pragma::math::to_integral(math::FrustumPlane::Count)},

	    {"FrustumPoint_FarBottomLeft", pragma::math::to_integral(math::FrustumPoint::FarBottomLeft)}, {"FrustumPoint_FAR_TOP_LEFT", pragma::math::to_integral(math::FrustumPoint::FarTopLeft)}, {"FrustumPoint_FAR_TOP_RIGHT", pragma::math::to_integral(math::FrustumPoint::FarTopRight)},
	    {"FrustumPoint_FAR_BOTTOM_RIGHT", pragma::math::to_integral(math::FrustumPoint::FarBottomRight)}, {"FrustumPoint_NearBottomLeft", pragma::math::to_integral(math::FrustumPoint::NearBottomLeft)},
	    {"FrustumPoint_NearTopLeft", pragma::math::to_integral(math::FrustumPoint::NearTopLeft)}, {"FrustumPoint_NearTopRight", pragma::math::to_integral(math::FrustumPoint::NearTopRight)}, {"FrustumPoint_NearBottomRight", pragma::math::to_integral(math::FrustumPoint::NearBottomRight)}});

	Lua::RegisterLibraryEnums(GetLuaState(), "gui",
	  {{"CURSOR_SHAPE_DEFAULT", math::to_integral(platform::Cursor::Shape::Default)}, {"CURSOR_SHAPE_HIDDEN", math::to_integral(platform::Cursor::Shape::Hidden)}, {"CURSOR_SHAPE_ARROW", math::to_integral(platform::Cursor::Shape::Arrow)},
	    {"CURSOR_SHAPE_IBEAM", math::to_integral(platform::Cursor::Shape::IBeam)}, {"CURSOR_SHAPE_CROSSHAIR", math::to_integral(platform::Cursor::Shape::Crosshair)}, {"CURSOR_SHAPE_HAND", math::to_integral(platform::Cursor::Shape::Hand)},
	    {"CURSOR_SHAPE_HRESIZE", math::to_integral(platform::Cursor::Shape::HResize)}, {"CURSOR_SHAPE_VRESIZE", math::to_integral(platform::Cursor::Shape::VResize)},

	    {"CURSOR_MODE_DISABLED", math::to_integral(platform::CursorMode::Disabled)}, {"CURSOR_MODE_HIDDEN", math::to_integral(platform::CursorMode::Hidden)}, {"CURSOR_MODE_NORMAL", math::to_integral(platform::CursorMode::Normal)}});

	Lua::RegisterLibraryEnums(GetLuaState(), "game",
	  {
	    {"RENDER_FLAG_NONE", 0},
	    {"RENDER_FLAG_BIT_WORLD", math::to_integral(rendering::RenderFlags::World)},
	    {"RENDER_FLAG_BIT_VIEW", math::to_integral(rendering::RenderFlags::View)},
	    {"RENDER_FLAG_BIT_SKYBOX", math::to_integral(rendering::RenderFlags::Skybox)},
	    {"RENDER_FLAG_BIT_SHADOWS", math::to_integral(rendering::RenderFlags::Shadows)},
	    {"RENDER_FLAG_BIT_PARTICLES", math::to_integral(rendering::RenderFlags::Particles)},
	    {"RENDER_FLAG_BIT_DEBUG", math::to_integral(rendering::RenderFlags::Debug)},
	    {"RENDER_FLAG_ALL", math::to_integral(rendering::RenderFlags::All)},
	    {"RENDER_FLAG_REFLECTION_BIT", math::to_integral(rendering::RenderFlags::Reflection)},
	    {"RENDER_FLAG_WATER_BIT", math::to_integral(rendering::RenderFlags::Water)},
	    {"RENDER_FLAG_STATIC_BIT", math::to_integral(rendering::RenderFlags::Static)},
	    {"RENDER_FLAG_DYNAMIC_BIT", math::to_integral(rendering::RenderFlags::Dynamic)},
	    {"RENDER_FLAG_TRANSLUCENT_BIT", math::to_integral(rendering::RenderFlags::Translucent)},
	    {"RENDER_FLAG_HDR_BIT", math::to_integral(rendering::RenderFlags::HDR)},
	    {"RENDER_FLAG_PARTICLE_DEPTH_BIT", math::to_integral(rendering::RenderFlags::ParticleDepth)},

	    {"ASSET_LOAD_FLAG_NONE", math::to_integral(util::AssetLoadFlags::None)},
	    {"ASSET_LOAD_FLAG_DONT_CACHE", math::to_integral(util::AssetLoadFlags::DontCache)},
	    {"ASSET_LOAD_FLAG_IGNORE_CACHE", math::to_integral(util::AssetLoadFlags::IgnoreCache)},
	  });

	auto gameMod = luabind::module(GetLuaState(), "game");
	RegisterLuaGameClasses(gameMod);

	// Needs to be registered AFTER RegisterLuaGameClasses has been called!
	Lua::register_base_entity_component(modEnts);
	auto defEntCmp = pragma::LuaCore::create_entity_component_class<CLuaBaseEntityComponent, luabind::bases<BaseLuaBaseEntityComponent, BaseEntityComponent>, LuaCore::CLuaBaseEntityComponentHolder>("BaseEntityComponent");
	defEntCmp.def(luabind::constructor<ecs::CBaseEntity &>());
	defEntCmp.def("ReceiveData", static_cast<void (*)(lua::State *, CLuaBaseEntityComponent &, NetPacket)>([](lua::State *l, CLuaBaseEntityComponent &hComponent, NetPacket packet) {

	}));
	defEntCmp.def("ReceiveNetEvent", static_cast<void (*)(lua::State *, CLuaBaseEntityComponent &, uint32_t, NetPacket)>([](lua::State *l, CLuaBaseEntityComponent &hComponent, uint32_t evId, NetPacket packet) {

	}));
	defEntCmp.def("ReceiveSnapshotData", static_cast<void (*)(lua::State *, CLuaBaseEntityComponent &, NetPacket)>([](lua::State *l, CLuaBaseEntityComponent &hComponent, NetPacket packet) {

	}));
	register_shared_lua_component_methods<CLuaBaseEntityComponent>(defEntCmp);
	modEnts[defEntCmp];

	auto _G = luabind::globals(GetLuaState());
	_G["BaseEntityComponent"] = _G["ents"]["BaseEntityComponent"];

	Lua::RegisterLibraryEnums(GetLuaState(), "input",
	  {
	    {"STATE_PRESS", math::to_integral(platform::KeyState::Press)},
	    {"STATE_RELEASE", math::to_integral(platform::KeyState::Release)},
	    {"STATE_REPEAT", math::to_integral(platform::KeyState::Repeat)},

	    {"MOUSE_BUTTON_1", math::to_integral(platform::MouseButton::N1)},
	    {"MOUSE_BUTTON_2", math::to_integral(platform::MouseButton::N2)},
	    {"MOUSE_BUTTON_3", math::to_integral(platform::MouseButton::N3)},
	    {"MOUSE_BUTTON_4", math::to_integral(platform::MouseButton::N4)},
	    {"MOUSE_BUTTON_5", math::to_integral(platform::MouseButton::N5)},
	    {"MOUSE_BUTTON_6", math::to_integral(platform::MouseButton::N6)},
	    {"MOUSE_BUTTON_7", math::to_integral(platform::MouseButton::N7)},
	    {"MOUSE_BUTTON_8", math::to_integral(platform::MouseButton::N8)},
	    {"MOUSE_BUTTON_LEFT", math::to_integral(platform::MouseButton::Left)},
	    {"MOUSE_BUTTON_RIGHT", math::to_integral(platform::MouseButton::Right)},
	    {"MOUSE_BUTTON_MIDDLE", math::to_integral(platform::MouseButton::Middle)},

	    {"MOD_NONE", math::to_integral(platform::Modifier::None)},
	    {"MOD_SHIFT", math::to_integral(platform::Modifier::Shift)},
	    {"MOD_CONTROL", math::to_integral(platform::Modifier::Control)},
	    {"MOD_ALT", math::to_integral(platform::Modifier::Alt)},
	    {"MOD_SUPER", math::to_integral(platform::Modifier::Super)},

	    {"KEY_UNKNOWN", math::to_integral(platform::Key::Unknown)},
	    {"KEY_SPACE", math::to_integral(platform::Key::Space)},
	    {"KEY_APOSTROPHE", math::to_integral(platform::Key::Apostrophe)},
	    {"KEY_COMMA", math::to_integral(platform::Key::Comma)},
	    {"KEY_MINUS", math::to_integral(platform::Key::Minus)},
	    {"KEY_PERIOD", math::to_integral(platform::Key::Period)},
	    {"KEY_SLASH", math::to_integral(platform::Key::Slash)},
	    {"KEY_0", math::to_integral(platform::Key::N0)},
	    {"KEY_1", math::to_integral(platform::Key::N1)},
	    {"KEY_2", math::to_integral(platform::Key::N2)},
	    {"KEY_3", math::to_integral(platform::Key::N3)},
	    {"KEY_4", math::to_integral(platform::Key::N4)},
	    {"KEY_5", math::to_integral(platform::Key::N5)},
	    {"KEY_6", math::to_integral(platform::Key::N6)},
	    {"KEY_7", math::to_integral(platform::Key::N7)},
	    {"KEY_8", math::to_integral(platform::Key::N8)},
	    {"KEY_9", math::to_integral(platform::Key::N9)},
	    {"KEY_SEMICOLON", math::to_integral(platform::Key::Semicolon)},
	    {"KEY_EQUAL", math::to_integral(platform::Key::Equal)},
	    {"KEY_A", math::to_integral(platform::Key::A)},
	    {"KEY_B", math::to_integral(platform::Key::B)},
	    {"KEY_C", math::to_integral(platform::Key::C)},
	    {"KEY_D", math::to_integral(platform::Key::D)},
	    {"KEY_E", math::to_integral(platform::Key::E)},
	    {"KEY_F", math::to_integral(platform::Key::F)},
	    {"KEY_G", math::to_integral(platform::Key::G)},
	    {"KEY_H", math::to_integral(platform::Key::H)},
	    {"KEY_I", math::to_integral(platform::Key::I)},
	    {"KEY_J", math::to_integral(platform::Key::J)},
	    {"KEY_K", math::to_integral(platform::Key::K)},
	    {"KEY_L", math::to_integral(platform::Key::L)},
	    {"KEY_M", math::to_integral(platform::Key::M)},
	    {"KEY_N", math::to_integral(platform::Key::N)},
	    {"KEY_O", math::to_integral(platform::Key::O)},
	    {"KEY_P", math::to_integral(platform::Key::P)},
	    {"KEY_Q", math::to_integral(platform::Key::Q)},
	    {"KEY_R", math::to_integral(platform::Key::R)},
	    {"KEY_S", math::to_integral(platform::Key::S)},
	    {"KEY_T", math::to_integral(platform::Key::T)},
	    {"KEY_U", math::to_integral(platform::Key::U)},
	    {"KEY_V", math::to_integral(platform::Key::V)},
	    {"KEY_W", math::to_integral(platform::Key::W)},
	    {"KEY_X", math::to_integral(platform::Key::X)},
	    {"KEY_Y", math::to_integral(platform::Key::Y)},
	    {"KEY_Z", math::to_integral(platform::Key::Z)},
	    {"KEY_LEFT_BRACKET", math::to_integral(platform::Key::LeftBracket)},
	    {"KEY_BACKSLASH", math::to_integral(platform::Key::Backslash)},
	    {"KEY_RIGHT_BRACKET", math::to_integral(platform::Key::RightBracket)},
	    {"KEY_GRAVE_ACCENT", math::to_integral(platform::Key::GraveAccent)},
	    {"KEY_WORLD_1", math::to_integral(platform::Key::World1)},
	    {"KEY_WORLD_2", math::to_integral(platform::Key::World2)},
	    {"KEY_ESCAPE", math::to_integral(platform::Key::Escape)},
	    {"KEY_ENTER", math::to_integral(platform::Key::Enter)},
	    {"KEY_TAB", math::to_integral(platform::Key::Tab)},
	    {"KEY_BACKSPACE", math::to_integral(platform::Key::Backspace)},
	    {"KEY_INSERT", math::to_integral(platform::Key::Insert)},
	    {"KEY_DELETE", math::to_integral(platform::Key::Delete)},
	    {"KEY_RIGHT", math::to_integral(platform::Key::Right)},
	    {"KEY_LEFT", math::to_integral(platform::Key::Left)},
	    {"KEY_DOWN", math::to_integral(platform::Key::Down)},
	    {"KEY_UP", math::to_integral(platform::Key::Up)},
	    {"KEY_PAGE_UP", math::to_integral(platform::Key::PageUp)},
	    {"KEY_PAGE_DOWN", math::to_integral(platform::Key::PageDown)},
	    {"KEY_HOME", math::to_integral(platform::Key::Home)},
	    {"KEY_END", math::to_integral(platform::Key::End)},
	    {"KEY_CAPS_LOCK", math::to_integral(platform::Key::CapsLock)},
	    {"KEY_SCROLL_LOCK", math::to_integral(platform::Key::ScrollLock)},
	    {"KEY_NUM_LOCK", math::to_integral(platform::Key::NumLock)},
	    {"KEY_PRINT_SCREEN", math::to_integral(platform::Key::PrintScreen)},
	    {"KEY_PAUSE", math::to_integral(platform::Key::Pause)},
	    {"KEY_F1", math::to_integral(platform::Key::F1)},
	    {"KEY_F2", math::to_integral(platform::Key::F2)},
	    {"KEY_F3", math::to_integral(platform::Key::F3)},
	    {"KEY_F4", math::to_integral(platform::Key::F4)},
	    {"KEY_F5", math::to_integral(platform::Key::F5)},
	    {"KEY_F6", math::to_integral(platform::Key::F6)},
	    {"KEY_F7", math::to_integral(platform::Key::F7)},
	    {"KEY_F8", math::to_integral(platform::Key::F8)},
	    {"KEY_F9", math::to_integral(platform::Key::F9)},
	    {"KEY_F10", math::to_integral(platform::Key::F10)},
	    {"KEY_F11", math::to_integral(platform::Key::F11)},
	    {"KEY_F12", math::to_integral(platform::Key::F12)},
	    {"KEY_F13", math::to_integral(platform::Key::F13)},
	    {"KEY_F14", math::to_integral(platform::Key::F14)},
	    {"KEY_F15", math::to_integral(platform::Key::F15)},
	    {"KEY_F16", math::to_integral(platform::Key::F16)},
	    {"KEY_F17", math::to_integral(platform::Key::F17)},
	    {"KEY_F18", math::to_integral(platform::Key::F18)},
	    {"KEY_F19", math::to_integral(platform::Key::F19)},
	    {"KEY_F20", math::to_integral(platform::Key::F20)},
	    {"KEY_F21", math::to_integral(platform::Key::F21)},
	    {"KEY_F22", math::to_integral(platform::Key::F22)},
	    {"KEY_F23", math::to_integral(platform::Key::F23)},
	    {"KEY_F24", math::to_integral(platform::Key::F24)},
	    {"KEY_F25", math::to_integral(platform::Key::F25)},
	    {"KEY_KP_0", math::to_integral(platform::Key::Kp0)},
	    {"KEY_KP_1", math::to_integral(platform::Key::Kp1)},
	    {"KEY_KP_2", math::to_integral(platform::Key::Kp2)},
	    {"KEY_KP_3", math::to_integral(platform::Key::Kp3)},
	    {"KEY_KP_4", math::to_integral(platform::Key::Kp4)},
	    {"KEY_KP_5", math::to_integral(platform::Key::Kp5)},
	    {"KEY_KP_6", math::to_integral(platform::Key::Kp6)},
	    {"KEY_KP_7", math::to_integral(platform::Key::Kp7)},
	    {"KEY_KP_8", math::to_integral(platform::Key::Kp8)},
	    {"KEY_KP_9", math::to_integral(platform::Key::Kp9)},
	    {"KEY_KP_DECIMAL", math::to_integral(platform::Key::KpDecimal)},
	    {"KEY_KP_DIVIDE", math::to_integral(platform::Key::KpDivide)},
	    {"KEY_KP_MULTIPLY", math::to_integral(platform::Key::KpMultiply)},
	    {"KEY_KP_SUBTRACT", math::to_integral(platform::Key::KpSubtract)},
	    {"KEY_KP_ADD", math::to_integral(platform::Key::KpAdd)},
	    {"KEY_KP_ENTER", math::to_integral(platform::Key::KpEnter)},
	    {"KEY_KP_EQUAL", math::to_integral(platform::Key::KpEqual)},
	    {"KEY_LEFT_SHIFT", math::to_integral(platform::Key::LeftShift)},
	    {"KEY_LEFT_CONTROL", math::to_integral(platform::Key::LeftControl)},
	    {"KEY_LEFT_ALT", math::to_integral(platform::Key::LeftAlt)},
	    {"KEY_LEFT_SUPER", math::to_integral(platform::Key::LeftSuper)},
	    {"KEY_RIGHT_SHIFT", math::to_integral(platform::Key::RightShift)},
	    {"KEY_RIGHT_CONTROL", math::to_integral(platform::Key::RightControl)},
	    {"KEY_RIGHT_ALT", math::to_integral(platform::Key::RightAlt)},
	    {"KEY_RIGHT_SUPER", math::to_integral(platform::Key::RightSuper)},
	    {"KEY_MENU", math::to_integral(platform::Key::Menu)},

	    {"KEY_SCRL_UP", GLFW_CUSTOM_KEY_SCRL_UP},
	    {"KEY_SCRL_DOWN", GLFW_CUSTOM_KEY_SCRL_DOWN},
	    {"KEY_SCRL_DOWN", GLFW_CUSTOM_KEY_SCRL_DOWN},

	    {"KEY_MOUSE_BUTTON_1", GLFW_KEY_SPECIAL_MOUSE_BUTTON_1},
	    {"KEY_MOUSE_BUTTON_2", GLFW_KEY_SPECIAL_MOUSE_BUTTON_2},
	    {"KEY_MOUSE_BUTTON_3", GLFW_KEY_SPECIAL_MOUSE_BUTTON_3},
	    {"KEY_MOUSE_BUTTON_4", GLFW_KEY_SPECIAL_MOUSE_BUTTON_4},
	    {"KEY_MOUSE_BUTTON_5", GLFW_KEY_SPECIAL_MOUSE_BUTTON_5},
	    {"KEY_MOUSE_BUTTON_6", GLFW_KEY_SPECIAL_MOUSE_BUTTON_6},
	    {"KEY_MOUSE_BUTTON_7", GLFW_KEY_SPECIAL_MOUSE_BUTTON_7},
	    {"KEY_MOUSE_BUTTON_8", GLFW_KEY_SPECIAL_MOUSE_BUTTON_8},
	    {"KEY_MOUSE_BUTTON_9", GLFW_KEY_SPECIAL_MOUSE_BUTTON_9},

	    {"CONTROLLER_0_KEY_START", GLFW_CUSTOM_KEY_JOYSTICK_0_KEY_START},
	    {"CONTROLLER_0_AXIS_START", GLFW_CUSTOM_KEY_JOYSTICK_0_AXIS_START},
	    {"CONTROLLER_1_KEY_START", GLFW_CUSTOM_KEY_JOYSTICK_1_KEY_START},
	    {"CONTROLLER_1_AXIS_START", GLFW_CUSTOM_KEY_JOYSTICK_1_AXIS_START},
	    {"CONTROLLER_2_KEY_START", GLFW_CUSTOM_KEY_JOYSTICK_2_KEY_START},
	    {"CONTROLLER_2_AXIS_START", GLFW_CUSTOM_KEY_JOYSTICK_2_AXIS_START},
	    {"CONTROLLER_3_KEY_START", GLFW_CUSTOM_KEY_JOYSTICK_3_KEY_START},
	    {"CONTROLLER_3_AXIS_START", GLFW_CUSTOM_KEY_JOYSTICK_3_AXIS_START},
	    {"CONTROLLER_4_KEY_START", GLFW_CUSTOM_KEY_JOYSTICK_4_KEY_START},
	    {"CONTROLLER_4_AXIS_START", GLFW_CUSTOM_KEY_JOYSTICK_4_AXIS_START},
	    {"CONTROLLER_5_KEY_START", GLFW_CUSTOM_KEY_JOYSTICK_5_KEY_START},
	    {"CONTROLLER_5_AXIS_START", GLFW_CUSTOM_KEY_JOYSTICK_5_AXIS_START},
	    {"CONTROLLER_6_KEY_START", GLFW_CUSTOM_KEY_JOYSTICK_6_KEY_START},
	    {"CONTROLLER_6_AXIS_START", GLFW_CUSTOM_KEY_JOYSTICK_6_AXIS_START},
	    {"CONTROLLER_7_KEY_START", GLFW_CUSTOM_KEY_JOYSTICK_7_KEY_START},
	    {"CONTROLLER_7_AXIS_START", GLFW_CUSTOM_KEY_JOYSTICK_7_AXIS_START},
	    {"CONTROLLER_8_KEY_START", GLFW_CUSTOM_KEY_JOYSTICK_8_KEY_START},
	    {"CONTROLLER_8_AXIS_START", GLFW_CUSTOM_KEY_JOYSTICK_8_AXIS_START},
	    {"CONTROLLER_9_KEY_START", GLFW_CUSTOM_KEY_JOYSTICK_9_KEY_START},
	    {"CONTROLLER_9_AXIS_START", GLFW_CUSTOM_KEY_JOYSTICK_9_AXIS_START},
	  });

	/*lua_registerglobalint(SHADER_VERTEX_BUFFER_LOCATION);
	lua_registerglobalint(SHADER_UV_BUFFER_LOCATION);
	lua_registerglobalint(SHADER_NORMAL_BUFFER_LOCATION);*/ // Vulkan TODO
}

void pragma::CGame::InitializeLua()
{
	Game::InitializeLua();
	CallCallbacks<void, lua::State *>("OnLuaInitialized", GetLuaState());
}

void pragma::CGame::SetupLua()
{
	Game::SetupLua();
	RunLuaFiles("autorun/");
	RunLuaFiles("autorun/client/");
	InitializeGameMode();
	//for(auto &subDir : GetLuaEntityDirectories())
	//	LoadLuaEntities(subDir);

	LoadLuaShaders();
}

void pragma::CGame::LoadLuaShaders()
{
	std::vector<std::string> files;
	//fs::find_files(Lua::SCRIPT_DIRECTORY_SLASH +"shaders\\*.lua",&files,nullptr); // Deprecated; Shaders have to be explicitely included now
	for(unsigned int i = 0; i < files.size(); i++)
		LoadLuaShader(files[i]);
}

void pragma::CGame::LoadLuaShader(std::string file)
{
	string::to_lower(file);
	std::string identifier = file.substr(0, file.length() - 4);
	file = "shaders/" + file;
	ExecuteLuaFile(file, nullptr, true);
}

std::string pragma::CGame::GetLuaNetworkDirectoryName() const { return "client"; }
std::string pragma::CGame::GetLuaNetworkFileName() const { return "cl_init" + Lua::DOT_FILE_EXTENSION; }

//////////////////////////////////////////////

void pragma::ClientState::RegisterSharedLuaGlobals(Lua::Interface &lua) {}
