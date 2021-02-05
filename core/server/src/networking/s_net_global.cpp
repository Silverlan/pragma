/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#include "stdafx_server.h"
#include "pragma/serverstate/serverutil.h"
#include "pragma/networking/s_net_global.h"
#include "pragma/game/s_game.h"
#include "pragma/networking/resourcemanager.h"
#include "pragma/entities/player.h"
#include "pragma/physics/movetypes.h"
#include <pragma/networking/nwm_util.h>
#include "pragma/networking/iserver.hpp"
#include "pragma/networking/iserver_client.hpp"
#include "pragma/networking/recipient_filter.hpp"
#include <pragma/engine.h>
#include "pragma/ai/ai_schedule.h"
#include "pragma/entities/components/s_player_component.hpp"
#include "pragma/entities/components/s_character_component.hpp"
#include "pragma/entities/components/s_ai_component.hpp"
#include "pragma/networking/master_server.hpp"
#include <pragma/networking/enums.hpp>
#include <pragma/networking/error.hpp>
#include <pragma/debug/debugbehaviortree.h>
#include <pragma/entities/components/base_player_component.hpp>
#include <pragma/entities/components/base_character_component.hpp>
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/entities/components/base_physics_component.hpp>
#include <pragma/entities/components/damageable_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <sharedutils/util_library.hpp>

extern DLLSERVER ServerState *server;
extern DLLSERVER SGame *s_game;
extern DLLNETWORK Engine *engine;
DLLSERVER void NET_sv_disconnect(pragma::networking::IServerClient &session,NetPacket packet)
{
#ifdef DEBUG_SOCKET
	Con::csv<<"Client '"<<session.GetIdentifier()<<"' has disconnected."<<Con::endl;
#endif
	server->DropClient(session);
}

DLLSERVER void NET_sv_userinput(pragma::networking::IServerClient &session,NetPacket packet)
{
	server->ReceiveUserInput(session,packet);
}

DLLSERVER void NET_sv_ent_event(pragma::networking::IServerClient &session,NetPacket packet)
{
	if(!server->IsGameActive())
		return;
	auto *pl = s_game->GetPlayer(session);
	if(pl == nullptr)
		return;
	packet->SetOffset(packet->GetDataSize() -sizeof(UInt32) -sizeof(unsigned int));
	auto *ent = static_cast<SBaseEntity*>(nwm::read_entity(packet));
	if(ent == nullptr)
		return;
	auto eventId = packet->Read<UInt32>();
	packet->SetOffset(0);
	ent->ReceiveNetEvent(*pl,eventId,packet);
}

DLLSERVER void NET_sv_clientinfo(pragma::networking::IServerClient &session,NetPacket packet)
{
	if(!server->IsGameActive())
		return;
	SGame *game = server->GetGameState();
	game->ReceiveUserInfo(session,packet);
}

void NET_sv_game_ready(pragma::networking::IServerClient &session,NetPacket packet)
{
	if(!server->IsGameActive())
		return;
	auto *game = server->GetGameState();
	game->ReceiveGameReady(session,packet);
}

DLLSERVER void NET_sv_cmd_setpos(pragma::networking::IServerClient &session,NetPacket packet)
{
	if(server->CheatsEnabled() == false)
		return;
	if(!server->IsGameActive())
		return;
	auto *pl = s_game->GetPlayer(session);
	if(pl == NULL)
		return;
	auto pTrComponent = pl->GetEntity().GetTransformComponent();
	if(pTrComponent == nullptr)
		return;
	Vector3 pos = nwm::read_vector(packet);
	pTrComponent->SetPosition(pos);
}

DLLSERVER void NET_sv_cmd_call(pragma::networking::IServerClient &session,NetPacket packet)
{
	auto *pl = s_game->GetPlayer(session);
	std::string cmd = packet->ReadString();
	auto pressState = static_cast<KeyState>(packet->Read<uint8_t>());
	auto magnitude = packet->Read<float>();
	unsigned char argc = packet->Read<unsigned char>();
	std::vector<std::string> argv;
	for(unsigned char i=0;i<argc;i++)
		argv.push_back(packet->ReadString());

	auto bActionCmd = (cmd.empty() == false && cmd.front() == '+') ? true : false;
	auto bReleased = (pressState == KeyState::Release) ? true : false;
	auto r = server->RunConsoleCommand(cmd,argv,pl,pressState,magnitude,[bActionCmd,bReleased](ConConf *cf,float &magnitude) -> bool {
		if(bReleased == false || bActionCmd == true)
			return true;
		auto flags = cf->GetFlags();
		if((flags &(ConVarFlags::JoystickAxisContinuous | ConVarFlags::JoystickAxisSingle)) == ConVarFlags::None)
			return false;
		return true;
	});
	if(engine->GetClientState() != nullptr)
		return;
	NetPacket p;
	auto *cv = server->GetConVar(cmd);
	if(r == false || cv == nullptr)
		p->Write<uint8_t>(static_cast<uint8_t>(0));
	else
	{
		if(cv->GetType() == ConType::Var)
		{
			p->Write<uint8_t>(2);
			p->WriteString(static_cast<ConVar*>(cv)->GetString());
		}
		else
		{
			p->Write<uint8_t>(1);
			p->WriteString("");
		}
	}
	server->SendPacket("cmd_call_response",p,pragma::networking::Protocol::SlowReliable,session);
}

DLLSERVER void NET_sv_rcon(pragma::networking::IServerClient &session,NetPacket packet)
{
	if(!server->IsGameActive())
		return;
	std::string passCl = packet->ReadString();
	std::string cvar = packet->ReadString();
	std::string pass = server->GetConVarString("rcon_password");
	if(pass.empty() || pass != passCl)
	{
		/*Game *game = server->GetGameState();
		Player *pl = game->GetPlayer(session);
		if(pl == NULL)
			return;
		pl->PrintMessage("Bad RCON password",MESSAGE::PRINTCONSOLE);*/
		Con::crit<<"Incorrect RCON Password! ("<<passCl<<")"<<Con::endl; // WEAVETODO
		return;
	}
	Con::csv<<"Remote console input from "<<session.GetIdentifier()<<": '"<<cvar<<"'"<<Con::endl;
	engine->ConsoleInput(cvar.c_str());
}

DLLSERVER void NET_sv_serverinfo_request(pragma::networking::IServerClient &session,NetPacket packet)
{
	std::string password = packet->ReadString();
	std::string passSv = server->GetConVarString("sv_password").c_str();
	if(passSv.empty() == false && passSv != password && session.IsListenServerHost() == false)
	{
		NetPacket p;
		server->SendPacket("invalidpassword",p,pragma::networking::Protocol::SlowReliable,session);
		server->DropClient(session);
		return;
	}
	NetPacket p;
	auto *sv = server->GetServer();
	if(sv && sv->GetHostPort().has_value())
	{
		auto port = sv->GetHostPort();
		p->Write<unsigned char>(1);
		p->Write<unsigned short>(*port);
	}
	else
		p->Write<unsigned char>((unsigned char)(0));

	p->Write<bool>(server->IsClientAuthenticationRequired());
	server->SendPacket("serverinfo",p,pragma::networking::Protocol::SlowReliable,session);
}

bool ServerState::IsClientAuthenticationRequired() const
{
	return IsMultiPlayer() && server->GetConVarBool("sv_require_authentication");
}

DLLSERVER void NET_sv_authenticate(pragma::networking::IServerClient &session,NetPacket packet)
{
	auto hasAuth = packet->Read<bool>();
	if(server->IsClientAuthenticationRequired())
	{
		if(hasAuth == false)
		{
			Con::cerr<<"ERROR: Unable to authenticate client '"<<session.GetIdentifier()<<"': Client did not transmit authentication information!"<<Con::endl;
			server->DropClient(session,pragma::networking::DropReason::AuthenticationFailed);
			return;
		}
		auto *reg = server->GetMasterServerRegistration();
		if(reg == nullptr)
		{
			Con::cerr<<"ERROR: Unable to authenticate client '"<<session.GetIdentifier()<<"': Server is not connected to master server!"<<Con::endl;
			server->DropClient(session,pragma::networking::DropReason::AuthenticationFailed);
			return;
		}
		auto steamId = packet->Read<uint64_t>();
		auto lenToken = packet->Read<uint16_t>();
		std::vector<char> token;
		token.resize(lenToken);
		packet->Read(token.data(),token.size() *sizeof(token.front()));

		std::string err;
		auto libSteamworks = server->InitializeLibrary("steamworks/pr_steamworks",&err);
		if(libSteamworks == nullptr)
		{
			Con::cerr<<"ERROR: Unable to authenticate client with steam id '"<<steamId<<"': Steamworks module could not be loaded: "<<err<<Con::endl;
			server->DropClient(session,pragma::networking::DropReason::AuthenticationFailed);
			reg->DropClient(steamId);
			return;
		}
		session.SetSteamId(steamId);
		// Authentication will end with 'OnClientAuthenticated' callback,
		// which will handle the rest
		reg->AuthenticateAndAddClient(steamId,token,"Player");
		return;
	}
	// No authentication required; Continue immediately
	server->OnClientAuthenticated(session,{});
}

DLLSERVER void NET_sv_cvar_set(pragma::networking::IServerClient &session,NetPacket packet)
{
	if(!server->IsGameActive())
		return;
	SGame *game = server->GetGameState();
	auto *pl = game->GetPlayer(session);
	if(pl == NULL)
		return;
	std::string cvar = packet->ReadString();
	std::string val = packet->ReadString();
	std::unordered_map<std::string,std::string> *cvars;
	pl->GetConVars(&cvars);
	(*cvars)[cvar] = val;
	game->OnClientConVarChanged(*pl,cvar,val);
}

DLLSERVER void NET_sv_noclip(pragma::networking::IServerClient &session,NetPacket packet)
{
	if(!server->CheatsEnabled())
		return;
	auto *pl = server->GetPlayer(session);
	if(pl == NULL)
		return;
	auto pPhysComponent = pl->GetEntity().GetPhysicsComponent();
	if(pPhysComponent == nullptr)
		return;
	auto bNoclip = pPhysComponent->GetMoveType() != MOVETYPE::NOCLIP;
	if(bNoclip == false)
	{
		pPhysComponent->SetMoveType(MOVETYPE::WALK);
		pPhysComponent->SetCollisionFilterGroup(CollisionMask::Player);
	}
	else
	{
		pPhysComponent->SetMoveType(MOVETYPE::NOCLIP);
		pPhysComponent->SetCollisionFilterGroup(CollisionMask::NoCollision);
		//pl->SetCollisionsEnabled(false); // Bugged due to CCD
	}
	NetPacket p;
	nwm::write_entity(p,&pl->GetEntity());
	p->Write<bool>(bNoclip);
	server->SendPacket("pl_toggle_noclip",p,pragma::networking::Protocol::SlowReliable);
}

void NET_sv_notarget(pragma::networking::IServerClient &session,NetPacket packet)
{
	if(!server->CheatsEnabled())
		return;
	auto *pl = server->GetPlayer(session);
	if(pl == nullptr)
		return;
	auto *charComponent = static_cast<pragma::SCharacterComponent*>(pl->GetEntity().GetCharacterComponent().get());
	if(charComponent == nullptr)
		return;
	charComponent->SetNoTarget(!charComponent->GetNoTarget());
	pl->PrintMessage(std::string("Notarget turned ") +((charComponent->GetNoTarget() == true) ? "ON" : "OFF"),MESSAGE::PRINTCONSOLE);
}

void NET_sv_godmode(pragma::networking::IServerClient &session,NetPacket packet)
{
	if(!server->CheatsEnabled())
		return;
	auto *pl = server->GetPlayer(session);
	if(pl == nullptr)
		return;
	auto *charComponent = static_cast<pragma::SCharacterComponent*>(pl->GetEntity().GetCharacterComponent().get());
	if(charComponent == nullptr)
		return;
	charComponent->SetGodMode(!charComponent->GetGodMode());
	pl->PrintMessage(std::string("God mode turned ") +((charComponent->GetGodMode() == true) ? "ON" : "OFF"),MESSAGE::PRINTCONSOLE);
}

void NET_sv_suicide(pragma::networking::IServerClient &session,NetPacket packet)
{
	if(!server->CheatsEnabled())
		return;
	auto *pl = server->GetPlayer(session);
	if(pl == nullptr)
		return;
	auto charComponent = pl->GetEntity().GetCharacterComponent();
	if(charComponent.expired())
		return;
	charComponent->Kill();
}

void NET_sv_hurtme(pragma::networking::IServerClient &session,NetPacket packet)
{
	if(!server->CheatsEnabled())
		return;
	auto *pl = server->GetPlayer(session);
	if(pl == nullptr)
		return;
	auto &ent = pl->GetEntity();
	auto pDamageableComponent = ent.GetComponent<pragma::DamageableComponent>();
	if(pDamageableComponent.expired())
		return;
	auto dmg = packet->Read<uint16_t>();
	DamageInfo dmgInfo {};
	dmgInfo.SetDamage(dmg);
	dmgInfo.SetAttacker(&ent);
	dmgInfo.SetInflictor(&ent);
	dmgInfo.SetDamageType(DAMAGETYPE::CRUSH);
	pDamageableComponent->TakeDamage(dmgInfo);
}

void NET_sv_weapon_next(pragma::networking::IServerClient &session,NetPacket packet)
{
	auto *pl = server->GetPlayer(session);
	if(pl == nullptr)
		return;
	auto sCharComponent = static_cast<pragma::SCharacterComponent*>(pl->GetEntity().GetCharacterComponent().get());
	if(sCharComponent == nullptr)
		return;
	sCharComponent->SelectNextWeapon();
}

void NET_sv_weapon_previous(pragma::networking::IServerClient &session,NetPacket packet)
{
	auto *pl = server->GetPlayer(session);
	if(pl == nullptr)
		return;
	auto sCharComponent = static_cast<pragma::SCharacterComponent*>(pl->GetEntity().GetCharacterComponent().get());
	if(sCharComponent == nullptr)
		return;
	sCharComponent->SelectPreviousWeapon();
}

void NET_sv_give_weapon(pragma::networking::IServerClient &session,NetPacket packet)
{
	if(!server->CheatsEnabled() || s_game == nullptr)
		return;
	auto *pl = server->GetPlayer(session);
	if(pl == nullptr)
		return;
	auto sCharComponent = static_cast<pragma::SCharacterComponent*>(pl->GetEntity().GetCharacterComponent().get());
	if(sCharComponent == nullptr)
		return;
	auto weaponClass = packet->ReadString();
	auto *wep = sCharComponent->GiveWeapon(weaponClass);
	if(wep == nullptr)
		return;
	sCharComponent->DeployWeapon(*wep);
}

void NET_sv_strip_weapons(pragma::networking::IServerClient &session,NetPacket packet)
{
	if(!server->CheatsEnabled() || s_game == nullptr)
		return;
	auto *pl = server->GetPlayer(session);
	if(pl == nullptr)
		return;
	auto sCharComponent = static_cast<pragma::SCharacterComponent*>(pl->GetEntity().GetCharacterComponent().get());
	if(sCharComponent == nullptr)
		return;
	sCharComponent->RemoveWeapons();
}

void NET_sv_give_ammo(pragma::networking::IServerClient &session,NetPacket packet)
{
	if(!server->CheatsEnabled() || s_game == nullptr)
		return;
	auto *pl = server->GetPlayer(session);
	if(pl == nullptr)
		return;
	auto ammoTypeClass = packet->ReadString();
	uint32_t ammoTypeId;
	if(s_game->GetAmmoType(ammoTypeClass,&ammoTypeId) == nullptr)
		return;
	auto sCharComponent = static_cast<pragma::SCharacterComponent*>(pl->GetEntity().GetCharacterComponent().get());
	if(sCharComponent == nullptr)
		return;
	auto am = packet->Read<uint32_t>();
	am += sCharComponent->GetAmmoCount(ammoTypeId);
	sCharComponent->SetAmmoCount(ammoTypeId,static_cast<uint16_t>(am));
}

void NET_sv_debug_ai_schedule_print(pragma::networking::IServerClient &session,NetPacket packet)
{
	if(!server->CheatsEnabled() || s_game == nullptr)
		return;
	auto *pl = server->GetPlayer(session);
	if(pl == nullptr)
		return;
	auto *npc = nwm::read_entity(packet);
	if(npc == nullptr || npc->IsNPC() == false)
		return;
	auto sAiComponent = npc->GetComponent<pragma::SAIComponent>();
	auto schedule = (sAiComponent.expired() == false) ? sAiComponent.get()->GetCurrentSchedule() : nullptr;
	NetPacket response;
	if(schedule == nullptr)
		response->Write<bool>(false);
	else
	{
		response->Write<bool>(true);
		std::stringstream ss;
		schedule->DebugPrint(ss);
		response->WriteString(ss.str());
	}
	server->SendPacket("debug_ai_schedule_print",response,pragma::networking::Protocol::SlowReliable,session);
}

void NET_sv_debug_ai_schedule_tree(pragma::networking::IServerClient &session,NetPacket packet)
{
	if(!server->CheatsEnabled() || s_game == nullptr)
		return;
	auto *pl = server->GetPlayer(session);
	if(pl == nullptr)
		return;
	auto *ent = nwm::read_entity(packet);
	if(ent == nullptr || ent->IsNPC() == false)
		return;
	auto hPl = pl->GetHandle<pragma::SPlayerComponent>();
	auto sAiComponent = ent->GetComponent<pragma::SAIComponent>();
	auto hSAiComponent = (sAiComponent.expired() == false) ? sAiComponent.get()->GetHandle<pragma::SAIComponent>() : util::WeakHandle<pragma::SAIComponent>{};
	auto dbgTree = std::make_shared<DebugBehaviorTreeNode>();
	std::shared_ptr<pragma::ai::Schedule> aiSchedule = nullptr;
	auto tLastScheduleUpdate = 0.f;
	auto hCbTick = FunctionCallback<void>::Create(nullptr);
	auto hCbOnGameEnd = FunctionCallback<void,SGame*>::Create(nullptr);
	auto fClearCallbacks = [hCbTick,hCbOnGameEnd]() mutable {
		if(hCbTick.IsValid())
			hCbTick.Remove();
		if(hCbOnGameEnd.IsValid())
			hCbOnGameEnd.Remove();
	};
	hCbTick.get<Callback<void>>()->SetFunction([hPl,hSAiComponent,hCbTick,hCbOnGameEnd,fClearCallbacks,dbgTree,aiSchedule,tLastScheduleUpdate]() mutable {
		if(hPl.expired() || hSAiComponent.expired())
		{
			fClearCallbacks();
			return;
		}
		hSAiComponent.get()->_debugSendScheduleInfo(*hPl.get(),dbgTree,aiSchedule,tLastScheduleUpdate);
	});
	hCbOnGameEnd.get<Callback<void,SGame*>>()->SetFunction([fClearCallbacks](SGame *game) mutable {
		fClearCallbacks();
	});
	s_game->AddCallback("Tick",hCbTick);
	s_game->AddCallback("OnGameEnd",hCbOnGameEnd);
}
