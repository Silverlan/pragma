// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.server;
import :game;

import :ai;
import :entities;
import :entities.components;
import :server_state;
import :scripting.lua;

#undef LoadLibrary

void SGame::RegisterLua()
{
	GetLuaInterface().SetIdentifier("sv");

	lua::push_boolean(GetLuaState(), 1);
	Lua::SetGlobal(GetLuaState(), "SERVER");

	lua::push_boolean(GetLuaState(), 0);
	Lua::SetGlobal(GetLuaState(), "CLIENT");

	auto engineMod = luabind::module(GetLuaState(), "engine");
	engineMod[(luabind::def("load_library", Lua::engine::LoadLibrary), luabind::def("unload_library", Lua::engine::UnloadLibrary), luabind::def("is_library_loaded", Lua::engine::IsLibraryLoaded), luabind::def("library_exists", Lua::engine::LibraryExists),
	  luabind::def("get_info", Lua::engine::get_info), luabind::def("poll_console_output", Lua::engine::poll_console_output), luabind::def("get_user_data_dir", util::get_user_data_dir), luabind::def("get_resource_dirs", util::get_resource_dirs))];
	Lua::engine::register_shared_functions(GetLuaState(), engineMod);

	Lua::RegisterLibrary(GetLuaState(), "game",
	  {//{"create_light",Lua::engine::CreateLight},
	    //{"remove_lights",Lua::engine::RemoveLights},
	    //{"create_sprite",Lua::engine::CreateSprite},
	    {"create_model", Lua::game::Server::create_model}, {"load_map", Lua::game::Server::load_map}});
	auto gameMod = luabind::module(GetLuaState(), "game");
	Lua::game::register_shared_functions(GetLuaState(), gameMod);
	gameMod[(luabind::def("change_map", static_cast<void (*)(const std::string &, const std::string &)>(Lua::game::Server::change_level)), luabind::def("change_map", static_cast<void (*)(const std::string &)>(Lua::game::Server::change_level)),
	  luabind::def("set_gravity", Lua::game::Server::set_gravity), luabind::def("get_gravity", Lua::game::Server::get_gravity), luabind::def("load_model", Lua::game::Server::load_model),
	  luabind::def("load_sound_scripts", static_cast<void (*)(lua::State *, const std::string &, bool)>(Lua::engine::LoadSoundScripts)), luabind::def("load_sound_scripts", static_cast<void (*)(lua::State *, const std::string &)>(Lua::engine::LoadSoundScripts)),
	  luabind::def("precache_model", Lua::engine::PrecacheModel_sv), luabind::def("get_model", Lua::engine::get_model), luabind::def("load_material", static_cast<msys::Material *(*)(const std::string &, bool)>(Lua::engine::server::LoadMaterial)),
	  luabind::def("load_material", static_cast<msys::Material *(*)(const std::string &)>(Lua::engine::server::LoadMaterial)), luabind::def("set_time_scale", &Lua::game::set_time_scale))];

	Lua::ents::register_library(GetLuaState());
	auto entsMod = luabind::module(GetLuaState(), "ents");
	entsMod[luabind::def("register_component_event", &Lua::ents::register_component_event)];
	auto &modEnts = GetLuaInterface().RegisterLibrary("ents", {{"register_component", Lua::ents::register_component<pragma::SLuaBaseEntityComponent>}});

	Lua::RegisterLibraryEnums(GetLuaState(), "ents", {{"COMPONENT_FLAG_NONE", umath::to_integral(pragma::ComponentFlags::None)}, {"COMPONENT_FLAG_BIT_NETWORKED", umath::to_integral(pragma::ComponentFlags::Networked)}});

	auto entityClassDef = luabind::class_<pragma::ecs::BaseEntity>("BaseEntityBase");
	Lua::Entity::register_class(entityClassDef);
	modEnts[entityClassDef];
	RegisterLuaGameClasses(gameMod);

	auto sEntityClassDef = luabind::class_<SBaseEntity, pragma::ecs::BaseEntity>("Entity");
	Lua::Entity::Server::register_class(sEntityClassDef);
	modEnts[sEntityClassDef];

	//auto tmp = luabind::class_<EntityHandle>("EntityOld");
	//modEnts[tmp];

	// Obsolete?
	auto classDefBase = luabind::class_<SLuaEntity, luabind::bases<SBaseEntity>, pragma::LuaCore::HandleHolder<SLuaEntity>>("BaseEntity");
	classDefBase.def(luabind::constructor<>());
	// classDefBase.def(luabind::tostring(luabind::self));
	//classDefBase.def(luabind::constructor<>());
	classDefBase.def("Initialize", &SLuaEntity::LuaInitialize, &SLuaEntity::default_Initialize);
	//classDefBase.def("ReceiveNetEvent",&SLuaEntityWrapper::ReceiveNetEvent,&SLuaBaseEntityWrapper::default_ReceiveNetEvent);
	modEnts[classDefBase];
	//

	// Needs to be registered AFTER RegisterLuaGameClasses has been called!
	Lua::register_base_entity_component(modEnts);
	auto defEntCmp = pragma::LuaCore::create_entity_component_class<pragma::SLuaBaseEntityComponent, luabind::bases<pragma::BaseLuaBaseEntityComponent, pragma::BaseEntityComponent>, pragma::LuaCore::SLuaBaseEntityComponentHolder>("BaseEntityComponent");
	defEntCmp.def(luabind::constructor<SBaseEntity &>());
	defEntCmp.def("SendData", static_cast<void (*)(lua::State *, pragma::SLuaBaseEntityComponent &, NetPacket, pragma::networking::ClientRecipientFilter &)>([](lua::State *l, pragma::SLuaBaseEntityComponent &hComponent, NetPacket packet, pragma::networking::ClientRecipientFilter &rp) {

	}));
	defEntCmp.def("ReceiveNetEvent", static_cast<void (*)(lua::State *, pragma::SLuaBaseEntityComponent &, pragma::SPlayerComponent &, uint32_t, NetPacket)>([](lua::State *l, pragma::SLuaBaseEntityComponent &hComponent, pragma::SPlayerComponent &pl, uint32_t evId, NetPacket packet) {

	}));
	defEntCmp.def("SendSnapshotData", static_cast<void (*)(lua::State *, pragma::SLuaBaseEntityComponent &, NetPacket, pragma::SPlayerComponent &)>([](lua::State *l, pragma::SLuaBaseEntityComponent &hComponent, NetPacket packet, pragma::SPlayerComponent &pl) {

	}));
	defEntCmp.def("SendNetEvent",
	  static_cast<void (*)(lua::State *, pragma::SLuaBaseEntityComponent &, uint32_t, uint32_t, NetPacket &, pragma::networking::TargetRecipientFilter &)>(
	    [](lua::State *l, pragma::SLuaBaseEntityComponent &hComponent, uint32_t protocol, uint32_t eventId, NetPacket &packet, pragma::networking::TargetRecipientFilter &rf) {
		    static_cast<SBaseEntity &>(hComponent.GetEntity()).SendNetEvent(eventId, packet, static_cast<pragma::networking::Protocol>(protocol), rf);
	    }));
	defEntCmp.def("SendNetEvent", static_cast<void (*)(lua::State *, pragma::SLuaBaseEntityComponent &, uint32_t, uint32_t, NetPacket &)>([](lua::State *l, pragma::SLuaBaseEntityComponent &hComponent, uint32_t protocol, uint32_t eventId, NetPacket &packet) {
		static_cast<SBaseEntity &>(hComponent.GetEntity()).SendNetEvent(eventId, packet, static_cast<pragma::networking::Protocol>(protocol));
	}));
	defEntCmp.def("SendNetEvent", static_cast<void (*)(lua::State *, pragma::SLuaBaseEntityComponent &, uint32_t, uint32_t)>([](lua::State *l, pragma::SLuaBaseEntityComponent &hComponent, uint32_t protocol, uint32_t eventId) {
		static_cast<SBaseEntity &>(hComponent.GetEntity()).SendNetEvent(eventId, static_cast<pragma::networking::Protocol>(protocol));
	}));
	defEntCmp.def("SendNetEvent",
	  static_cast<void (*)(lua::State *, pragma::SLuaBaseEntityComponent &, uint32_t, NetPacket &)>([](lua::State *l, pragma::SLuaBaseEntityComponent &hComponent, uint32_t eventId, NetPacket &packet) { static_cast<SBaseEntity &>(hComponent.GetEntity()).SendNetEvent(eventId, packet); }));
	register_shared_lua_component_methods<pragma::SLuaBaseEntityComponent>(defEntCmp);
	modEnts[defEntCmp];

	auto modNet = luabind::module(GetLuaState(), "net");
	modNet[(luabind::def("broadcast", &Lua::net::server::broadcast), luabind::def("send", static_cast<void (*)(lua::State *, pragma::networking::Protocol, const std::string &, NetPacket &, const luabind::tableT<pragma::SPlayerComponent> &)>(&Lua::net::server::send)),
	  luabind::def("send", static_cast<void (*)(lua::State *, pragma::networking::Protocol, const std::string &, NetPacket &, pragma::networking::TargetRecipientFilter &)>(&Lua::net::server::send)),
	  luabind::def("send", static_cast<void (*)(lua::State *, pragma::networking::Protocol, const std::string &, NetPacket &, pragma::SPlayerComponent &)>(&Lua::net::server::send)), luabind::def("receive", &Lua::net::server::receive),
	  luabind::def("register", &Lua::net::server::register_net_message), luabind::def("register_event", &Lua::net::register_event))];
	auto netPacketClassDef = luabind::class_<NetPacket>("Packet");
	Lua::NetPacket::Server::register_class(netPacketClassDef);
	netPacketClassDef.def("WritePlayer", static_cast<void (*)(lua::State *, ::NetPacket &, util::WeakHandle<pragma::SPlayerComponent> &)>([](lua::State *l, ::NetPacket &packet, util::WeakHandle<pragma::SPlayerComponent> &pl) { nwm::write_player(packet, pl.get()); }));
	netPacketClassDef.def("WritePlayer", static_cast<void (*)(lua::State *, ::NetPacket &, EntityHandle &)>([](lua::State *l, ::NetPacket &packet, EntityHandle &hEnt) { nwm::write_player(packet, hEnt.get()); }));
	netPacketClassDef.def("ReadPlayer", static_cast<void (*)(lua::State *, ::NetPacket &)>([](lua::State *l, ::NetPacket &packet) {
		auto *pl = static_cast<pragma::SPlayerComponent *>(nwm::read_player(packet));
		if(pl == nullptr)
			return;
		pl->PushLuaObject(l);
	}));
	modNet[netPacketClassDef];
	Lua::net::RegisterLibraryEnums(GetLuaState());

	auto classDefRp = luabind::class_<pragma::networking::TargetRecipientFilter>("RecipientFilter");
	classDefRp.def(luabind::constructor<>());
	classDefRp.def("GetRecipients", &Lua::RecipientFilter::GetRecipients);
	classDefRp.def("AddRecipient", &Lua::RecipientFilter::AddRecipient);
	classDefRp.def("RemoveRecipient", &Lua::RecipientFilter::RemoveRecipient);
	classDefRp.def("HasRecipient", &Lua::RecipientFilter::HasRecipient);
	classDefRp.def("GetFilterType", static_cast<void (*)(lua::State *, pragma::networking::TargetRecipientFilter &)>([](lua::State *l, pragma::networking::TargetRecipientFilter &rp) { Lua::PushInt(l, umath::to_integral(rp.GetFilterType())); }));
	classDefRp.def("SetFilterType",
	  static_cast<void (*)(lua::State *, pragma::networking::TargetRecipientFilter &, uint32_t)>([](lua::State *l, pragma::networking::TargetRecipientFilter &rp, uint32_t filterType) { rp.SetFilterType(static_cast<pragma::networking::ClientRecipientFilter::FilterType>(filterType)); }));
	classDefRp.add_static_constant("TYPE_INCLUDE", umath::to_integral(pragma::networking::ClientRecipientFilter::FilterType::Include));
	classDefRp.add_static_constant("TYPE_EXCLUDE", umath::to_integral(pragma::networking::ClientRecipientFilter::FilterType::Exclude));
	modNet[classDefRp];

	auto classDefClRp = luabind::class_<pragma::networking::ClientRecipientFilter>("ClientRecipientFilter");
	classDefClRp.def("GetRecipients", static_cast<void (*)(lua::State *, pragma::networking::ClientRecipientFilter &)>([](lua::State *l, pragma::networking::ClientRecipientFilter &rp) {
		auto *sv = ServerState::Get()->GetServer();
		if(sv == nullptr)
			return;
		auto t = Lua::CreateTable(l);
		auto idx = 1;
		for(auto &cl : sv->GetClients()) {
			if(rp(*cl) == false)
				continue;
			auto *pl = ServerState::Get()->GetPlayer(*cl);
			if(pl == nullptr)
				continue;
			Lua::PushInt(l, idx++);
			pl->PushLuaObject(l);
			Lua::SetTableValue(l, t);
		}
	}));
	modNet[classDefClRp];

	Lua::RegisterLibrary(GetLuaState(), "input", {});
	pragma::Game::RegisterLua();
}

void SGame::InitializeLua()
{
	pragma::Game::InitializeLua();
	CallCallbacks<void, lua::State *>("OnLuaInitialized", GetLuaState());
}

void SGame::SetupLua()
{
	pragma::Game::SetupLua();
	RunLuaFiles("autorun\\");
	RunLuaFiles("autorun\\server\\");
	SetGameMode(GetConVarString("sv_gamemode"));
	InitializeGameMode();
	//for(auto &subDir : GetLuaEntityDirectories())
	//	LoadLuaEntities(subDir);
}

bool SGame::LoadLuaComponent(const std::string &luaFilePath, const std::string &mainPath, const std::string &componentName)
{
	auto r = pragma::Game::LoadLuaComponent(luaFilePath, mainPath, componentName);
	if(r == false)
		return r;
	auto nComponentName = FileManager::GetCanonicalizedPath(componentName);
	auto componentPath = Lua::SCRIPT_DIRECTORY_SLASH + mainPath + "\\components\\" + nComponentName;
	auto filePathLuaFile = componentPath + "\\init" + Lua::DOT_FILE_EXTENSION;
	if(FileManager::Exists(filePathLuaFile))
		return r;                                                                                               // init.lua is in main component directory, which means no network directories are used. In this case files that need to be transferred cannot be determined automatically.
	std::vector<std::string> transferFiles;                                                                     // Files which need to be transferred to the client
	FileManager::FindFiles((componentPath + "\\*" + Lua::DOT_FILE_EXTENSION).c_str(), &transferFiles, nullptr); // Shared Files
	if(Lua::are_precompiled_files_enabled())
		FileManager::FindFiles((componentPath + "\\*" + Lua::DOT_FILE_EXTENSION_PRECOMPILED).c_str(), &transferFiles, nullptr);
	for(auto &fName : transferFiles)
		fName = componentPath + '\\' + fName;

	auto componentPathClient = componentPath + "\\client";
	auto offset = transferFiles.size();
	FileManager::FindFiles((componentPathClient + "\\*" + Lua::DOT_FILE_EXTENSION).c_str(), &transferFiles, nullptr); // Clientside Files
	if(Lua::are_precompiled_files_enabled())
		FileManager::FindFiles((componentPathClient + "\\*" + Lua::DOT_FILE_EXTENSION_PRECOMPILED).c_str(), &transferFiles, nullptr);
	for(auto i = offset; i < transferFiles.size(); ++i)
		transferFiles.at(i) = componentPathClient + '\\' + transferFiles.at(i);

	for(auto &fname : transferFiles)
		ResourceManager::AddResource(fname);
	return r;
}
std::string SGame::GetLuaNetworkDirectoryName() const { return "server"; }
std::string SGame::GetLuaNetworkFileName() const { return "init" + Lua::DOT_FILE_EXTENSION; }
