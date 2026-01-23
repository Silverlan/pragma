// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/console/helper.hpp"

module pragma.client;

import :console.register_commands;

#undef PlaySound

static void CMD_entities_cl(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &);
static void CMD_setpos(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &);
static void CMD_getpos(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_setcampos(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_getcampos(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_setang(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_getang(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_setcamang(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_getcamang(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_sound_play(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_sound_stop(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_status_cl(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_screenshot(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_shader_reload(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_shader_list(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_shader_optimize(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_debug_ai_schedule_print(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);

static void CMD_thirdperson(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_flashlight_toggle(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_reloadmaterial(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_reloadmaterials(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_fps(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
#ifdef _DEBUG
static void CMD_cl_dump_sounds(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_cl_dump_netmessages(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
#endif

namespace Console {
	namespace commands {
		static void cl_list(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
		static void cl_find(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);

		static void vk_dump_limits(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
		static void vk_dump_features(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
		static void vk_dump_format_properties(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
		static void vk_dump_image_format_properties(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
		static void vk_dump_layers(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
		static void vk_dump_extensions(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
		static void vk_dump_memory_stats(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);

		static void vk_print_memory_stats(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
	};
};

static void CMD_lua_reload_entity(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_lua_reload_weapon(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_lua_reload_entities(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_lua_reload_weapons(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);

namespace {
	using namespace pragma::console::client;
	auto UVN = register_command("cl_entities", &CMD_entities_cl, pragma::console::ConVarFlags::None, "Prints a list of all current clientside entities in the world.");
	auto UVN = register_command("setpos", &CMD_setpos, pragma::console::ConVarFlags::None, "Changes the position of the local player. Requires cheats to be enabled. Usage: setpos <x> <y> <z>");
	auto UVN = register_command("getpos", &CMD_getpos, pragma::console::ConVarFlags::None, "Prints the current position of the local player to the console.");
	auto UVN = register_command("setcampos", &CMD_setcampos, pragma::console::ConVarFlags::None, "Changes the current position of the camera.");
	auto UVN = register_command("getcampos", &CMD_getcampos, pragma::console::ConVarFlags::None, "Prints the current position of the camera.");
	auto UVN = register_command("setang", &CMD_setang, pragma::console::ConVarFlags::None, "Changes the angles of the local player. Requires cheats to be enabled. Usage: setang <pitch> <yaw> <roll>");
	auto UVN = register_command("getang", &CMD_getang, pragma::console::ConVarFlags::None, "Prints the current angles of the local player to the console.");
	auto UVN = register_command("setcamang", &CMD_setcamang, pragma::console::ConVarFlags::None, "Changes the current angles of the camera to the console.");
	auto UVN = register_command("getcamang", &CMD_getcamang, pragma::console::ConVarFlags::None, "Prints the current angles of the camera to the console.");
	auto UVN = register_command("sound_play", &CMD_sound_play, pragma::console::ConVarFlags::None, "Precaches and immediately plays the given sound file. Usage: sound_play <soundFile>");
	auto UVN = register_command("sound_stop", &CMD_sound_stop, pragma::console::ConVarFlags::None, "Stops all sounds that are currently playing.");
	auto UVN = register_command("status", &CMD_status_cl, pragma::console::ConVarFlags::None, "Prints information about the current connection to the console.");
	auto UVN = register_command("screenshot", &CMD_screenshot, pragma::console::ConVarFlags::None,
	  "Writes the contents of the screen into a .tga-file in 'screenshots/'. The name of the file will start with the map's name, or '" + pragma::engine_info::get_identifier()
	    + "' if no map is loaded. After the name follows an ID which is incremented each time. You can use 'screenshot 1' to take a screenshot without the GUI (Ingame only).");
	auto UVN = register_command("thirdperson", &CMD_thirdperson, pragma::console::ConVarFlags::None, "Toggles between first- and third-person mode.");
	auto UVN = register_command("shader_reload", &CMD_shader_reload, pragma::console::ConVarFlags::None, "Reloads a specific shader, or all registered shaders if no arguments are given. Usage: shader_reload <shaderName>");
	auto UVN = register_command("shader_list", &CMD_shader_list, pragma::console::ConVarFlags::None, "Prints a list of all currently loaded shaders");
	auto UVN = register_command("shader_optimize", &CMD_shader_optimize, pragma::console::ConVarFlags::None, "Uses LunarGLASS to optimize the specified shader.");
	auto UVN = register_command("debug_ai_schedule_print", &CMD_debug_ai_schedule_print, pragma::console::ConVarFlags::None, "Prints the current schedule behavior tree for the specified NPC.");

	auto UVN = register_command("flashlight", &CMD_flashlight_toggle, pragma::console::ConVarFlags::None, "Toggles the flashlight.");
	auto UVN = register_command("reloadmaterial", &CMD_reloadmaterial, pragma::console::ConVarFlags::None, "Reloads the given material (and textures associated with it). Usage: reloadmaterial <materialFile>");
	auto UVN = register_command("reloadmaterials", &CMD_reloadmaterials, pragma::console::ConVarFlags::None, "Reloads all loaded materials and textures.");
	auto UVN = register_command("cl_list", &Console::commands::cl_list, pragma::console::ConVarFlags::None, "Prints a list of all clientside console commands to the console.");
	auto UVN = register_command("cl_find", &Console::commands::cl_find, pragma::console::ConVarFlags::None, "Finds similar console commands to whatever was given as argument.");
	auto UVN = register_command("fps", &CMD_fps, pragma::console::ConVarFlags::None, "Prints the current fps to the console.");

	auto UVN = register_command("vk_dump_limits", &Console::commands::vk_dump_limits, pragma::console::ConVarFlags::None, "Dumps all vulkan limitations to 'vk_limits.txt'.");
	auto UVN = register_command("vk_dump_features", &Console::commands::vk_dump_features, pragma::console::ConVarFlags::None, "Dumps all vulkan features to 'vk_features.txt'.");
	auto UVN = register_command("vk_dump_format_properties", &Console::commands::vk_dump_format_properties, pragma::console::ConVarFlags::None, "Dumps all vulkan format properties to 'vk_format_properties.txt'.");
	auto UVN = register_command("vk_dump_image_format_properties", &Console::commands::vk_dump_image_format_properties, pragma::console::ConVarFlags::None, "Dumps all vulkan image format properties to 'vk_image_format_properties.txt'.");
	auto UVN = register_command("vk_dump_layers", &Console::commands::vk_dump_layers, pragma::console::ConVarFlags::None, "Dumps all available vulkan layers to 'vk_layers.txt'.");
	auto UVN = register_command("vk_dump_extensions", &Console::commands::vk_dump_extensions, pragma::console::ConVarFlags::None, "Dumps all available vulkan extensions to 'vk_extensions.txt'.");
	auto UVN = register_command("vk_dump_memory_stats", &Console::commands::vk_dump_memory_stats, pragma::console::ConVarFlags::None, "Dumps information about currently allocated GPU memory.");
	auto UVN = register_command("vk_print_memory_stats", &Console::commands::vk_print_memory_stats, pragma::console::ConVarFlags::None, "Prints information about currently allocated GPU memory to the console.");

#ifdef _DEBUG
	auto UVN = register_command("cl_dump_sounds", &CMD_cl_dump_sounds, pragma::console::ConVarFlags::None, "Prints information about all active sounds to the console.");
	auto UVN = register_command("cl_dump_netmessages", &CMD_cl_dump_netmessages, pragma::console::ConVarFlags::None, "Prints all registered netmessages to the console.");
#endif

	auto UVN = register_command("lua_reload_entity", &CMD_lua_reload_entity, pragma::console::ConVarFlags::None, "Reloads the scripts for the given entity class. Usage: lua_reload_entity <className>");
	auto UVN = register_command("lua_reload_weapon", &CMD_lua_reload_weapon, pragma::console::ConVarFlags::None, "Reloads the scripts for the given weapon class. Usage: lua_reload_weapon <className>");
	auto UVN = register_command("lua_reload_entities", &CMD_lua_reload_entities, pragma::console::ConVarFlags::None, "Reloads the scripts for all registered lua entity classes.");
	auto UVN = register_command("lua_reload_weapons", &CMD_lua_reload_weapons, pragma::console::ConVarFlags::None, "Reloads the scripts for all registered lua weapon classes.");
}

void CMD_lua_reload_entity(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	if(argv.empty())
		return;
	auto *cstate = static_cast<pragma::ClientState *>(state);
	auto *game = cstate->GetGameState();
	if(game == nullptr)
		return;
	game->LoadLuaEntity("entities", argv[0]);
}

void CMD_lua_reload_weapon(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	if(argv.empty())
		return;
	auto *cstate = static_cast<pragma::ClientState *>(state);
	auto *game = cstate->GetGameState();
	if(game == nullptr)
		return;
	game->LoadLuaEntity("weapons", argv[0]);
}

void CMD_lua_reload_entities(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	auto *server = pragma::get_cengine()->GetServerNetworkState();
	pragma::Game *sgame = server->GetGameState();
	if(sgame != nullptr)
		sgame->LoadLuaEntities("entities");
	auto *client = pragma::get_cengine()->GetClientState();
	pragma::Game *cgame = client->GetGameState();
	if(cgame != nullptr)
		cgame->LoadLuaEntities("entities");
}

void CMD_lua_reload_weapons(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	auto *server = pragma::get_cengine()->GetServerNetworkState();
	pragma::Game *sgame = server->GetGameState();
	if(sgame != nullptr)
		sgame->LoadLuaEntities("weapons");
	auto *client = pragma::get_cengine()->GetClientState();
	pragma::Game *cgame = client->GetGameState();
	if(cgame != nullptr)
		cgame->LoadLuaEntities("weapons");
}

void CMD_entities_cl(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	if(!state->IsGameActive())
		return;
	auto sortedEnts = pragma::console::get_sorted_entities(*pragma::get_cgame(), pl);
	std::optional<std::string> className = {};
	if(argv.empty() == false)
		className = '*' + argv.front() + '*';
	std::optional<std::string> modelName {};
	if(argv.size() > 1)
		modelName = '*' + argv[1] + '*';
	for(auto &pair : sortedEnts) {
		if(className.has_value() && pragma::string::match(pair.first->GetClass().c_str(), className->c_str()) == false)
			continue;
		if(modelName.has_value() && pragma::string::match(pair.first->GetModelName().c_str(), modelName->c_str()) == false)
			continue;
		Con::COUT << *pair.first << Con::endl;
	}
}

void CMD_thirdperson(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	if(pl == nullptr)
		return;
	auto *observableC = static_cast<pragma::CPlayerComponent *>(pl)->GetObservableComponent();
	if(!observableC)
		return;
	auto *observer = observableC->GetObserver();
	if(!observer)
		return;
	auto *cstate = static_cast<pragma::ClientState *>(state);
	if(!pragma::check_cheats("thirdperson", cstate))
		return;
	auto bThirdPerson = false;
	if(!argv.empty())
		bThirdPerson = (pragma::string::to_int(argv.front()) != 0) ? true : false;
	else
		bThirdPerson = (observer->GetObserverMode() != ObserverMode::ThirdPerson) ? true : false;
	auto obsTarget = observer->GetObserverTarget();
	if(obsTarget) {
		if((bThirdPerson && obsTarget->IsCameraEnabled(pragma::BaseObservableComponent::CameraType::ThirdPerson) == false) || (bThirdPerson == false && obsTarget->IsCameraEnabled(pragma::BaseObservableComponent::CameraType::FirstPerson) == false))
			return;
	}
	observer->SetObserverMode((bThirdPerson == true) ? ObserverMode::ThirdPerson : ObserverMode::FirstPerson);
}

void CMD_setpos(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	if(argv.size() < 3)
		return;
	auto *cstate = static_cast<pragma::ClientState *>(state);
	if(!pragma::check_cheats("setpos", cstate))
		return;
    Vector3 pos {
		pragma::string::to_float(argv[0]),
		pragma::string::to_float(argv[1]),
		pragma::string::to_float(argv[2])
    };
	NetPacket p;
	pragma::networking::write_vector(p, pos);
	cstate->SendPacket(pragma::networking::net_messages::server::CMD_SETPOS, p, pragma::networking::Protocol::SlowReliable);
}

void CMD_getpos(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	auto *game = static_cast<pragma::CGame *>(state->GetGameState());
	if(pl == nullptr) {
		Con::COUT << "0 0 0" << Con::endl;
		return;
	}
	auto *cPl = game->GetLocalPlayer();
	auto pTrComponent = cPl->GetEntity().GetTransformComponent();
	if(pTrComponent == nullptr) {
		Con::COUT << "0 0 0" << Con::endl;
		return;
	}
	auto &pos = pTrComponent->GetPosition();
	Con::COUT << pos.x << " " << pos.y << " " << pos.z << Con::endl;
}

void CMD_setcampos(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	if(!state->IsGameActive())
		return;
	if(argv.size() < 3)
		return;
	auto *cstate = static_cast<pragma::ClientState *>(state);
	if(!pragma::check_cheats("setpos", cstate))
		return;
    Vector3 pos {
		pragma::string::to_float(argv[0]),
		pragma::string::to_float(argv[1]),
		pragma::string::to_float(argv[2])
    };
	auto *game = static_cast<pragma::CGame *>(state->GetGameState());
	auto *pCam = game->GetRenderCamera<pragma::CCameraComponent>();
	if(pCam == nullptr)
		return;
	pCam->GetEntity().SetPosition(pos);
}

void CMD_getcampos(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	if(!state->IsGameActive())
		return;
	auto *game = static_cast<pragma::CGame *>(state->GetGameState());
	auto *pCam = game->GetRenderCamera<pragma::CCameraComponent>();
	if(pCam == nullptr)
		return;
	auto &pos = pCam->GetEntity().GetPosition();
	Con::COUT << pos.x << " " << pos.y << " " << pos.z << Con::endl;
}

void CMD_setang(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	if(argv.size() < 3)
		return;
	auto *cstate = static_cast<pragma::ClientState *>(state);
	if(!cstate->IsGameActive())
		return;
	if(pl == nullptr)
		return;
	if(!pragma::check_cheats("setang", cstate))
		return;
	auto charComponent = pl->GetEntity().GetCharacterComponent();
	if(charComponent.expired())
		return;
	EulerAngles ang(pragma::util::to_float(argv[0]), pragma::util::to_float(argv[1]), pragma::util::to_float(argv[2]));
	charComponent->SetViewAngles(ang);
}

void CMD_getang(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	if(pl == nullptr) {
		Con::COUT << "0 0 0" << Con::endl;
		return;
	}
	auto charComponent = pl->GetEntity().GetCharacterComponent();
	if(charComponent.expired())
		return;
	EulerAngles ang = charComponent->GetViewAngles();
	Con::COUT << ang.p << " " << ang.y << " " << ang.r << Con::endl;
}

void CMD_setcamang(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	if(!state->IsGameActive())
		return;
	if(argv.size() < 3)
		return;
	auto *cstate = static_cast<pragma::ClientState *>(state);
	if(!pragma::check_cheats("setpos", cstate))
		return;
	EulerAngles ang(pragma::util::to_float(argv[0]), pragma::util::to_float(argv[1]), pragma::util::to_float(argv[2]));
	auto *game = static_cast<pragma::CGame *>(state->GetGameState());
	auto *pCam = game->GetRenderCamera<pragma::CCameraComponent>();
	if(pCam == nullptr)
		return;
	pCam->GetEntity().SetRotation(uquat::create(ang));
}

void CMD_getcamang(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	auto *game = static_cast<pragma::CGame *>(state->GetGameState());
	auto *pCam = game->GetRenderCamera<pragma::CCameraComponent>();
	if(pCam == nullptr)
		return;
	auto ang = EulerAngles {pCam->GetEntity().GetRotation()};
	Con::COUT << ang.p << " " << ang.y << " " << ang.r << Con::endl;
}

void CMD_sound_play(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	if(argv.empty())
		return;
	auto *client = pragma::get_client_state();
	if(client->PrecacheSound(argv[0]) == false)
		return;
	client->PlaySound(argv[0], pragma::audio::ALSoundType::GUI, pragma::audio::ALCreateFlags::None);
}

void CMD_sound_stop(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &) { pragma::get_client_state()->StopSounds(); }

void CMD_status_cl(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	auto &players = pragma::CPlayerComponent::GetAll();
	auto *cl = pragma::get_client_state()->GetClient();
	if(cl == nullptr) {
		Con::CWAR << "Not connected to a server!" << Con::endl;
		return;
	}
	Con::COUT << "hostname:\t"
	          << "Unknown" << Con::endl;
	Con::COUT << "udp/ip:\t\t" << cl->GetIdentifier() << Con::endl;
	Con::COUT << "map:\t\t"
	          << "Unknown" << Con::endl;
	Con::COUT << "players:\t" << players.size() << " (" << 0 << " max)" << Con::endl << Con::endl;
	Con::COUT << "#  userid\tname    \tconnected\tping";
	Con::COUT << Con::endl;
	auto i = 0u;
	for(auto *plComponent : players) {
		auto nameC = plComponent->GetEntity().GetNameComponent();
		Con::COUT << "# \t" << i << "\t"
		          << "\"" << (nameC.valid() ? nameC->GetName() : "") << "\""
		          << "\t" << pragma::string::format_time(plComponent->TimeConnected()) << "     \t";
		if(plComponent->IsLocalPlayer() == true)
			Con::COUT << cl->GetLatency();
		else
			Con::COUT << "?";
		Con::COUT << Con::endl;
		++i;
	}
}

#ifdef _DEBUG
void CMD_cl_dump_sounds(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	auto &sounds = pragma::get_client_state()->GetSounds();
	for(auto &sndInfo : sounds) {
		auto &snd = sndInfo.sound;
		if(sndInfo.container == false) {
			Con::COUT << sndInfo.index << ": ";
			if(snd == nullptr)
				Con::COUT << "NULL";
			else {
				auto *csnd = static_cast<CALSound *>(snd.get());
				auto src = csnd->GetSource();
				auto buf = csnd->GetBuffer();
				Con::COUT << "Buffer " << buf << " on source " << src << ";";
				auto state = csnd->GetState();
				Con::COUT << " State: ";
				switch(state) {
				case AL_INITIAL:
					Con::COUT << "Initial";
					break;
				case AL_PLAYING:
					Con::COUT << "Playing";
					break;
				case AL_PAUSED:
					Con::COUT << "Paused";
					break;
				case AL_STOPPED:
					Con::COUT << "Stopped";
					break;
				default:
					Con::COUT << "Unknown";
					break;
				}
				Con::COUT << "; Source: ";
				std::string name;
				if(pragma::get_client_state()->GetSoundName(buf, name) == true)
					Con::COUT << name;
				else
					Con::COUT << "Unknown";
			}
			Con::COUT << Con::endl;
		}
	}
}

void CMD_cl_dump_netmessages(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	auto *map = GetClientMessageMap();
	std::unordered_map<std::string, unsigned int> *netmessages;
	map->GetNetMessages(&netmessages);
	if(!argv.empty()) {
		auto id = pragma::string::to_int(argv.front());
		for(auto it = netmessages->begin(); it != netmessages->end(); ++it) {
			if(it->second == id) {
				Con::COUT << "Message Identifier: " << it->first << Con::endl;
				return;
			}
		}
		Con::COUT << "No message with id " << id << " found!" << Con::endl;
		return;
	}
	for(auto it = netmessages->begin(); it != netmessages->end(); ++it)
		Con::COUT << it->first << " = " << it->second << Con::endl;
}
#endif

void CMD_screenshot(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	auto *game = pragma::get_client_state()->GetGameState();
	if(game == nullptr)
		return;

	std::unordered_map<std::string, pragma::console::CommandOption> commandOptions {};
	pragma::console::parse_command_options(argv, commandOptions);

	auto mode = pragma::console::get_command_option_parameter_value(commandOptions, "mode");
	if(pragma::string::compare<std::string>(mode, "raytracing", false)) {
		auto resolution = pragma::get_cengine()->GetRenderResolution();
		auto width = pragma::util::to_uint(pragma::console::get_command_option_parameter_value(commandOptions, "width", std::to_string(resolution.x)));
		auto height = pragma::util::to_uint(pragma::console::get_command_option_parameter_value(commandOptions, "height", std::to_string(resolution.y)));

		auto format = pragma::image::ImageFormat::PNG;
		auto itFormat = commandOptions.find("format");
		if(itFormat != commandOptions.end()) {
			std::string customFormat {};
			if(itFormat->second.parameters.empty() == false)
				customFormat = itFormat->second.parameters.front();
			auto eCustomFormat = pragma::image::string_to_image_output_format(customFormat);
			if(eCustomFormat.has_value())
				format = *eCustomFormat;
			else
				Con::CWAR << "Unsupported format '" << customFormat << "'! Using PNG instead..." << Con::endl;
		}

		pragma::util::RtScreenshotSettings settings {};
		settings.samples = pragma::util::to_uint(pragma::console::get_command_option_parameter_value(commandOptions, "samples", "1024"));
		settings.denoise = true;
		auto itDenoise = commandOptions.find("nodenoise");
		if(itDenoise != commandOptions.end())
			settings.denoise = false;

		auto quality = 1.f;
		auto itQuality = commandOptions.find("quality");
		if(itQuality != commandOptions.end() && itQuality->second.parameters.empty() == false)
			quality = pragma::util::to_float(itQuality->second.parameters.front());
		settings.quality = quality;

		auto toneMapping = pragma::image::ToneMapping::GammaCorrection;
		auto itToneMapping = commandOptions.find("tone_mapping");
		if(itToneMapping != commandOptions.end() && itToneMapping->second.parameters.empty() == false) {
			auto customToneMapping = pragma::image::string_to_tone_mapping(itToneMapping->second.parameters.front());
			if(customToneMapping.has_value() == false)
				Con::CWAR << "'" << itToneMapping->second.parameters.front() << "' is not a valid tone mapper!" << Con::endl;
			else
				toneMapping = *customToneMapping;
		}
		settings.toneMapping = toneMapping;

		auto itSky = commandOptions.find("sky");
		if(itSky != commandOptions.end() && itSky->second.parameters.empty() == false)
			settings.sky = itSky->second.parameters.front();

		auto itSkyStrength = commandOptions.find("sky_strength");
		if(itSkyStrength != commandOptions.end() && itSkyStrength->second.parameters.empty() == false)
			settings.skyStrength = pragma::util::to_float(itSkyStrength->second.parameters.front());

		auto itSkyAngles = commandOptions.find("sky_angles");
		if(itSkyAngles != commandOptions.end() && itSkyAngles->second.parameters.empty() == false)
			settings.skyAngles = EulerAngles {itSkyAngles->second.parameters.front()};

		pragma::util::rt_screenshot(*game, width, height, settings, format);
		return;
	}

	auto path = pragma::util::screenshot(*game);
	if(path)
		Con::COUT << "Saved screenshot as '" << *path << "'!" << Con::endl;
}

void CMD_shader_reload(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	if(argv.empty()) {
		auto &shaderManager = pragma::get_cengine()->GetShaderManager();
		for(auto &pair : shaderManager.GetShaderNameToIndexTable()) {
			Con::COUT << "Reloading shader '" << pair.first << "'..." << Con::endl;
			pragma::get_cengine()->ReloadShader(pair.first);
		}
		Con::COUT << "All shaders have been reloaded!" << Con::endl;
		return;
	}
	pragma::get_cengine()->ReloadShader(argv.front());
}

void CMD_shader_optimize(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	std::unordered_map<std::string, pragma::console::CommandOption> commandOptions {};
	pragma::console::parse_command_options(argv, commandOptions);
	if(argv.empty()) {
		Con::CWAR << "No shader specified!" << Con::endl;
		return;
	}
	auto &shaderName = argv.front();
	auto &renderContext = pragma::get_cengine()->GetRenderContext();
	if(renderContext.GetAPIAbbreviation() != "VK") {
		Con::CWAR << "Shader optimization only supported for Vulkan!" << Con::endl;
		return;
	}
	auto shader = renderContext.GetShader(shaderName);
	if(shader.expired()) {
		Con::CWAR << "Shader '" << shaderName << "' not found!" << Con::endl;
		return;
	}
	if(shader->IsValid() == false) {
		Con::CWAR << "Shader '" << shaderName << "' is invalid!" << Con::endl;
		return;
	}
	std::unordered_map<prosper::ShaderStage, std::string> shaderStages;
	for(auto &stageData : shader->GetStages()) {
		if(stageData == nullptr)
			continue;
		shaderStages[stageData->stage] = stageData->path;
	}
	std::string infoLog;
	auto optimizedShaders = renderContext.OptimizeShader(shaderStages, infoLog);
	if(optimizedShaders.has_value() == false) {
		Con::CWAR << "Unable to optimize shader: " << infoLog << Con::endl;
		return;
	}
	auto validate = pragma::console::get_command_option_parameter_value(commandOptions, "validate", "0");
	if(pragma::util::to_boolean(validate)) {
		Con::COUT << "Optimization complete!" << Con::endl;
		return; // Don't save shaders
	}
	Con::COUT << "Optimization complete! Saving optimized shader files..." << Con::endl;
	std::string outputPath = "addons/vulkan/";
	auto reload = pragma::util::to_boolean(pragma::console::get_command_option_parameter_value(commandOptions, "reload", "0"));
	for(auto &pair : *optimizedShaders) {
		auto itSrc = shaderStages.find(pair.first);
		if(itSrc == shaderStages.end())
			continue;
		auto shaderFile = renderContext.FindShaderFile(pair.first, "shaders/" + itSrc->second);
		if(shaderFile.has_value() == false) {
			Con::CWAR << "Unable to find shader file for '" << pair.second << "'!" << Con::endl;
			return;
		}
		auto fileName = outputPath + *shaderFile;
		ufile::remove_extension_from_filename(fileName);
		fileName += "_vk.gls";
		if(reload == false && pragma::fs::exists(fileName))
			continue;
		pragma::fs::create_path(ufile::get_path_from_filename(fileName));
		auto f = pragma::fs::open_file<pragma::fs::VFilePtrReal>(fileName, pragma::fs::FileMode::Write);
		if(f == nullptr) {
			Con::CWAR << "Unable to open file '" << fileName << "' for writing!" << Con::endl;
			return;
		}
		f->WriteString(pair.second);
		f = nullptr;
	}
}

void CMD_shader_list(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	auto &shaderManager = pragma::get_cengine()->GetShaderManager();
	std::vector<std::shared_ptr<prosper::Shader>> shaderList;
	auto shaders = shaderManager.GetShaders();
	shaderList.reserve(shaders.size());
	for(auto &hShader : shaders)
		shaderList.push_back(hShader);
	std::sort(shaderList.begin(), shaderList.end(), [](const std::shared_ptr<prosper::Shader> &a, const std::shared_ptr<prosper::Shader> &b) { return (a->GetIdentifier() < b->GetIdentifier()) ? true : false; });
	for(auto &shader : shaderList) {
		auto &id = shader->GetIdentifier();
		Con::COUT << id;
		if(shader->IsComputeShader())
			Con::COUT << " (Compute)";
		else if(shader->IsGraphicsShader())
			Con::COUT << " (Graphics)";
		else
			Con::COUT << " (Unknown)";
		auto shaderSources = shader->GetSourceFilePaths();
		for(auto &src : shaderSources)
			Con::COUT << " (" << src << ")";
		Con::COUT << Con::endl;
	}
}

void CMD_flashlight_toggle(pragma::NetworkState *, pragma::BasePlayerComponent *pl, std::vector<std::string> &)
{
	auto *game = pragma::get_client_state()->GetGameState();
	if(game == nullptr)
		return;
	if(pl == nullptr)
		return;
	pl->ToggleFlashlight();
}

void CMD_debug_ai_schedule_print(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	if(!check_cheats("debug_ai_schedule_print", state))
		return;
	if(pragma::get_cgame() == nullptr || pl == nullptr)
		return;
	auto charComponent = pl->GetEntity().GetCharacterComponent();
	if(charComponent.expired())
		return;
	auto ents = pragma::console::find_target_entity(state, *charComponent, argv);
	pragma::ecs::BaseEntity *npc = nullptr;
	for(auto *ent : ents) {
		if(ent->IsNPC() == false)
			continue;
		npc = ent;
		break;
	}
	if(npc == nullptr) {
		Con::CWAR << "No valid NPC target found!" << Con::endl;
		return;
	}
	Con::COUT << "Querying schedule data for NPC " << *npc << "..." << Con::endl;
	NetPacket p;
	pragma::networking::write_entity(p, npc);
	pragma::get_client_state()->SendPacket(pragma::networking::net_messages::server::DEBUG_AI_SCHEDULE_PRINT, p, pragma::networking::Protocol::SlowReliable);
}

void CMD_reloadmaterial(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	if(!check_cheats("reloadmaterial", state))
		return;
	if(argv.empty())
		return;
	Con::COUT << "Reloading '" << argv[0] << "'..." << Con::endl;
	pragma::get_client_state()->LoadMaterial(argv[0].c_str(), nullptr, true);
}

void CMD_reloadmaterials(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!check_cheats("reloadmaterials", state))
		return;
}

void Console::commands::cl_list(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	auto &convars = state->GetConVars();
	std::vector<std::string> cvars(convars.size());
	size_t idx = 0;
	for(auto &pair : convars) {
		cvars[idx] = pair.first;
		idx++;
	}
	std::sort(cvars.begin(), cvars.end());
	std::vector<std::string>::iterator it;
	for(it = cvars.begin(); it != cvars.end(); it++)
		Con::COUT << *it << Con::endl;
}

void Console::commands::cl_find(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	if(argv.empty()) {
		Con::CWAR << "No argument given!" << Con::endl;
		return;
	}
	auto similar = state->FindSimilarConVars(argv.front());
	if(similar.empty()) {
		Con::COUT << "No potential candidates found!" << Con::endl;
		return;
	}
	Con::COUT << "Found " << similar.size() << " potential candidates:" << Con::endl;
	for(auto &name : similar)
		Con::COUT << "- " << name << Con::endl;
}

void CMD_fps(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &) { Con::COUT << "FPS: " << pragma::util::round_string(pragma::get_cengine()->GetFPS(), 0) << Con::endl << "Frame Time: " << pragma::util::round_string(pragma::get_cengine()->GetFrameTime(), 2) << "ms" << Con::endl; }

static void write_to_file(const std::string &fileName, const std::optional<std::string> &contents)
{
	if(!contents) {
		Con::CWAR << "Unable to dump '" << fileName << "': No contents available!" << Con::endl;
		return;
	}
	if(!pragma::fs::write_file(fileName, *contents)) {
		Con::CWAR << "Unable to write '" << fileName << "'!" << Con::endl;
		return;
	}
	Con::COUT << "Dumped contents to '" << fileName << "'!" << Con::endl;
}
void Console::commands::vk_dump_limits(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	auto limits = pragma::get_cengine()->GetRenderContext().DumpLimits();
	write_to_file("vk_limits.txt", limits);
}
void Console::commands::vk_dump_features(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	auto limits = pragma::get_cengine()->GetRenderContext().DumpFeatures();
	write_to_file("vk_features.txt", limits);
}
void Console::commands::vk_dump_format_properties(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	auto limits = pragma::get_cengine()->GetRenderContext().DumpFormatProperties();
	write_to_file("vk_format_properties.txt", limits);
}
void Console::commands::vk_dump_image_format_properties(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	auto limits = pragma::get_cengine()->GetRenderContext().DumpImageFormatProperties();
	write_to_file("vk_image_format_properties.txt", limits);
}
void Console::commands::vk_dump_layers(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	auto limits = pragma::get_cengine()->GetRenderContext().DumpLayers();
	write_to_file("vk_layers.txt", limits);
}
void Console::commands::vk_dump_extensions(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	auto limits = pragma::get_cengine()->GetRenderContext().DumpExtensions();
	write_to_file("vk_extensions.txt", limits);
}
/*static void print_memory_stats(std::stringstream &ss,Vulkan::MemoryManager::StatInfo &info)
{
	ss<<"Number of allocations: "<<info.allocationCount<<"\n";
	ss<<"Allocation size bounds (avg,min,max): "<<
		pragma::util::get_pretty_bytes(info.allocationSizeAvg)<<","<<
		pragma::util::get_pretty_bytes(info.allocationSizeMin)<<","<<
		pragma::util::get_pretty_bytes(info.allocationSizeMax)<<",\n";
	ss<<"Block count: "<<info.blockCount<<"\n";
	ss<<"Used data: "<<pragma::util::get_pretty_bytes(info.usedBytes)<<"\n";
	ss<<"Unused data: "<<pragma::util::get_pretty_bytes(info.unusedBytes)<<"\n";
	ss<<"Unused range (count,avg,min,max): "<<info.unusedRangeCount<<","<<
		pragma::util::get_pretty_bytes(info.unusedRangeSizeAvg)<<","<<
		pragma::util::get_pretty_bytes(info.unusedRangeSizeMin)<<","<<
		pragma::util::get_pretty_bytes(info.unusedRangeSizeMax)<<"\n";
	ss<<"\n";
}*/ // prosper TODO
void Console::commands::vk_dump_memory_stats(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	/*auto &context = pragma::get_cengine()->GetRenderContext();
	auto &memoryMan = context.GetMemoryManager();
	auto stats = memoryMan.GetStatistics();
	auto f = fs::open_file<fs::VFilePtrReal>("vk_memory_stats.txt",fs::FileMode::Write);
	if(f == nullptr)
		return;
	std::stringstream ss;
	ss<<"Total memory usage:\n";
	print_memory_stats(ss,stats.total);
	ss<<"\n";
	auto idx = 0u;
	for(auto &heapStats : stats.memoryHeap)
	{
		ss<<"Heap "<<idx++<<" memory usage:\n";
		print_memory_stats(ss,heapStats);
	}
	ss<<"\n";
	idx = 0u;
	for(auto &heapStats : stats.memoryHeap)
	{
		ss<<"Type "<<idx++<<" memory usage:\n";
		print_memory_stats(ss,heapStats);
	}
	f->WriteString(ss.str());*/ // prosper TODO
}

void Console::commands::vk_print_memory_stats(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
#if 0
	//bool prosper::util::get_memory_stats(IPrContext &context,MemoryPropertyFlags memPropFlags,DeviceSize &outAvailableSize,DeviceSize &outAllocatedSize)
	prosper::DeviceSize availableSize,allocatedSize;
	std::vector<uint32_t> memIndices;
	auto r = prosper::util::get_memory_stats(pragma::get_cengine()->GetRenderContext(),prosper::MemoryPropertyFlags::DeviceLocalBit,availableSize,allocatedSize,&memIndices);
	if(r == false)
	{
		Con::CWAR<<"No device local memory types found!"<<Con::endl;
		return;
	}
	std::stringstream ss;
	ss<<"Total Available GPU Memory: "<<pragma::util::get_pretty_bytes(availableSize)<<"\n";
	ss<<"Memory in use: "<<pragma::util::get_pretty_bytes(allocatedSize)<<" ("<<pragma::math::round(allocatedSize /static_cast<double>(availableSize) *100.0,2)<<"%)\n";
	ss<<"Memory usage by resource type:\n";
	const std::unordered_map<prosper::MemoryTracker::Resource::TypeFlags,std::string> types = {
		{prosper::MemoryTracker::Resource::TypeFlags::StandAloneBufferBit,"Dedicated buffers"},
		{prosper::MemoryTracker::Resource::TypeFlags::UniformBufferBit,"Uniform resizable buffers"},
		{prosper::MemoryTracker::Resource::TypeFlags::DynamicBufferBit,"Dynamic resizable buffers"},
		{prosper::MemoryTracker::Resource::TypeFlags::ImageBit,"Images"}
	};
	auto &memTracker = prosper::MemoryTracker::GetInstance();
	for(auto &pair : types)
	{
		auto allocatedSizeOfType = 0ull;
		std::vector<const prosper::MemoryTracker::Resource*> resources {};
		for(auto idx : memIndices)
		{
			uint64_t allocatedSize = 0ull;
			uint64_t totalSize = 0ull;
			memTracker.GetMemoryStats(pragma::get_cengine()->GetRenderContext(),idx,allocatedSize,totalSize,pair.first);
			allocatedSizeOfType += allocatedSize;

			memTracker.GetResources(idx,resources,pair.first);
		}
		std::vector<uint64_t> resourceSizes {};
		resourceSizes.reserve(resources.size());
		std::vector<size_t> sortedIndices = {};
		auto i = 0ull;
		for(auto *res : resources)
		{
			sortedIndices.push_back(i++);
			auto numMemoryBlocks = res->GetMemoryBlockCount();
			auto size = 0ull;
			for(auto i=decltype(numMemoryBlocks){0u};i<numMemoryBlocks;++i)
			{
				auto *memBlock = res->GetMemoryBlock(i);
				if(memBlock == nullptr)
					continue;
				// TODO
				//size += memBlock->get_create_info_ptr()->get_size();
			}
			resourceSizes.push_back(size);
		}
		ss<<"\t"<<pair.second<<": "<<pragma::util::get_pretty_bytes(allocatedSizeOfType)<<" ("<<pragma::math::round(allocatedSizeOfType /static_cast<double>(allocatedSize) *100.0,2)<<"%)\n";
		ss<<"\tNumber of Resources: "<<resources.size()<<"\n";
		std::sort(sortedIndices.begin(),sortedIndices.end(),[&resourceSizes](const size_t idx0,const size_t idx1) {
			return resourceSizes.at(idx0) > resourceSizes.at(idx1);
		});
		for(auto idx : sortedIndices)
		{
			auto res = resources.at(idx);
			auto size = resourceSizes.at(idx);
			ss<<"\t\t";
			if((res->typeFlags &prosper::MemoryTracker::Resource::TypeFlags::StandAloneBufferBit) != prosper::MemoryTracker::Resource::TypeFlags::None)
				ss<<"StandAloneBuffer: "<<static_cast<prosper::IBuffer*>(res->resource)->GetDebugName();
			else if((res->typeFlags &prosper::MemoryTracker::Resource::TypeFlags::UniformBufferBit) != prosper::MemoryTracker::Resource::TypeFlags::None)
				ss<<"UniformResizableBuffer: "<<static_cast<prosper::IUniformResizableBuffer*>(res->resource)->GetDebugName();
			else if((res->typeFlags &prosper::MemoryTracker::Resource::TypeFlags::DynamicBufferBit) != prosper::MemoryTracker::Resource::TypeFlags::None)
				ss<<"DynamicResizableBuffer: "<<static_cast<prosper::IDynamicResizableBuffer*>(res->resource)->GetDebugName();
			else if((res->typeFlags &prosper::MemoryTracker::Resource::TypeFlags::ImageBit) != prosper::MemoryTracker::Resource::TypeFlags::None)
				ss<<"Image: "<<static_cast<prosper::IImage*>(res->resource)->GetDebugName();
			ss<<" "<<pragma::util::get_pretty_bytes(size)<<" ("<<pragma::math::round(size /static_cast<double>(availableSize) *100.0,2)<<"%)\n";
		}
		ss<<"\n";
	}

	auto &resources = prosper::MemoryTracker::GetInstance().GetResources();
	std::vector<size_t> sortedResourceIndices = {};
	sortedResourceIndices.reserve(resources.size());
	for(auto i=decltype(sortedResourceIndices.size()){0u};i<resources.size();++i)
	{
		if((resources.at(i).typeFlags &prosper::MemoryTracker::Resource::TypeFlags::BufferBit) == prosper::MemoryTracker::Resource::TypeFlags::None)
			continue;
		sortedResourceIndices.push_back(i);
	}
	std::sort(sortedResourceIndices.begin(),sortedResourceIndices.end(),[&resources](size_t idx0,size_t idx1) {
		return static_cast<prosper::IBuffer*>(resources.at(idx0).resource)->GetSize() > static_cast<prosper::IBuffer*>(resources.at(idx1).resource)->GetSize();
	});

	ss<<"Uniform resizable buffers:\n";
	for(auto idx : sortedResourceIndices)
	{
		auto &res = resources.at(idx);
		if((res.typeFlags &prosper::MemoryTracker::Resource::TypeFlags::UniformBufferBit) != prosper::MemoryTracker::Resource::TypeFlags::None)
		{
			auto &uniBuf = *static_cast<prosper::IUniformResizableBuffer*>(res.resource);
			ss<<uniBuf.GetDebugName()<<"\n";
			auto &allocatedSubBuffers = uniBuf.GetAllocatedSubBuffers();
			auto instanceCount = std::count_if(allocatedSubBuffers.begin(),allocatedSubBuffers.end(),[](const prosper::IBuffer *buffer) {
				return buffer != nullptr;
			});
			auto assignedMemory = uniBuf.GetAssignedMemory();
			auto totalInstanceCount = uniBuf.GetTotalInstanceCount();
			ss<<"\tInstances in use: "<<instanceCount<<" / "<<totalInstanceCount<<" ("<<pragma::math::round(instanceCount /static_cast<double>(totalInstanceCount) *100.0,2)<<"%)\n";
			ss<<"\tMemory in use: "<<pragma::util::get_pretty_bytes(assignedMemory)<<" / "<<pragma::util::get_pretty_bytes(uniBuf.GetSize())<<" ("<<pragma::math::round(assignedMemory /static_cast<double>(uniBuf.GetSize()) *100.0,2)<<"%)\n";
		}
	}

	ss<<"\nDynamic resizable buffers:\n";
	for(auto &res : prosper::MemoryTracker::GetInstance().GetResources())
	{
		if((res.typeFlags &prosper::MemoryTracker::Resource::TypeFlags::DynamicBufferBit) != prosper::MemoryTracker::Resource::TypeFlags::None)
		{
			auto &dynBuf = *static_cast<prosper::IDynamicResizableBuffer*>(res.resource);
			ss<<dynBuf.GetDebugName()<<"\n";
			auto &allocatedSubBuffers = dynBuf.GetAllocatedSubBuffers();
			auto instanceCount = std::count_if(allocatedSubBuffers.begin(),allocatedSubBuffers.end(),[](const prosper::IBuffer *buffer) {
				return buffer != nullptr;
			});
			auto szFree = dynBuf.GetFreeSize();
			auto assignedMemory = dynBuf.GetSize() -szFree;
			ss<<"\tInstances: "<<instanceCount<<"\n";
			ss<<"\tMemory in use: "<<pragma::util::get_pretty_bytes(assignedMemory)<<" / "<<pragma::util::get_pretty_bytes(dynBuf.GetSize())<<" ("<<pragma::math::round(assignedMemory /static_cast<double>(dynBuf.GetSize()) *100.0,2)<<"%)\n";
			ss<<"\tFragmentation: "<<dynBuf.GetFragmentationPercent()<<"\n";
		}
	}
	std::cout<<ss.str()<<std::endl;

	/*auto &context = pragma::get_cengine()->GetRenderContext();
	auto &memoryMan = context.GetMemoryManager();
	auto stats = memoryMan.GetStatistics();
	if(argv.empty())
	{
		Con::COUT<<"Memory usage:"<<Con::endl;
		std::stringstream ss;
		print_memory_stats(ss,stats.total);
		Con::COUT<<ss.str();
		Con::flush();
	}
	else
	{
		if(argv.size() == 1)
		{
			Con::CWAR<<"Not enough arguments given!"<<Con::endl;
			return;
		}
		auto id = pragma::util::to_int(argv.at(1));
		if(argv.front() == "heap")
		{
			if(id >= stats.memoryHeap.size())
			{
				Con::CWAR<<"Second argument has to be in the range [0,"<<stats.memoryHeap.size()<<"]!"<<Con::endl;
				return;
			}
			Con::COUT<<"Memory usage for heap "<<id<<":"<<Con::endl;
			std::stringstream ss;
			print_memory_stats(ss,stats.memoryHeap.at(id));
			Con::COUT<<ss.str();
			Con::flush();
		}
		else if(argv.front() == "type")
		{
			if(id >= stats.memoryType.size())
			{
				Con::CWAR<<"Second argument has to be in the range [0,"<<stats.memoryType.size()<<"]!"<<Con::endl;
				return;
			}
			Con::COUT<<"Memory usage for type "<<id<<":"<<Con::endl;
			std::stringstream ss;
			print_memory_stats(ss,stats.memoryType.at(id));
			Con::COUT<<ss.str();
			Con::flush();
		}
	}*/ // prosper TODO
#endif
}

static void cvar_net_graph(bool val)
{
	static std::unique_ptr<DebugGameGUI> dbg = nullptr;
	if(dbg == nullptr) {
		if(val == false)
			return;
		dbg = std::make_unique<DebugGameGUI>([]() {
			auto &wgui = pragma::gui::WGUI::GetInstance();
			auto sz = wgui.GetContext().GetWindow()->GetSize();
			auto el = wgui.Create<pragma::gui::types::WINetGraph>();
			el->SetSize(540, 180);
			el->SetPos(sz.x - el->GetWidth(), 0);
			return el->GetHandle();
		});
		return;
	}
	else if(val == true)
		return;
	dbg = nullptr;
}
namespace {
	auto _ = pragma::console::client::register_variable_listener<bool>("net_graph", +[](pragma::NetworkState *nw, const pragma::console::ConVar &cv, bool oldVal, bool newVal) -> void { cvar_net_graph(newVal); });
}
