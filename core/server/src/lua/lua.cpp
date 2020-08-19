/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#include "stdafx_server.h"
#include "pragma/game/s_game.h"
#include "pragma/lua/libraries/lengine.h"
#include "pragma/lua/libraries/s_lents.h"
#include "pragma/lua/libraries/s_lnetmessages.h"
#include "pragma/lua/libraries/s_lengine.h"
#include "pragma/lua/libraries/lglobal.h"
#include "pragma/lua/libraries/s_lgame.h"
#include "pragma/ai/ai_task.h"
#include "pragma/lua/libraries/lray.h"
#include "pragma/lua/libraries/lgame.h"
#include "pragma/ai/ai_behavior.h"
#include "pragma/networking/resourcemanager.h"
#include "pragma/networking/recipient_filter.hpp"
#include "pragma/networking/iserver.hpp"
#include "pragma/lua/classes/ldef_recipientfilter.h"
#include "pragma/lua/classes/lrecipientfilter.h"
#include "pragma/lua/classes/s_lnetpacket.h"
#include "pragma/lua/s_lentity_components.hpp"
#include "pragma/entities/components/s_player_component.hpp"
#include "pragma/lua/s_lua_component.hpp"
#include "pragma/lua/classes/s_lentity.h"
#include "pragma/lua/classes/s_lua_entity.h"
#include "pragma/lua/sh_lua_component_t.hpp"
#include <pragma/lua/lua_component_event.hpp>
#include <pragma/lua/lua_entity_component.hpp>
#include <pragma/lua/sh_lua_component_wrapper.hpp>
#include <pragma/lua/lentity_component_lua.hpp>
#include <pragma/lua/sh_lua_component.hpp>
#include <pragma/lua/libraries/lnet.hpp>
#include <pragma/model/model.h>
#include <pragma/networking/nwm_util.h>
#include <luainterface.hpp>

extern DLLSERVER ServerState *server;

void SGame::RegisterLua()
{
	GetLuaInterface().SetIdentifier("sv");

	lua_pushboolean(GetLuaState(),1);
	lua_setglobal(GetLuaState(),"SERVER");

	lua_pushboolean(GetLuaState(),0);
	lua_setglobal(GetLuaState(),"CLIENT");

	Lua::RegisterLibrary(GetLuaState(),"engine",{
		{"load_library",&Lua::engine::LoadLibrary},
		{"get_info",&Lua::engine::get_info},
		{"poll_console_output",&Lua::engine::poll_console_output}
	});
	auto engineMod = luabind::module(GetLuaState(),"engine");
	engineMod[
		luabind::def("shutdown",Lua::engine::exit),
		luabind::def("get_working_directory",Lua::engine::get_working_directory),
		luabind::def("set_record_console_output",Lua::engine::set_record_console_output),
		luabind::def("get_tick_count",Lua::engine::GetTickCount)
	];
	
	Lua::RegisterLibrary(GetLuaState(),"game",{
		LUA_LIB_GAME_SHARED
		//{"create_light",Lua::engine::CreateLight},
		//{"remove_lights",Lua::engine::RemoveLights},
		//{"create_sprite",Lua::engine::CreateSprite},
		{"create_model",Lua::game::Server::create_model},
		{"set_time_scale",Lua::game::set_time_scale},
		{"load_map",Lua::game::Server::load_map}
	});
	auto gameMod = luabind::module(GetLuaState(),"game");
	gameMod[
		luabind::def("change_map",static_cast<void(*)(const std::string&,const std::string&)>(Lua::game::Server::change_level)),
		luabind::def("change_map",static_cast<void(*)(const std::string&)>(Lua::game::Server::change_level)),
		luabind::def("set_gravity",Lua::game::Server::set_gravity),
		luabind::def("get_gravity",Lua::game::Server::get_gravity),
		luabind::def("load_model",Lua::game::Server::load_model),
		luabind::def("load_sound_scripts",static_cast<void(*)(lua_State*,const std::string&,bool)>(Lua::engine::LoadSoundScripts)),
		luabind::def("load_sound_scripts",static_cast<void(*)(lua_State*,const std::string&)>(Lua::engine::LoadSoundScripts)),
		luabind::def("precache_model",Lua::engine::PrecacheModel_sv),
		luabind::def("get_model",Lua::engine::get_model),
		luabind::def("load_material",static_cast<Material*(*)(const std::string&,bool)>(Lua::engine::server::LoadMaterial)),
		luabind::def("load_material",static_cast<Material*(*)(const std::string&)>(Lua::engine::server::LoadMaterial))
	];
	RegisterLuaGameClasses(gameMod);

	Lua::ents::register_library(GetLuaState());
	auto &modEnts = GetLuaInterface().RegisterLibrary("ents",{
		LUA_LIB_ENTS_SHARED
		{"register_component",Lua::ents::register_component<pragma::SLuaBaseEntityComponent>},
		{"register_component_event",Lua::ents::register_component_event}
	});

	Lua::RegisterLibraryEnums(GetLuaState(),"ents",{
		{"COMPONENT_FLAG_NONE",umath::to_integral(pragma::ComponentFlags::None)},
		{"COMPONENT_FLAG_BIT_NETWORKED",umath::to_integral(pragma::ComponentFlags::Networked)}
	});

	auto entityClassDef = luabind::class_<EntityHandle>("Entity");
	Lua::Entity::Server::register_class(entityClassDef);
	modEnts[entityClassDef];

	// Obsolete?
	auto classDefBase = luabind::class_<SLuaEntityHandle,luabind::bases<EntityHandle>,luabind::default_holder,SLuaEntityWrapper>("BaseEntity");
	classDefBase.def(luabind::tostring(luabind::self));
	classDefBase.def(luabind::constructor<>());
	classDefBase.def("Initialize",&SLuaEntityWrapper::Initialize,&SLuaEntityWrapper::default_Initialize);
	//classDefBase.def("ReceiveNetEvent",&SLuaEntityWrapper::ReceiveNetEvent,&SLuaBaseEntityWrapper::default_ReceiveNetEvent);
	modEnts[classDefBase];
	//

	// Needs to be registered AFTER RegisterLuaGameClasses has been called!
	auto defEntCmp = luabind::class_<BaseLuaBaseEntityHandle,luabind::bases<BaseEntityComponentHandle>,luabind::default_holder,LuaBaseEntityComponentWrapper>("BaseEntityComponent");
	Lua::register_base_entity_component<luabind::class_<BaseLuaBaseEntityHandle,luabind::bases<BaseEntityComponentHandle>,luabind::default_holder,LuaBaseEntityComponentWrapper>>(defEntCmp);
	defEntCmp.def("SendData",static_cast<void(*)(lua_State*,BaseLuaBaseEntityHandle&,NetPacket,pragma::networking::ClientRecipientFilter&)>([](lua_State *l,BaseLuaBaseEntityHandle &hComponent,NetPacket packet,pragma::networking::ClientRecipientFilter &rp) {
		
	}));
	defEntCmp.def("ReceiveNetEvent",static_cast<void(*)(lua_State*,BaseLuaBaseEntityHandle&,SPlayerHandle&,uint32_t,NetPacket)>([](lua_State *l,BaseLuaBaseEntityHandle &hComponent,SPlayerHandle &pl,uint32_t evId,NetPacket packet) {
		
	}));
	defEntCmp.def("SendSnapshotData",static_cast<void(*)(lua_State*,BaseLuaBaseEntityHandle&,NetPacket,SPlayerHandle&)>([](lua_State *l,BaseLuaBaseEntityHandle &hComponent,NetPacket packet,SPlayerHandle &pl) {
		
	}));
	modEnts[defEntCmp];

	Lua::RegisterLibrary(GetLuaState(),"net",{
		{"broadcast",Lua_sv_net_Broadcast},
		{"send",Lua_sv_net_Send},
		{"receive",Lua_sv_net_Receive},
		{"register",Lua_sv_net_Register},
		{"register_event",Lua::net::register_event}
		//{"send_resource_file",Lua_sv_net_SendResourceFile}
	});
	
	auto modNet = luabind::module(GetLuaState(),"net");
	auto netPacketClassDef = luabind::class_<NetPacket>("Packet");
	Lua::NetPacket::Server::register_class(netPacketClassDef);
	netPacketClassDef.def("WritePlayer",static_cast<void(*)(lua_State*,::NetPacket&,util::WeakHandle<pragma::SPlayerComponent>&)>([](lua_State *l,::NetPacket &packet,util::WeakHandle<pragma::SPlayerComponent> &pl) {
		nwm::write_player(packet,pl.get());
	}));
	netPacketClassDef.def("WritePlayer",static_cast<void(*)(lua_State*,::NetPacket&,EntityHandle&)>([](lua_State *l,::NetPacket &packet,EntityHandle &hEnt) {
		nwm::write_player(packet,hEnt.get());
	}));
	netPacketClassDef.def("ReadPlayer",static_cast<void(*)(lua_State*,::NetPacket&)>([](lua_State *l,::NetPacket &packet) {
		auto *pl = static_cast<pragma::SPlayerComponent*>(nwm::read_player(packet));
		if(pl == nullptr)
			return;
		pl->PushLuaObject(l);
	}));
	modNet[netPacketClassDef];
	Lua::net::RegisterLibraryEnums(GetLuaState());

	auto classDefRp = luabind::class_<pragma::networking::TargetRecipientFilter>("RecipientFilter");
	classDefRp.def(luabind::constructor<>());
	classDefRp.def("GetRecipients",&Lua::RecipientFilter::GetRecipients);
	classDefRp.def("AddRecipient",&Lua::RecipientFilter::AddRecipient);
	classDefRp.def("RemoveRecipient",&Lua::RecipientFilter::RemoveRecipient);
	classDefRp.def("HasRecipient",&Lua::RecipientFilter::HasRecipient);
	classDefRp.def("GetFilterType",static_cast<void(*)(lua_State*,pragma::networking::TargetRecipientFilter&)>([](lua_State *l,pragma::networking::TargetRecipientFilter &rp) {
		Lua::PushInt(l,umath::to_integral(rp.GetFilterType()));
	}));
	classDefRp.def("SetFilterType",static_cast<void(*)(lua_State*,pragma::networking::TargetRecipientFilter&,uint32_t)>([](lua_State *l,pragma::networking::TargetRecipientFilter &rp,uint32_t filterType) {
		rp.SetFilterType(static_cast<pragma::networking::ClientRecipientFilter::FilterType>(filterType));
	}));
	classDefRp.add_static_constant("TYPE_INCLUDE",umath::to_integral(pragma::networking::ClientRecipientFilter::FilterType::Include));
	classDefRp.add_static_constant("TYPE_EXCLUDE",umath::to_integral(pragma::networking::ClientRecipientFilter::FilterType::Exclude));
	modNet[classDefRp];

	auto classDefClRp = luabind::class_<pragma::networking::ClientRecipientFilter>("ClientRecipientFilter");
	classDefClRp.def("GetRecipients",static_cast<void(*)(lua_State*,pragma::networking::ClientRecipientFilter&)>([](lua_State *l,pragma::networking::ClientRecipientFilter &rp) {
		auto *sv = server->GetServer();
		if(sv == nullptr)
			return;
		auto t = Lua::CreateTable(l);
		auto idx = 1;
		for(auto &cl : sv->GetClients())
		{
			if(rp(*cl) == false)
				continue;
			auto *pl = server->GetPlayer(*cl);
			if(pl == nullptr)
				continue;
			Lua::PushInt(l,idx++);
			pl->PushLuaObject(l);
			Lua::SetTableValue(l,t);
		}
	}));
	modNet[classDefClRp];

	Lua::RegisterLibrary(GetLuaState(),"input",{});
	Game::RegisterLua();
}

void SGame::InitializeLua()
{
	Game::InitializeLua();
	CallCallbacks<void,lua_State*>("OnLuaInitialized",GetLuaState());
}

void SGame::SetupLua()
{
	Game::SetupLua();
	RunLuaFiles("autorun\\");
	RunLuaFiles("autorun\\server\\");
	SetGameMode(GetConVarString("sv_gamemode"));
	InitializeGameMode();
	//for(auto &subDir : GetLuaEntityDirectories())
	//	LoadLuaEntities(subDir);
}

bool SGame::LoadLuaComponent(const std::string &luaFilePath,const std::string &mainPath,const std::string &componentName)
{
	auto r = Game::LoadLuaComponent(luaFilePath,mainPath,componentName);
	if(r == false)
		return r;
	auto nComponentName = FileManager::GetCanonicalizedPath(componentName);
	auto componentPath = "lua\\" +mainPath +"\\components\\" +nComponentName;
	auto filePathLuaFile = componentPath +"\\init.lua";
	if(FileManager::Exists(filePathLuaFile))
		return r; // init.lua is in main component directory, which means no network directories are used. In this case files that need to be transferred cannot be determined automatically.
	std::vector<std::string> transferFiles; // Files which need to be transferred to the client
	FileManager::FindFiles((componentPath +"\\*.lua").c_str(),&transferFiles,nullptr); // Shared Files
	if(Lua::are_precompiled_files_enabled())
		FileManager::FindFiles((componentPath +"\\*.clua").c_str(),&transferFiles,nullptr);
	for(auto &fName : transferFiles)
		fName = componentPath +'\\' +fName;

	auto componentPathClient = componentPath +"\\client";
	auto offset = transferFiles.size();
	FileManager::FindFiles((componentPathClient +"\\*.lua").c_str(),&transferFiles,nullptr); // Clientside Files
	if(Lua::are_precompiled_files_enabled())
		FileManager::FindFiles((componentPathClient +"\\*.clua").c_str(),&transferFiles,nullptr);
	for(auto i=offset;i<transferFiles.size();++i)
		transferFiles.at(i) = componentPathClient +'\\' +transferFiles.at(i);

	for(auto &fname : transferFiles)
		ResourceManager::AddResource(fname);
	return r;
}
std::string SGame::GetLuaNetworkDirectoryName() const {return "server";}
std::string SGame::GetLuaNetworkFileName() const {return "init.lua";}
