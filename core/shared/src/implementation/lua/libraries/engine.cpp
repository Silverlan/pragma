// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :scripting.lua.libraries.engine;

#undef LoadLibrary

void Lua::engine::exit() { pragma::Engine::Get()->ShutDown(); }

std::string Lua::engine::get_working_directory()
{
	auto path = pragma::util::Path::CreatePath(pragma::util::get_program_path());
	return path.GetString();
}

Lua::tb<void> Lua::engine::get_info(lua::State *l)
{
	auto t = luabind::newtable(l);
	t["version"] = pragma::get_engine_version();
	t["prettyVersion"] = pragma::get_pretty_engine_version();
	t["identifier"] = pragma::engine_info::get_identifier();
	t["discordURL"] = pragma::engine_info::get_discord_url();
	t["steamAppId"] = pragma::engine_info::get_steam_app_id();
	t["websiteURL"] = pragma::engine_info::get_website_url();
	t["wikiURL"] = pragma::engine_info::get_wiki_url();
	t["gitHubURL"] = pragma::engine_info::get_github_url();
	t["name"] = pragma::engine_info::get_name();
	return t;
}

Lua::opt<Lua::tb<void>> Lua::engine::get_git_info(lua::State *l)
{
	auto gitInfo = pragma::engine_info::get_git_info();
	if(!gitInfo.has_value())
		return nil;
	auto t = luabind::newtable(l);
	t["ref"] = gitInfo->ref;
	t["commitSha"] = gitInfo->commitSha;
	t["dateTime"] = gitInfo->dateTime;
	return t;
}

void Lua::engine::PrecacheModel_sv(lua::State *l, const std::string &mdlName)
{
	/*auto *nw = ::Engine::Get()->GetNetworkState(l);
	FWMD wmd(nw->GetGameState());
	wmd.Load<Model,ModelMesh,ModelSubMesh>(
		nw->GetGameState(),mdlName,[nw](const std::string &mdlName) -> std::shared_ptr<Model> {
			return nw->GetGameState()->LoadModel(mdlName,false);
		}
	);*/
}

std::shared_ptr<pragma::asset::Model> Lua::engine::get_model(lua::State *l, const std::string &mdlName)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	auto *game = state->GetGameState();
	return game->LoadModel(mdlName);
}

void Lua::engine::LoadSoundScripts(lua::State *l, const std::string &fileName, bool precache)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	state->LoadSoundScripts(fileName.c_str(), precache);
}
void Lua::engine::LoadSoundScripts(lua::State *l, const std::string &fileName) { LoadSoundScripts(l, fileName, false); }

bool Lua::engine::LibraryExists(lua::State *l, const std::string &library)
{
	auto libAbs = pragma::util::get_normalized_module_path(library, pragma::Engine::Get()->GetNetworkState(l)->IsClient());
	return pragma::fs::exists(libAbs);
}

bool Lua::engine::UnloadLibrary(lua::State *l, const std::string &path)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	std::string err;
	return state->UnloadLibrary(path);
}

bool Lua::engine::IsLibraryLoaded(pragma::NetworkState &nw, const std::string &path) { return nw.GetLibraryModule(path) != nullptr; }

Lua::var<bool, std::string> Lua::engine::LoadLibrary(lua::State *l, const std::string &path)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	std::string err;
	bool b = state->InitializeLibrary(path, &err, l) != nullptr;
	if(b)
		return luabind::object {l, true};
	return luabind::object {l, err};
}

uint64_t Lua::engine::GetTickCount() { return pragma::Engine::Get()->GetTickCount(); }

void Lua::engine::set_record_console_output(bool record) { pragma::Engine::Get()->SetRecordConsoleOutput(record); }
Lua::opt<Lua::mult<std::string, pragma::console::MessageFlags, Lua::opt<Color>>> Lua::engine::poll_console_output(lua::State *l)
{
	auto output = pragma::Engine::Get()->PollConsoleOutput();
	if(output.has_value() == false)
		return nil;
	luabind::object color {};
	if(output->color)
		color = {l, *output->color};
	return Lua::mult<std::string, pragma::console::MessageFlags, opt<::Color>> {l, output->output, output->messageFlags, opt<::Color> {color}};
}

void Lua::engine::register_shared_functions(lua::State *l, luabind::module_ &modEn)
{
	modEn[(luabind::def("set_record_console_output", set_record_console_output), luabind::def("get_tick_count", &GetTickCount), luabind::def("shutdown", &exit), luabind::def("get_working_directory", get_working_directory),
	  luabind::def("get_git_info", get_git_info), luabind::def("mount_addon", static_cast<bool (*)(const std::string &)>(&pragma::AddonSystem::MountAddon)),
	  luabind::def(
	    "mount_sub_addon",
	    +[](lua::State *l, const std::string &subAddon) {
		    auto path = pragma::util::Path::CreatePath(util::get_addon_path(l));
		    path.PopFront();
		    path = path + "addons/" + subAddon;
		    return pragma::AddonSystem::MountAddon(path.GetString());
	    }),
	  luabind::def("is_managed_by_package_manager", +[](lua::State *l) { return pragma::get_engine()->IsManagedByPackageManager(); }), luabind::def("is_application_sandboxed", +[](lua::State *l) { return pragma::get_engine()->IsSandboxed(); }))];
}
