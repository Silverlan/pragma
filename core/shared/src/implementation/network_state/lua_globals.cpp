// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "noise/noise.h"

// import pragma.scripting.lua;

#ifdef __linux__
#define FILE_ATTRIBUTE_ARCHIVE 0x20
#define FILE_ATTRIBUTE_COMPRESSED 0x800
#define FILE_ATTRIBUTE_DEVICE 0x40
#define FILE_ATTRIBUTE_DIRECTORY 0x10
#define FILE_ATTRIBUTE_ENCRYPTED 0x4000
#define FILE_ATTRIBUTE_HIDDEN 0x2
#define FILE_ATTRIBUTE_NORMAL 0x80
#define FILE_ATTRIBUTE_NOT_CONTENT_INDEXED 0x2000
#define FILE_ATTRIBUTE_OFFLINE 0x1000
#define FILE_ATTRIBUTE_READONLY 0x1
#define FILE_ATTRIBUTE_REPARSE_POINT 0x400
#define FILE_ATTRIBUTE_SPARSE_FILE 0x200
#define FILE_ATTRIBUTE_SYSTEM 0x4
#define FILE_ATTRIBUTE_TEMPORARY 0x100
#define FILE_ATTRIBUTE_VIRTUAL 0x10000
#endif

module pragma.shared;

import :network_state;

static int32_t include(lua::State *l)
{
	std::string path = Lua::CheckString(l, 1);
	auto ignoreGlobalCache = false;
	if(Lua::IsSet(l, 2))
		ignoreGlobalCache = Lua::CheckBool(l, 2);
	auto flags = pragma::scripting::lua_core::IncludeFlags::Default;
	pragma::math::set_flag(flags, pragma::scripting::lua_core::IncludeFlags::IgnoreGlobalCache, ignoreGlobalCache);

	auto result = pragma::scripting::lua_core::include(l, path, flags);
	if(result.statusCode != Lua::StatusCode::Ok) {
		pragma::scripting::lua_core::raise_error(l, result.errorMessage); // Propagate the error on top of the stack
		// Unreachable
		return 0;
	}

	// Just return whatever was returned by the include call
	return result.numResults;
}

static int32_t exec(lua::State *l)
{
	std::string path = Lua::CheckString(l, 1);
	std::string errMsg;
	auto stackTop = Lua::GetStackTop(l);
	auto statusCode = pragma::scripting::lua_core::execute_file(l, path, &errMsg);
	if(statusCode != Lua::StatusCode::Ok) {
		pragma::scripting::lua_core::raise_error(l, errMsg); // Propagate the error on top of the stack
		// Unreachable
		return 0;
	}
	auto numResults = Lua::GetStackTop(l) - stackTop;

	// Just return whatever was returned by the include call
	return numResults;
}

void pragma::NetworkState::RegisterSharedLuaGlobals(Lua::Interface &lua)
{
	// To make sure Lua errors are handled properly, we need to use a regular Lua binding here
	// without luabind
	lua::register_function(lua.GetState(), "include", &include);
	lua::register_function(lua.GetState(), "exec", &exec);

	luabind::module(lua.GetState())[luabind::def("get_script_path", Lua::global::get_script_path)];
	lua::register_function(lua.GetState(), "toboolean", static_cast<int32_t (*)(lua::State *)>([](lua::State *l) -> int32_t {
		if(Lua::IsBool(l, 1)) {
			Lua::PushBool(l, Lua::CheckBool(l, 1));
			return 1;
		}
		if(Lua::IsNumber(l, 1)) {
			Lua::PushBool(l, Lua::CheckNumber(l, 1) != 0.f);
			return 1;
		}
		std::string v = Lua::CheckString(l, 1);
		auto b = util::to_boolean(v);
		Lua::PushBool(l, b);
		return 1;
	}));
	lua::register_function(lua.GetState(), "toint", static_cast<int32_t (*)(lua::State *)>([](lua::State *l) -> int32_t {
		if(Lua::IsBool(l, 1)) {
			Lua::PushInt(l, Lua::CheckBool(l, 1) ? 1 : 0);
			return 1;
		}
		if(Lua::IsNumber(l, 1)) {
			Lua::PushInt(l, math::round(Lua::CheckNumber(l, 1)));
			return 1;
		}
		std::string v = Lua::CheckString(l, 1);
		auto i = util::to_int(v);
		Lua::PushInt(l, i);
		return 1;
	}));

	Lua::RegisterLibraryEnums(lua.GetState(), "console",
	  {
	    {"COLOR_FLAG_NONE", math::to_integral(console::ConsoleColorFlags::None)},
	    {"COLOR_FLAG_RED_BIT", math::to_integral(console::ConsoleColorFlags::Red)},
	    {"COLOR_FLAG_GREEN_BIT", math::to_integral(console::ConsoleColorFlags::Green)},
	    {"COLOR_FLAG_BLUE_BIT", math::to_integral(console::ConsoleColorFlags::Blue)},
	    {"COLOR_FLAG_INTENSITY_BIT", math::to_integral(console::ConsoleColorFlags::Intensity)},
	    {"COLOR_FLAG_BACKGROUND_RED_BIT", math::to_integral(console::ConsoleColorFlags::BackgroundRed)},
	    {"COLOR_FLAG_BACKGROUND_GREEN_BIT", math::to_integral(console::ConsoleColorFlags::BackgroundGreen)},
	    {"COLOR_FLAG_BACKGROUND_BLUE_BIT", math::to_integral(console::ConsoleColorFlags::BackgroundBlue)},
	    {"COLOR_FLAG_BACKGROUND_INTENSITY_BIT", math::to_integral(console::ConsoleColorFlags::BackgroundIntensity)},
	    {"COLOR_FLAG_YELLOW", math::to_integral(console::ConsoleColorFlags::Yellow)},
	    {"COLOR_FLAG_MAGENTA", math::to_integral(console::ConsoleColorFlags::Magenta)},
	    {"COLOR_FLAG_CYAN", math::to_integral(console::ConsoleColorFlags::Cyan)},
	    {"COLOR_FLAG_WHITE", math::to_integral(console::ConsoleColorFlags::White)},
	    {"COLOR_FLAG_BLACK", math::to_integral(console::ConsoleColorFlags::Black)},
	    {"COLOR_FLAG_BACKGROUND_YELLOW", math::to_integral(console::ConsoleColorFlags::BackgroundYellow)},
	    {"COLOR_FLAG_BACKGROUND_MAGENTA", math::to_integral(console::ConsoleColorFlags::BackgroundMagenta)},
	    {"COLOR_FLAG_BACKGROUND_CYAN", math::to_integral(console::ConsoleColorFlags::BackgroundCyan)},
	    {"COLOR_FLAG_BACKGROUND_WHITE", math::to_integral(console::ConsoleColorFlags::BackgroundWhite)},
	    {"COLOR_FLAG_BACKGROUND_BLACK", math::to_integral(console::ConsoleColorFlags::BackgroundBlack)},
	    {"COLOR_FLAG_RESET_BIT", math::to_integral(console::ConsoleColorFlags::Reset)},

	    {"MESSAGE_FLAG_NONE", math::to_integral(console::MessageFlags::None)},
	    {"MESSAGE_FLAG_BIT_GENERIC", math::to_integral(console::MessageFlags::Generic)},
	    {"MESSAGE_FLAG_BIT_WARNING", math::to_integral(console::MessageFlags::Warning)},
	    {"MESSAGE_FLAG_BIT_ERROR", math::to_integral(console::MessageFlags::Error)},
	    {"MESSAGE_FLAG_BIT_CRITICAL", math::to_integral(console::MessageFlags::Critical)},
	    {"MESSAGE_FLAG_BIT_SERVER_SIDE", math::to_integral(console::MessageFlags::ServerSide)},
	    {"MESSAGE_FLAG_BIT_CLIENT_SIDE", math::to_integral(console::MessageFlags::ClientSide)},
	  });

	Lua::RegisterLibraryEnums(lua.GetState(), "console",
	  {
	    {"FLAG_NONE", math::to_integral(console::ConVarFlags::None)},
	    {"FLAG_BIT_CHEAT", math::to_integral(console::ConVarFlags::Cheat)},
	    {"FLAG_BIT_SINGLEPLAYER", math::to_integral(console::ConVarFlags::Singleplayer)},
	    {"FLAG_BIT_USERINFO", math::to_integral(console::ConVarFlags::Userinfo)},
	    {"FLAG_BIT_REPLICATED", math::to_integral(console::ConVarFlags::Replicated)},
	    {"FLAG_BIT_ARCHIVE", math::to_integral(console::ConVarFlags::Archive)},
	    {"FLAG_BIT_NOTIFY", math::to_integral(console::ConVarFlags::Notify)},
	    {"FLAG_BIT_JOYSTICK_AXIS_CONTINUOUS", math::to_integral(console::ConVarFlags::JoystickAxisContinuous)},
	    {"FLAG_BIT_JOYSTICK_AXIS_SINGLE", math::to_integral(console::ConVarFlags::JoystickAxisSingle)},
	    {"FLAG_BIT_HIDDEN", math::to_integral(console::ConVarFlags::Hidden)},
	    {"FLAG_BIT_PASSWORD", math::to_integral(console::ConVarFlags::Password)},
	  });

	Lua::RegisterLibraryEnums(lua.GetState(), "sound",
	  {
	    {"TYPE_GENERIC", math::to_integral(audio::ALSoundType::Generic)},
	    {"TYPE_EFFECT", math::to_integral(audio::ALSoundType::Effect)},
	    {"TYPE_MUSIC", math::to_integral(audio::ALSoundType::Music)},
	    {"TYPE_VOICE", math::to_integral(audio::ALSoundType::Voice)},
	    {"TYPE_WEAPON", math::to_integral(audio::ALSoundType::Weapon)},
	    {"TYPE_NPC", math::to_integral(audio::ALSoundType::NPC)},
	    {"TYPE_PLAYER", math::to_integral(audio::ALSoundType::Player)},
	    {"TYPE_VEHICLE", math::to_integral(audio::ALSoundType::Vehicle)},
	    {"TYPE_PHYSICS", math::to_integral(audio::ALSoundType::Physics)},
	    {"TYPE_ENVIRONMENT", math::to_integral(audio::ALSoundType::Environment)},
	    {"TYPE_GUI", math::to_integral(audio::ALSoundType::GUI)},

	    {"STATE_NO_ERROR", math::to_integral(audio::ALState::NoError)},
	    {"STATE_INITIAL", math::to_integral(audio::ALState::Initial)},
	    {"STATE_PLAYING", math::to_integral(audio::ALState::Playing)},
	    {"STATE_PAUSED", math::to_integral(audio::ALState::Paused)},
	    {"STATE_STOPPED", math::to_integral(audio::ALState::Stopped)},
	  });
}

static pragma::ecs::BaseEntity *find_entity(lua::State *l, pragma::Game &game, luabind::object o)
{
	pragma::ecs::BaseEntity *ent = nullptr;
	auto idx = luabind::object_cast_nothrow<int>(o, -1);
	if(idx != -1)
		ent = game.GetEntityByLocalIndex(idx);
	else {
		std::string targetName = Lua::CheckString(l, 1);
		pragma::ecs::EntityIterator entIt {game, pragma::ecs::EntityIterator::FilterFlags::Default | pragma::ecs::EntityIterator::FilterFlags::Pending};
		entIt.AttachFilter<EntityIteratorFilterNameOrClass>(targetName);
		auto it = entIt.begin();
		if(it != entIt.end())
			ent = *it;
		else {
			pragma::ecs::EntityIterator entIt {game, pragma::ecs::EntityIterator::FilterFlags::Default | pragma::ecs::EntityIterator::FilterFlags::Pending};
			struct EntityNamePair {
				std::vector<std::string> names;
				std::vector<pragma::ecs::BaseEntity *> ents;
				void Add(pragma::ecs::BaseEntity *ent, const std::string &name)
				{
					ents.push_back(ent);
					names.push_back(name);
				}
			};
			EntityNamePair entNames;
			EntityNamePair classNames;
			EntityNamePair mdlNames;
			for(auto *ent : entIt) {
				auto name = ent->GetName();
				if(!name.empty())
					entNames.Add(ent, name);

				auto className = ent->GetClass();
				if(!className.empty())
					entNames.Add(ent, className);

				auto mdlName = ent->GetModelName();
				if(!mdlName.empty())
					entNames.Add(ent, mdlName);
			}
			std::vector<EntityNamePair *> lists = {&entNames, &classNames, &mdlNames};
			for(auto *pair : lists) {
				std::vector<size_t> similarElements {};
				std::vector<float> similarities {};
				pragma::string::gather_similar_elements(targetName, pair->names, similarElements, 1, &similarities);
				if(!similarElements.empty() && similarities[0] >= 0.3f) {
					ent = pair->ents[similarElements.front()];
					break;
				}
			}
			for(auto *pair : lists) {
				for(auto i = decltype(pair->names.size()) {0u}; i < pair->names.size(); ++i) {
					if(pragma::string::match(pair->names[i], '*' + targetName + '*'))
						return pair->ents[i];
				}
			}
		}
	}
	return ent;
}

void pragma::Game::RegisterLuaGlobals()
{
	NetworkState::RegisterSharedLuaGlobals(GetLuaInterface());

	lua::register_function(GetLuaState(), "include_component", static_cast<int32_t (*)(lua::State *)>([](lua::State *l) -> int32_t {
		std::string componentName = Lua::CheckString(l, 1);
		auto *nw = Engine::Get()->GetNetworkState(l);
		auto *game = nw->GetGameState();
		Lua::PushBool(l, game->LoadLuaComponentByName(componentName));
		return 1;
	}));
	Lua::RegisterLibraryEnums(GetLuaState(), "game",
	  {
	    {"DAMAGETYPE_GENERIC", math::to_integral(Generic)},
	    {"DAMAGETYPE_BULLET", math::to_integral(Bullet)},
	    {"DAMAGETYPE_EXPLOSION", math::to_integral(Explosion)},
	    {"DAMAGETYPE_FIRE", math::to_integral(Fire)},
	    {"DAMAGETYPE_PLASMA", math::to_integral(Plasma)},
	    {"DAMAGETYPE_BASH", math::to_integral(Bash)},
	    {"DAMAGETYPE_CRUSH", math::to_integral(Crush)},
	    {"DAMAGETYPE_SLASH", math::to_integral(Slash)},
	    {"DAMAGETYPE_ELECTRICITY", math::to_integral(Electricity)},
	    {"DAMAGETYPE_ENERGY", math::to_integral(Energy)},

	    {"STATE_FLAG_NONE", math::to_integral(GameFlags::None)},
	    {"STATE_FLAG_BIT_MAP_INITIALIZED", math::to_integral(GameFlags::MapInitialized)},
	    {"STATE_FLAG_BIT_GAME_INITIALIZED", math::to_integral(GameFlags::GameInitialized)},
	    {"STATE_FLAG_BIT_MAP_LOADED", math::to_integral(GameFlags::MapLoaded)},
	    {"STATE_FLAG_BIT_INITIAL_TICK", math::to_integral(GameFlags::InitialTick)},
	    {"STATE_FLAG_BIT_LEVEL_TRANSITION", math::to_integral(GameFlags::LevelTransition)},
	  });

	auto *l = GetLuaState();
	Lua::GetGlobal(l, "game"); /* 1 */
	auto t = Lua::GetStackTop(l);
	Lua::PushString(l, "limits"); /* 2 */
	Lua::CreateTable(l);          /* 3 */
	Lua::SetTableValue(l, t);     /* 1 */
	Lua::Pop(l, 1);               /* 0 */

	Lua::RegisterLibraryEnums(l, "game.limits",
	  {
	    {"MAX_ABSOLUTE_LIGHTS", math::to_integral(GameLimits::MaxAbsoluteLights)},
	    {"MAX_ABSOLUTE_SHADOW_LIGHTS", math::to_integral(GameLimits::MaxAbsoluteShadowLights)},
	    {"MAX_CSM_CASCADES", math::to_integral(GameLimits::MaxCSMCascades)},
	    {"MAX_DIRECTIONAL_LIGHT_SOURCES", math::to_integral(GameLimits::MaxDirectionalLightSources)},
	    {"MAX_ACTIVE_SHADOW_MAPS", math::to_integral(GameLimits::MaxActiveShadowMaps)},
	    {"MAX_ACTIVE_SHADOW_CUBE_MAPS", math::to_integral(GameLimits::MaxActiveShadowCubeMaps)},
	    {"MAX_MESH_VERTICES", math::to_integral(GameLimits::MaxMeshVertices)},
	    {"MAX_WORLD_DISTANCE", math::to_integral(GameLimits::MaxWorldDistance)},
	    {"MAX_RAY_CAST_RANGE", math::to_integral(GameLimits::MaxRayCastRange)},
	    {"MAX_BONES", math::to_integral(GameLimits::MaxBones)},
	    {"MAX_IMAGE_ARRAY_LAYERS", math::to_integral(GameLimits::MaxImageArrayLayers)},
	  });

	Lua::RegisterLibraryEnums(GetLuaState(), "sound",
	  {
	    {"CHANNEL_AUTO", math::to_integral(audio::ALChannel::Auto)},
	    {"CHANNEL_MONO", math::to_integral(audio::ALChannel::Mono)},
	    {"CHANNEL_BOTH", math::to_integral(audio::ALChannel::Both)},
	  });

	Lua::RegisterLibraryEnums(GetLuaState(), "intersect",
	  {
	    // TODO: These should be obsolete?
	    {"RESULT_OUTSIDE", math::to_integral(math::intersection::Intersect::Outside)},
	    {"RESULT_INSIDE", math::to_integral(math::intersection::Intersect::Inside)},
	    {"RESULT_OVERLAP", math::to_integral(math::intersection::Intersect::Overlap)},
	    //

	    {"RESULT_NO_INTERSECTION", math::to_integral(math::intersection::Result::NoIntersection)},
	    {"RESULT_INTERSECT", math::to_integral(math::intersection::Result::Intersect)},
	    {"RESULT_OUT_OF_RANGE", math::to_integral(math::intersection::Result::OutOfRange)},
	  });

	Lua::RegisterLibraryEnums(GetLuaState(), "time",
	  {
	    {"TIMER_TYPE_CURTIME", math::to_integral(TimerType::CurTime)},
	    {"TIMER_TYPE_REALTIME", math::to_integral(TimerType::RealTime)},

	    {"DURATION_TYPE_NANO_SECONDS", math::to_integral(util::DurationType::NanoSeconds)},
	    {"DURATION_TYPE_MICRO_SECONDS", math::to_integral(util::DurationType::MicroSeconds)},
	    {"DURATION_TYPE_MILLI_SECONDS", math::to_integral(util::DurationType::MilliSeconds)},
	    {"DURATION_TYPE_SECONDS", math::to_integral(util::DurationType::Seconds)},
	    {"DURATION_TYPE_MINUTES", math::to_integral(util::DurationType::Minutes)},
	    {"DURATION_TYPE_HOURS", math::to_integral(util::DurationType::Hours)},
	  });

	Lua::RegisterLibraryValue<bool>(GetLuaState(), "os", "SYSTEM_WINDOWS", util::is_windows_system());
	Lua::RegisterLibraryValue<bool>(GetLuaState(), "os", "SYSTEM_LINUX", util::is_linux_system());
	Lua::RegisterLibraryValue<bool>(GetLuaState(), "os", "SYSTEM_X86", util::is_x86_system());
	Lua::RegisterLibraryValue<bool>(GetLuaState(), "os", "SYSTEM_X64", util::is_x64_system());

	Lua::RegisterLibraryEnums(GetLuaState(), "input",
	  {
	    {"ACTION_MOVEFORWARD", math::to_integral(Action::MoveForward)},
	    {"ACTION_MOVEBACKWARD", math::to_integral(Action::MoveBackward)},
	    {"ACTION_MOVELEFT", math::to_integral(Action::MoveLeft)},
	    {"ACTION_MOVERIGHT", math::to_integral(Action::MoveRight)},
	    {"ACTION_SPRINT", math::to_integral(Action::Sprint)},
	    {"ACTION_WALK", math::to_integral(Action::Walk)},
	    {"ACTION_JUMP", math::to_integral(Action::Jump)},
	    {"ACTION_CROUCH", math::to_integral(Action::Crouch)},
	    {"ACTION_ATTACK", math::to_integral(Action::Attack)},
	    {"ACTION_ATTACK2", math::to_integral(Action::Attack2)},
	    {"ACTION_ATTACK3", math::to_integral(Action::Attack3)},
	    {"ACTION_ATTACK4", math::to_integral(Action::Attack4)},
	    {"ACTION_RELOAD", math::to_integral(Action::Reload)},
	    {"ACTION_USE", math::to_integral(Action::Use)},
	    {"ACTION_LAST", math::to_integral(Action::Last)},
	  });

	Lua::RegisterLibraryEnums(GetLuaState(), "game",
	  {
	    {"HITGROUP_INVALID", math::to_integral(physics::HitGroup::Invalid)},
	    {"HITGROUP_GENERIC", math::to_integral(physics::HitGroup::Generic)},
	    {"HITGROUP_HEAD", math::to_integral(physics::HitGroup::Head)},
	    {"HITGROUP_CHEST", math::to_integral(physics::HitGroup::Chest)},
	    {"HITGROUP_STOMACH", math::to_integral(physics::HitGroup::Stomach)},
	    {"HITGROUP_LEFT_ARM", math::to_integral(physics::HitGroup::LeftArm)},
	    {"HITGROUP_RIGHT_ARM", math::to_integral(physics::HitGroup::RightArm)},
	    {"HITGROUP_LEFT_LEG", math::to_integral(physics::HitGroup::LeftLeg)},
	    {"HITGROUP_RIGHT_LEG", math::to_integral(physics::HitGroup::RightLeg)},
	    {"HITGROUP_GEAR", math::to_integral(physics::HitGroup::Gear)},
	    {"HITGROUP_TAIL", math::to_integral(physics::HitGroup::Tail)},
	  });

	Lua::RegisterLibraryEnums(GetLuaState(), "file",
	  {
	    {"FLAG_INVALID", math::to_integral(fs::FVFile::Invalid)},
	    {"FLAG_PACKAGE", math::to_integral(fs::FVFile::Package)},
	    {"FLAG_COMPRESSED", math::to_integral(fs::FVFile::Compressed)},
	    {"FLAG_DIRECTORY", math::to_integral(fs::FVFile::Directory)},
	    {"FLAG_ENCRYPTED", math::to_integral(fs::FVFile::Encrypted)},
	    {"FLAG_VIRTUAL", math::to_integral(fs::FVFile::Virtual)},
	    {"FLAG_READONLY", math::to_integral(fs::FVFile::ReadOnly)},

	    {"OPEN_MODE_READ", math::to_integral(fs::FileMode::Read)},
	    {"OPEN_MODE_WRITE", math::to_integral(fs::FileMode::Write)},
	    {"OPEN_MODE_APPEND", math::to_integral(fs::FileMode::Append)},
	    {"OPEN_MODE_BINARY", math::to_integral(fs::FileMode::Binary)},
	  });

	Lua::RegisterLibraryEnums(GetLuaState(), "file",
	  {
	    {"TYPE_VIRTUAL", math::to_integral(EVFile::Virtual)},
	    {"TYPE_LOCAL", math::to_integral(EVFile::Local)},
	    {"TYPE_PACKAGE", math::to_integral(EVFile::Package)},
	  });

	/*Lua::RegisterLibraryEnums(GetLuaState(), "file",
	  {
	    {"ATTRIBUTE_ARCHIVE", FILE_ATTRIBUTE_ARCHIVE},
	    {"ATTRIBUTE_COMPRESSED", FILE_ATTRIBUTE_COMPRESSED},
	    {"ATTRIBUTE_DEVICE", FILE_ATTRIBUTE_DEVICE},
	    {"ATTRIBUTE_DIRECTORY", FILE_ATTRIBUTE_DIRECTORY},
	    {"ATTRIBUTE_ENCRYPTED", FILE_ATTRIBUTE_ENCRYPTED},
	    {"ATTRIBUTE_HIDDEN", FILE_ATTRIBUTE_HIDDEN},
	    {"ATTRIBUTE_NORMAL", FILE_ATTRIBUTE_NORMAL},
	    {"ATTRIBUTE_NOT_CONTENT_INDEXED", FILE_ATTRIBUTE_NOT_CONTENT_INDEXED},
	    {"ATTRIBUTE_OFFLINE", FILE_ATTRIBUTE_OFFLINE},
	    {"ATTRIBUTE_READONLY", FILE_ATTRIBUTE_READONLY},
	    {"ATTRIBUTE_REPARSE_POINT", FILE_ATTRIBUTE_REPARSE_POINT},
	    {"ATTRIBUTE_SPARSE_FILE", FILE_ATTRIBUTE_SPARSE_FILE},
	    {"ATTRIBUTE_SYSTEM", FILE_ATTRIBUTE_SYSTEM},
	    {"ATTRIBUTE_TEMPORARY", FILE_ATTRIBUTE_TEMPORARY},
	    {"ATTRIBUTE_VIRTUAL", FILE_ATTRIBUTE_VIRTUAL},
	    {"ATTRIBUTE_INVALID", INVALID_FILE_ATTRIBUTES},
	  });*/

	Lua::RegisterLibraryEnums(GetLuaState(), "file",
	  {
	    {"SEARCH_NONE", math::to_integral(fs::SearchFlags::None)},
	    {"SEARCH_VIRTUAL", math::to_integral(fs::SearchFlags::Virtual)},
	    {"SEARCH_PACKAGE", math::to_integral(fs::SearchFlags::Package)},
	    {"SEARCH_LOCAL", math::to_integral(fs::SearchFlags::Local)},
	    {"SEARCH_NO_MOUNTS", math::to_integral(fs::SearchFlags::NoMounts)},
	    {"SEARCH_LOCAL_ROOT", math::to_integral(fs::SearchFlags::LocalRoot)},
	    {"SEARCH_ALL", math::to_integral(fs::SearchFlags::All)},
	    {"SEARCH_ADDON", FSYS_SEARCH_ADDON},
	  });

	auto enableShorthand = Lua::get_extended_lua_modules_enabled();
	if(enableShorthand) {
		luabind::globals(l)["ec"] = luabind::make_function(l, static_cast<luabind::object (*)(lua::State *, luabind::object, luabind::object)>([](lua::State *l, luabind::object o, luabind::object o2) -> luabind::object {
			auto &nw = *Engine::Get()->GetNetworkState(l);
			auto *game = nw.GetGameState();
			if(!game)
				return {};
			auto *ent = find_entity(l, *game, o);
			if(!Lua::IsSet(l, 2) || !ent)
				return ent ? ent->GetLuaObject() : luabind::object {};
			std::string name = Lua::CheckString(l, 2);
			auto &components = ent->GetComponents();
			std::vector<std::string> componentNames;
			componentNames.reserve(components.size());
			for(auto &c : components) {
				if(c.expired())
					continue;
				auto *info = game->GetEntityComponentManager().GetComponentInfo(c->GetComponentId());
				if(!info)
					continue;
				componentNames.push_back(info->name);
			}
			std::vector<size_t> similarElements {};
			std::vector<float> similarities {};
			string::gather_similar_elements(name, componentNames, similarElements, 1, &similarities);
			if(!similarElements.empty() && similarities.front() >= 0.3f) {
				auto &componentName = componentNames[similarElements.front()];
				auto c = ent->FindComponent(componentName);
				if(c.valid())
					return c->GetLuaObject();
			}
			componentNames.clear();
			for(auto &c : game->GetEntityComponentManager().GetRegisteredComponentTypes())
				componentNames.push_back(c->name);
			similarElements.clear();
			similarities.clear();
			string::gather_similar_elements(name, componentNames, similarElements, 1, &similarities);
			if(!similarElements.empty()) {
				auto &componentName = componentNames[similarElements.front()];
				auto c = ent->AddComponent(componentName);
				if(c.valid())
					return c->GetLuaObject();
			}
			return {};
		}));
		luabind::globals(l)["e"] = luabind::make_function(l, static_cast<luabind::object (*)(lua::State *, luabind::object)>([](lua::State *l, luabind::object o) -> luabind::object {
			auto &nw = *Engine::Get()->GetNetworkState(l);
			auto *game = nw.GetGameState();
			if(!game)
				return {};
			auto *ent = find_entity(l, *game, o);
			return ent ? ent->GetLuaObject() : luabind::object {};
		}));
	}

	Lua::udm::register_library(GetLuaInterface());
}
