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
	umath::set_flag(flags, pragma::scripting::lua_core::IncludeFlags::IgnoreGlobalCache, ignoreGlobalCache);

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

void NetworkState::RegisterSharedLuaGlobals(Lua::Interface &lua)
{
	// To make sure Lua errors are handled properly, we need to use a regular Lua binding here
	// without luabind
	lua_register(lua.GetState(), "include", &include);
	lua_register(lua.GetState(), "exec", &exec);

	luabind::module(lua.GetState())[luabind::def("get_script_path", Lua::global::get_script_path)];
	lua_register(lua.GetState(), "toboolean", static_cast<int32_t (*)(lua::State *)>([](lua::State *l) -> int32_t {
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
	lua_register(lua.GetState(), "toint", static_cast<int32_t (*)(lua::State *)>([](lua::State *l) -> int32_t {
		if(Lua::IsBool(l, 1)) {
			Lua::PushInt(l, Lua::CheckBool(l, 1) ? 1 : 0);
			return 1;
		}
		if(Lua::IsNumber(l, 1)) {
			Lua::PushInt(l, umath::round(Lua::CheckNumber(l, 1)));
			return 1;
		}
		std::string v = Lua::CheckString(l, 1);
		auto i = util::to_int(v);
		Lua::PushInt(l, i);
		return 1;
	}));

	Lua::RegisterLibraryEnums(lua.GetState(), "console",
	  {
	    {"COLOR_FLAG_NONE", umath::to_integral(pragma::console::ConsoleColorFlags::None)},
	    {"COLOR_FLAG_RED_BIT", umath::to_integral(pragma::console::ConsoleColorFlags::Red)},
	    {"COLOR_FLAG_GREEN_BIT", umath::to_integral(pragma::console::ConsoleColorFlags::Green)},
	    {"COLOR_FLAG_BLUE_BIT", umath::to_integral(pragma::console::ConsoleColorFlags::Blue)},
	    {"COLOR_FLAG_INTENSITY_BIT", umath::to_integral(pragma::console::ConsoleColorFlags::Intensity)},
	    {"COLOR_FLAG_BACKGROUND_RED_BIT", umath::to_integral(pragma::console::ConsoleColorFlags::BackgroundRed)},
	    {"COLOR_FLAG_BACKGROUND_GREEN_BIT", umath::to_integral(pragma::console::ConsoleColorFlags::BackgroundGreen)},
	    {"COLOR_FLAG_BACKGROUND_BLUE_BIT", umath::to_integral(pragma::console::ConsoleColorFlags::BackgroundBlue)},
	    {"COLOR_FLAG_BACKGROUND_INTENSITY_BIT", umath::to_integral(pragma::console::ConsoleColorFlags::BackgroundIntensity)},
	    {"COLOR_FLAG_YELLOW", umath::to_integral(pragma::console::ConsoleColorFlags::Yellow)},
	    {"COLOR_FLAG_MAGENTA", umath::to_integral(pragma::console::ConsoleColorFlags::Magenta)},
	    {"COLOR_FLAG_CYAN", umath::to_integral(pragma::console::ConsoleColorFlags::Cyan)},
	    {"COLOR_FLAG_WHITE", umath::to_integral(pragma::console::ConsoleColorFlags::White)},
	    {"COLOR_FLAG_BLACK", umath::to_integral(pragma::console::ConsoleColorFlags::Black)},
	    {"COLOR_FLAG_BACKGROUND_YELLOW", umath::to_integral(pragma::console::ConsoleColorFlags::BackgroundYellow)},
	    {"COLOR_FLAG_BACKGROUND_MAGENTA", umath::to_integral(pragma::console::ConsoleColorFlags::BackgroundMagenta)},
	    {"COLOR_FLAG_BACKGROUND_CYAN", umath::to_integral(pragma::console::ConsoleColorFlags::BackgroundCyan)},
	    {"COLOR_FLAG_BACKGROUND_WHITE", umath::to_integral(pragma::console::ConsoleColorFlags::BackgroundWhite)},
	    {"COLOR_FLAG_BACKGROUND_BLACK", umath::to_integral(pragma::console::ConsoleColorFlags::BackgroundBlack)},
	    {"COLOR_FLAG_RESET_BIT", umath::to_integral(pragma::console::ConsoleColorFlags::Reset)},

	    {"MESSAGE_FLAG_NONE", umath::to_integral(Con::MessageFlags::None)},
	    {"MESSAGE_FLAG_BIT_GENERIC", umath::to_integral(Con::MessageFlags::Generic)},
	    {"MESSAGE_FLAG_BIT_WARNING", umath::to_integral(Con::MessageFlags::Warning)},
	    {"MESSAGE_FLAG_BIT_ERROR", umath::to_integral(Con::MessageFlags::Error)},
	    {"MESSAGE_FLAG_BIT_CRITICAL", umath::to_integral(Con::MessageFlags::Critical)},
	    {"MESSAGE_FLAG_BIT_SERVER_SIDE", umath::to_integral(Con::MessageFlags::ServerSide)},
	    {"MESSAGE_FLAG_BIT_CLIENT_SIDE", umath::to_integral(Con::MessageFlags::ClientSide)},
	  });

	Lua::RegisterLibraryEnums(lua.GetState(), "console",
	  {
	    {"FLAG_NONE", umath::to_integral(pragma::console::ConVarFlags::None)},
	    {"FLAG_BIT_CHEAT", umath::to_integral(pragma::console::ConVarFlags::Cheat)},
	    {"FLAG_BIT_SINGLEPLAYER", umath::to_integral(pragma::console::ConVarFlags::Singleplayer)},
	    {"FLAG_BIT_USERINFO", umath::to_integral(pragma::console::ConVarFlags::Userinfo)},
	    {"FLAG_BIT_REPLICATED", umath::to_integral(pragma::console::ConVarFlags::Replicated)},
	    {"FLAG_BIT_ARCHIVE", umath::to_integral(pragma::console::ConVarFlags::Archive)},
	    {"FLAG_BIT_NOTIFY", umath::to_integral(pragma::console::ConVarFlags::Notify)},
	    {"FLAG_BIT_JOYSTICK_AXIS_CONTINUOUS", umath::to_integral(pragma::console::ConVarFlags::JoystickAxisContinuous)},
	    {"FLAG_BIT_JOYSTICK_AXIS_SINGLE", umath::to_integral(pragma::console::ConVarFlags::JoystickAxisSingle)},
	    {"FLAG_BIT_HIDDEN", umath::to_integral(pragma::console::ConVarFlags::Hidden)},
	    {"FLAG_BIT_PASSWORD", umath::to_integral(pragma::console::ConVarFlags::Password)},
	  });

	Lua::RegisterLibraryEnums(lua.GetState(), "sound",
	  {
	    {"TYPE_GENERIC", umath::to_integral(pragma::audio::ALSoundType::Generic)},
	    {"TYPE_EFFECT", umath::to_integral(pragma::audio::ALSoundType::Effect)},
	    {"TYPE_MUSIC", umath::to_integral(pragma::audio::ALSoundType::Music)},
	    {"TYPE_VOICE", umath::to_integral(pragma::audio::ALSoundType::Voice)},
	    {"TYPE_WEAPON", umath::to_integral(pragma::audio::ALSoundType::Weapon)},
	    {"TYPE_NPC", umath::to_integral(pragma::audio::ALSoundType::NPC)},
	    {"TYPE_PLAYER", umath::to_integral(pragma::audio::ALSoundType::Player)},
	    {"TYPE_VEHICLE", umath::to_integral(pragma::audio::ALSoundType::Vehicle)},
	    {"TYPE_PHYSICS", umath::to_integral(pragma::audio::ALSoundType::Physics)},
	    {"TYPE_ENVIRONMENT", umath::to_integral(pragma::audio::ALSoundType::Environment)},
	    {"TYPE_GUI", umath::to_integral(pragma::audio::ALSoundType::GUI)},

	    {"STATE_NO_ERROR", umath::to_integral(ALState::NoError)},
	    {"STATE_INITIAL", umath::to_integral(ALState::Initial)},
	    {"STATE_PLAYING", umath::to_integral(ALState::Playing)},
	    {"STATE_PAUSED", umath::to_integral(ALState::Paused)},
	    {"STATE_STOPPED", umath::to_integral(ALState::Stopped)},
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
				ustring::gather_similar_elements(targetName, pair->names, similarElements, 1, &similarities);
				if(!similarElements.empty() && similarities[0] >= 0.3f) {
					ent = pair->ents[similarElements.front()];
					break;
				}
			}
			for(auto *pair : lists) {
				for(auto i = decltype(pair->names.size()) {0u}; i < pair->names.size(); ++i) {
					if(ustring::match(pair->names[i], '*' + targetName + '*'))
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

	lua_register(GetLuaState(), "include_component", static_cast<int32_t (*)(lua::State *)>([](lua::State *l) -> int32_t {
		std::string componentName = Lua::CheckString(l, 1);
		auto *nw = pragma::Engine::Get()->GetNetworkState(l);
		auto *game = nw->GetGameState();
		Lua::PushBool(l, game->LoadLuaComponentByName(componentName));
		return 1;
	}));
	Lua::RegisterLibraryEnums(GetLuaState(), "game",
	  {
	    {"DAMAGETYPE_GENERIC", umath::to_integral(DAMAGETYPE::GENERIC)},
	    {"DAMAGETYPE_BULLET", umath::to_integral(DAMAGETYPE::BULLET)},
	    {"DAMAGETYPE_EXPLOSION", umath::to_integral(DAMAGETYPE::EXPLOSION)},
	    {"DAMAGETYPE_FIRE", umath::to_integral(DAMAGETYPE::FIRE)},
	    {"DAMAGETYPE_PLASMA", umath::to_integral(DAMAGETYPE::PLASMA)},
	    {"DAMAGETYPE_BASH", umath::to_integral(DAMAGETYPE::BASH)},
	    {"DAMAGETYPE_CRUSH", umath::to_integral(DAMAGETYPE::CRUSH)},
	    {"DAMAGETYPE_SLASH", umath::to_integral(DAMAGETYPE::SLASH)},
	    {"DAMAGETYPE_ELECTRICITY", umath::to_integral(DAMAGETYPE::ELECTRICITY)},
	    {"DAMAGETYPE_ENERGY", umath::to_integral(DAMAGETYPE::ENERGY)},

	    {"STATE_FLAG_NONE", umath::to_integral(pragma::Game::GameFlags::None)},
	    {"STATE_FLAG_BIT_MAP_INITIALIZED", umath::to_integral(pragma::Game::GameFlags::MapInitialized)},
	    {"STATE_FLAG_BIT_GAME_INITIALIZED", umath::to_integral(pragma::Game::GameFlags::GameInitialized)},
	    {"STATE_FLAG_BIT_MAP_LOADED", umath::to_integral(pragma::Game::GameFlags::MapLoaded)},
	    {"STATE_FLAG_BIT_INITIAL_TICK", umath::to_integral(pragma::Game::GameFlags::InitialTick)},
	    {"STATE_FLAG_BIT_LEVEL_TRANSITION", umath::to_integral(pragma::Game::GameFlags::LevelTransition)},
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
	    {"MAX_ABSOLUTE_LIGHTS", umath::to_integral(pragma::GameLimits::MaxAbsoluteLights)},
	    {"MAX_ABSOLUTE_SHADOW_LIGHTS", umath::to_integral(pragma::GameLimits::MaxAbsoluteShadowLights)},
	    {"MAX_CSM_CASCADES", umath::to_integral(pragma::GameLimits::MaxCSMCascades)},
	    {"MAX_DIRECTIONAL_LIGHT_SOURCES", umath::to_integral(pragma::GameLimits::MaxDirectionalLightSources)},
	    {"MAX_ACTIVE_SHADOW_MAPS", umath::to_integral(pragma::GameLimits::MaxActiveShadowMaps)},
	    {"MAX_ACTIVE_SHADOW_CUBE_MAPS", umath::to_integral(pragma::GameLimits::MaxActiveShadowCubeMaps)},
	    {"MAX_MESH_VERTICES", umath::to_integral(pragma::GameLimits::MaxMeshVertices)},
	    {"MAX_WORLD_DISTANCE", umath::to_integral(pragma::GameLimits::MaxWorldDistance)},
	    {"MAX_RAY_CAST_RANGE", umath::to_integral(pragma::GameLimits::MaxRayCastRange)},
	    {"MAX_BONES", umath::to_integral(pragma::GameLimits::MaxBones)},
	    {"MAX_IMAGE_ARRAY_LAYERS", umath::to_integral(pragma::GameLimits::MaxImageArrayLayers)},
	  });

	Lua::RegisterLibraryEnums(GetLuaState(), "sound",
	  {
	    {"CHANNEL_AUTO", umath::to_integral(ALChannel::Auto)},
	    {"CHANNEL_MONO", umath::to_integral(ALChannel::Mono)},
	    {"CHANNEL_BOTH", umath::to_integral(ALChannel::Both)},
	  });

	Lua::RegisterLibraryEnums(GetLuaState(), "intersect",
	  {
	    // TODO: These should be obsolete?
	    {"RESULT_OUTSIDE", umath::to_integral(umath::intersection::Intersect::Outside)},
	    {"RESULT_INSIDE", umath::to_integral(umath::intersection::Intersect::Inside)},
	    {"RESULT_OVERLAP", umath::to_integral(umath::intersection::Intersect::Overlap)},
	    //

	    {"RESULT_NO_INTERSECTION", umath::to_integral(umath::intersection::Result::NoIntersection)},
	    {"RESULT_INTERSECT", umath::to_integral(umath::intersection::Result::Intersect)},
	    {"RESULT_OUT_OF_RANGE", umath::to_integral(umath::intersection::Result::OutOfRange)},
	  });

	Lua::RegisterLibraryEnums(GetLuaState(), "time",
	  {
	    {"TIMER_TYPE_CURTIME", umath::to_integral(TimerType::CurTime)},
	    {"TIMER_TYPE_REALTIME", umath::to_integral(TimerType::RealTime)},

	    {"DURATION_TYPE_NANO_SECONDS", umath::to_integral(util::DurationType::NanoSeconds)},
	    {"DURATION_TYPE_MICRO_SECONDS", umath::to_integral(util::DurationType::MicroSeconds)},
	    {"DURATION_TYPE_MILLI_SECONDS", umath::to_integral(util::DurationType::MilliSeconds)},
	    {"DURATION_TYPE_SECONDS", umath::to_integral(util::DurationType::Seconds)},
	    {"DURATION_TYPE_MINUTES", umath::to_integral(util::DurationType::Minutes)},
	    {"DURATION_TYPE_HOURS", umath::to_integral(util::DurationType::Hours)},
	  });

	Lua::RegisterLibraryValue<bool>(GetLuaState(), "os", "SYSTEM_WINDOWS", util::is_windows_system());
	Lua::RegisterLibraryValue<bool>(GetLuaState(), "os", "SYSTEM_LINUX", util::is_linux_system());
	Lua::RegisterLibraryValue<bool>(GetLuaState(), "os", "SYSTEM_X86", util::is_x86_system());
	Lua::RegisterLibraryValue<bool>(GetLuaState(), "os", "SYSTEM_X64", util::is_x64_system());

	Lua::RegisterLibraryEnums(GetLuaState(), "input",
	  {
	    {"ACTION_MOVEFORWARD", umath::to_integral(pragma::Action::MoveForward)},
	    {"ACTION_MOVEBACKWARD", umath::to_integral(pragma::Action::MoveBackward)},
	    {"ACTION_MOVELEFT", umath::to_integral(pragma::Action::MoveLeft)},
	    {"ACTION_MOVERIGHT", umath::to_integral(pragma::Action::MoveRight)},
	    {"ACTION_SPRINT", umath::to_integral(pragma::Action::Sprint)},
	    {"ACTION_WALK", umath::to_integral(pragma::Action::Walk)},
	    {"ACTION_JUMP", umath::to_integral(pragma::Action::Jump)},
	    {"ACTION_CROUCH", umath::to_integral(pragma::Action::Crouch)},
	    {"ACTION_ATTACK", umath::to_integral(pragma::Action::Attack)},
	    {"ACTION_ATTACK2", umath::to_integral(pragma::Action::Attack2)},
	    {"ACTION_ATTACK3", umath::to_integral(pragma::Action::Attack3)},
	    {"ACTION_ATTACK4", umath::to_integral(pragma::Action::Attack4)},
	    {"ACTION_RELOAD", umath::to_integral(pragma::Action::Reload)},
	    {"ACTION_USE", umath::to_integral(pragma::Action::Use)},
	    {"ACTION_LAST", umath::to_integral(pragma::Action::Last)},
	  });

	Lua::RegisterLibraryEnums(GetLuaState(), "game",
	  {
	    {"HITGROUP_INVALID", umath::to_integral(HitGroup::Invalid)},
	    {"HITGROUP_GENERIC", umath::to_integral(HitGroup::Generic)},
	    {"HITGROUP_HEAD", umath::to_integral(HitGroup::Head)},
	    {"HITGROUP_CHEST", umath::to_integral(HitGroup::Chest)},
	    {"HITGROUP_STOMACH", umath::to_integral(HitGroup::Stomach)},
	    {"HITGROUP_LEFT_ARM", umath::to_integral(HitGroup::LeftArm)},
	    {"HITGROUP_RIGHT_ARM", umath::to_integral(HitGroup::RightArm)},
	    {"HITGROUP_LEFT_LEG", umath::to_integral(HitGroup::LeftLeg)},
	    {"HITGROUP_RIGHT_LEG", umath::to_integral(HitGroup::RightLeg)},
	    {"HITGROUP_GEAR", umath::to_integral(HitGroup::Gear)},
	    {"HITGROUP_TAIL", umath::to_integral(HitGroup::Tail)},
	  });

	Lua::RegisterLibraryEnums(GetLuaState(), "file",
	  {
	    {"FLAG_INVALID", umath::to_integral(fsys::FVFile::Invalid)},
	    {"FLAG_PACKAGE", umath::to_integral(fsys::FVFile::Package)},
	    {"FLAG_COMPRESSED", umath::to_integral(fsys::FVFile::Compressed)},
	    {"FLAG_DIRECTORY", umath::to_integral(fsys::FVFile::Directory)},
	    {"FLAG_ENCRYPTED", umath::to_integral(fsys::FVFile::Encrypted)},
	    {"FLAG_VIRTUAL", umath::to_integral(fsys::FVFile::Virtual)},
	    {"FLAG_READONLY", umath::to_integral(fsys::FVFile::ReadOnly)},

	    {"OPEN_MODE_READ", umath::to_integral(pragma::FileOpenMode::Read)},
	    {"OPEN_MODE_WRITE", umath::to_integral(pragma::FileOpenMode::Write)},
	    {"OPEN_MODE_APPEND", umath::to_integral(pragma::FileOpenMode::Append)},
	    {"OPEN_MODE_UPDATE", umath::to_integral(pragma::FileOpenMode::Update)},
	    {"OPEN_MODE_BINARY", umath::to_integral(pragma::FileOpenMode::Binary)},
	  });

	Lua::RegisterLibraryEnums(GetLuaState(), "file",
	  {
	    {"TYPE_VIRTUAL", umath::to_integral(EVFile::Virtual)},
	    {"TYPE_LOCAL", umath::to_integral(EVFile::Local)},
	    {"TYPE_PACKAGE", umath::to_integral(EVFile::Package)},
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
	    {"SEARCH_NONE", umath::to_integral(fsys::SearchFlags::None)},
	    {"SEARCH_VIRTUAL", umath::to_integral(fsys::SearchFlags::Virtual)},
	    {"SEARCH_PACKAGE", umath::to_integral(fsys::SearchFlags::Package)},
	    {"SEARCH_LOCAL", umath::to_integral(fsys::SearchFlags::Local)},
	    {"SEARCH_NO_MOUNTS", umath::to_integral(fsys::SearchFlags::NoMounts)},
	    {"SEARCH_LOCAL_ROOT", umath::to_integral(fsys::SearchFlags::LocalRoot)},
	    {"SEARCH_ALL", umath::to_integral(fsys::SearchFlags::All)},
	    {"SEARCH_ADDON", FSYS_SEARCH_ADDON},
	  });

	auto enableShorthand = Lua::get_extended_lua_modules_enabled();
	if(enableShorthand) {
		luabind::globals(l)["ec"] = luabind::make_function(l, static_cast<luabind::object (*)(lua::State *, luabind::object, luabind::object)>([](lua::State *l, luabind::object o, luabind::object o2) -> luabind::object {
			auto &nw = *pragma::Engine::Get()->GetNetworkState(l);
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
			ustring::gather_similar_elements(name, componentNames, similarElements, 1, &similarities);
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
			ustring::gather_similar_elements(name, componentNames, similarElements, 1, &similarities);
			if(!similarElements.empty()) {
				auto &componentName = componentNames[similarElements.front()];
				auto c = ent->AddComponent(componentName);
				if(c.valid())
					return c->GetLuaObject();
			}
			return {};
		}));
		luabind::globals(l)["e"] = luabind::make_function(l, static_cast<luabind::object (*)(lua::State *, luabind::object)>([](lua::State *l, luabind::object o) -> luabind::object {
			auto &nw = *pragma::Engine::Get()->GetNetworkState(l);
			auto *game = nw.GetGameState();
			if(!game)
				return {};
			auto *ent = find_entity(l, *game, o);
			return ent ? ent->GetLuaObject() : luabind::object {};
		}));
	}

	Lua::udm::register_library(GetLuaInterface());
}
