// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_server.h"
#include "pragma/networking/s_net_global.h"
#include "pragma/networking/resourcemanager.h"
#include "pragma/physics/movetypes.h"
#include "pragma/game/damagetype.h"
#include <pragma/networking/nwm_util.h>
#include "pragma/networking/iserver.hpp"
#include "pragma/networking/iserver_client.hpp"
#include "pragma/networking/recipient_filter.hpp"
#include <pragma/engine.h>
#include "pragma/ai/ai_schedule.h"
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

import pragma.server.entities;
import pragma.server.entities.components;
import pragma.server.game;
import pragma.server.networking;
import pragma.server.server_state;

DLLSERVER void NET_sv_disconnect(pragma::networking::IServerClient &session, NetPacket packet)
{
#ifdef DEBUG_SOCKET
	Con::csv << "Client '" << session.GetIdentifier() << "' has disconnected." << Con::endl;
#endif
	ServerState::Get()->DropClient(session);
}

DLLSERVER void NET_sv_userinput(pragma::networking::IServerClient &session, NetPacket packet) { ServerState::Get()->ReceiveUserInput(session, packet); }

DLLSERVER void NET_sv_ent_event(pragma::networking::IServerClient &session, NetPacket packet)
{
	if(!ServerState::Get()->IsGameActive())
		return;
	auto *pl = SGame::Get()->GetPlayer(session);
	if(pl == nullptr)
		return;
	packet->SetOffset(packet->GetDataSize() - sizeof(UInt32) - sizeof(unsigned int));
	auto *ent = static_cast<SBaseEntity *>(nwm::read_entity(packet));
	if(ent == nullptr)
		return;
	auto eventId = packet->Read<UInt32>();
	packet->SetOffset(0);
	ent->ReceiveNetEvent(*pl, eventId, packet);
}

DLLSERVER void NET_sv_clientinfo(pragma::networking::IServerClient &session, NetPacket packet)
{
	if(!ServerState::Get()->IsGameActive())
		return;
	SGame *game = ServerState::Get()->GetGameState();
	game->ReceiveUserInfo(session, packet);
}

void NET_sv_game_ready(pragma::networking::IServerClient &session, NetPacket packet)
{
	if(!ServerState::Get()->IsGameActive())
		return;
	auto *game = ServerState::Get()->GetGameState();
	game->ReceiveGameReady(session, packet);
}

DLLSERVER void NET_sv_cmd_setpos(pragma::networking::IServerClient &session, NetPacket packet)
{
	if(ServerState::Get()->CheatsEnabled() == false)
		return;
	if(!ServerState::Get()->IsGameActive())
		return;
	auto *pl = SGame::Get()->GetPlayer(session);
	if(pl == NULL)
		return;
	auto pTrComponent = pl->GetEntity().GetTransformComponent();
	if(pTrComponent == nullptr)
		return;
	Vector3 pos = nwm::read_vector(packet);
	pTrComponent->SetPosition(pos);
}

DLLSERVER void NET_sv_cmd_call(pragma::networking::IServerClient &session, NetPacket packet)
{
	auto *pl = SGame::Get()->GetPlayer(session);
	std::string cmd = packet->ReadString();
	auto pressState = static_cast<KeyState>(packet->Read<uint8_t>());
	auto magnitude = packet->Read<float>();
	unsigned char argc = packet->Read<unsigned char>();
	std::vector<std::string> argv;
	for(unsigned char i = 0; i < argc; i++)
		argv.push_back(packet->ReadString());

	auto bActionCmd = (cmd.empty() == false && cmd.front() == '+') ? true : false;
	auto bReleased = (pressState == KeyState::Release) ? true : false;
	auto r = ServerState::Get()->RunConsoleCommand(cmd, argv, pl, pressState, magnitude, [bActionCmd, bReleased](ConConf *cf, float &magnitude) -> bool {
		if(bReleased == false || bActionCmd == true)
			return true;
		auto flags = cf->GetFlags();
		if((flags & (ConVarFlags::JoystickAxisContinuous | ConVarFlags::JoystickAxisSingle)) == ConVarFlags::None)
			return false;
		return true;
	});
	if(Engine::Get()->GetClientState() != nullptr)
		return;
	NetPacket p;
	auto *cv = ServerState::Get()->GetConVar(cmd);
	if(r == false || cv == nullptr)
		p->Write<uint8_t>(static_cast<uint8_t>(0));
	else {
		if(cv->GetType() == ConType::Var) {
			p->Write<uint8_t>(2);
			p->WriteString(static_cast<ConVar *>(cv)->GetString());
		}
		else {
			p->Write<uint8_t>(1);
			p->WriteString("");
		}
	}
	ServerState::Get()->SendPacket("cmd_call_response", p, pragma::networking::Protocol::SlowReliable, session);
}

DLLSERVER void NET_sv_rcon(pragma::networking::IServerClient &session, NetPacket packet)
{
	if(!ServerState::Get()->IsGameActive())
		return;
	std::string passCl = packet->ReadString();
	std::string cvar = packet->ReadString();
	std::string pass = ServerState::Get()->GetConVarString("rcon_password");
	if(pass.empty() || pass != passCl) {
		/*Game *game = ServerState::Get()->GetGameState();
		Player *pl = game->GetPlayer(session);
		if(pl == NULL)
			return;
		pl->PrintMessage("Bad RCON password",MESSAGE::PRINTCONSOLE);*/ // TODO
		Con::cerr << "Incorrect RCON Password! (" << passCl << ")" << Con::endl;
		return;
	}
	Con::csv << "Remote console input from " << session.GetIdentifier() << ": '" << cvar << "'" << Con::endl;
	Engine::Get()->ConsoleInput(cvar.c_str());
}

DLLSERVER void NET_sv_serverinfo_request(pragma::networking::IServerClient &session, NetPacket packet)
{
	std::string password = packet->ReadString();
	std::string passSv = ServerState::Get()->GetConVarString("sv_password").c_str();
	if(passSv.empty() == false && passSv != password && session.IsListenServerHost() == false) {
		NetPacket p;
		ServerState::Get()->SendPacket("invalidpassword", p, pragma::networking::Protocol::SlowReliable, session);
		ServerState::Get()->DropClient(session);
		return;
	}
	NetPacket p;
	auto *sv = ServerState::Get()->GetServer();
	if(sv && sv->GetHostPort().has_value()) {
		auto port = sv->GetHostPort();
		p->Write<unsigned char>(1);
		p->Write<unsigned short>(*port);
	}
	else
		p->Write<unsigned char>((unsigned char)(0));

	p->Write<bool>(ServerState::Get()->IsClientAuthenticationRequired());
	ServerState::Get()->SendPacket("serverinfo", p, pragma::networking::Protocol::SlowReliable, session);
}

DLLSERVER void NET_sv_authenticate(pragma::networking::IServerClient &session, NetPacket packet)
{
	auto hasAuth = packet->Read<bool>();
	if(ServerState::Get()->IsClientAuthenticationRequired()) {
		if(hasAuth == false) {
			Con::cerr << "Unable to authenticate client '" << session.GetIdentifier() << "': Client did not transmit authentication information!" << Con::endl;
			ServerState::Get()->DropClient(session, pragma::networking::DropReason::AuthenticationFailed);
			return;
		}
		auto *reg = ServerState::Get()->GetMasterServerRegistration();
		if(reg == nullptr) {
			Con::cerr << "Unable to authenticate client '" << session.GetIdentifier() << "': Server is not connected to master server!" << Con::endl;
			ServerState::Get()->DropClient(session, pragma::networking::DropReason::AuthenticationFailed);
			return;
		}
		auto steamId = packet->Read<uint64_t>();
		auto lenToken = packet->Read<uint16_t>();
		std::vector<char> token;
		token.resize(lenToken);
		packet->Read(token.data(), token.size() * sizeof(token.front()));

		std::string err;
		auto libSteamworks = ServerState::Get()->InitializeLibrary("steamworks/pr_steamworks", &err);
		if(libSteamworks == nullptr) {
			Con::cerr << "Unable to authenticate client with steam id '" << steamId << "': Steamworks module could not be loaded: " << err << Con::endl;
			ServerState::Get()->DropClient(session, pragma::networking::DropReason::AuthenticationFailed);
			reg->DropClient(steamId);
			return;
		}
		session.SetSteamId(steamId);
		// Authentication will end with 'OnClientAuthenticated' callback,
		// which will handle the rest
		reg->AuthenticateAndAddClient(steamId, token, "Player");
		return;
	}
	// No authentication required; Continue immediately
	ServerState::Get()->OnClientAuthenticated(session, {});
}

DLLSERVER void NET_sv_cvar_set(pragma::networking::IServerClient &session, NetPacket packet)
{
	if(!ServerState::Get()->IsGameActive())
		return;
	SGame *game = ServerState::Get()->GetGameState();
	auto *pl = game->GetPlayer(session);
	if(pl == NULL)
		return;
	std::string cvar = packet->ReadString();
	std::string val = packet->ReadString();
	std::unordered_map<std::string, std::string> *cvars;
	pl->GetConVars(&cvars);
	(*cvars)[cvar] = val;
	game->OnClientConVarChanged(*pl, cvar, val);
}

DLLSERVER void NET_sv_noclip(pragma::networking::IServerClient &session, NetPacket packet)
{
	if(!ServerState::Get()->CheatsEnabled())
		return;
	auto *pl = ServerState::Get()->GetPlayer(session);
	if(pl == NULL)
		return;
	auto pPhysComponent = pl->GetEntity().GetPhysicsComponent();
	if(pPhysComponent == nullptr)
		return;
	auto bNoclip = pPhysComponent->GetMoveType() != MOVETYPE::NOCLIP;
	if(bNoclip == false) {
		pPhysComponent->SetMoveType(MOVETYPE::WALK);
		pPhysComponent->SetCollisionFilterGroup(CollisionMask::Player);
	}
	else {
		pPhysComponent->SetMoveType(MOVETYPE::NOCLIP);
		pPhysComponent->SetCollisionFilterGroup(CollisionMask::NoCollision);
		//pl->SetCollisionsEnabled(false); // Bugged due to CCD
	}
	NetPacket p;
	nwm::write_entity(p, &pl->GetEntity());
	p->Write<bool>(bNoclip);
	ServerState::Get()->SendPacket("pl_toggle_noclip", p, pragma::networking::Protocol::SlowReliable);
}

DLLSERVER void NET_sv_luanet(pragma::networking::IServerClient &session, ::NetPacket packet) { ServerState::Get()->HandleLuaNetPacket(session, packet); }

void NET_sv_notarget(pragma::networking::IServerClient &session, NetPacket packet)
{
	if(!ServerState::Get()->CheatsEnabled())
		return;
	auto *pl = ServerState::Get()->GetPlayer(session);
	if(pl == nullptr)
		return;
	auto *charComponent = static_cast<pragma::SCharacterComponent *>(pl->GetEntity().GetCharacterComponent().get());
	if(charComponent == nullptr)
		return;
	charComponent->SetNoTarget(!charComponent->GetNoTarget());
	pl->PrintMessage(std::string("Notarget turned ") + ((charComponent->GetNoTarget() == true) ? "ON" : "OFF"), MESSAGE::PRINTCONSOLE);
}

void NET_sv_godmode(pragma::networking::IServerClient &session, NetPacket packet)
{
	if(!ServerState::Get()->CheatsEnabled())
		return;
	auto *pl = ServerState::Get()->GetPlayer(session);
	if(pl == nullptr)
		return;
	auto *charComponent = static_cast<pragma::SCharacterComponent *>(pl->GetEntity().GetCharacterComponent().get());
	if(charComponent == nullptr)
		return;
	charComponent->SetGodMode(!charComponent->GetGodMode());
	pl->PrintMessage(std::string("God mode turned ") + ((charComponent->GetGodMode() == true) ? "ON" : "OFF"), MESSAGE::PRINTCONSOLE);
}

void NET_sv_suicide(pragma::networking::IServerClient &session, NetPacket packet)
{
	if(!ServerState::Get()->CheatsEnabled())
		return;
	auto *pl = ServerState::Get()->GetPlayer(session);
	if(pl == nullptr)
		return;
	auto charComponent = pl->GetEntity().GetCharacterComponent();
	if(charComponent.expired())
		return;
	charComponent->Kill();
}

void NET_sv_hurtme(pragma::networking::IServerClient &session, NetPacket packet)
{
	if(!ServerState::Get()->CheatsEnabled())
		return;
	auto *pl = ServerState::Get()->GetPlayer(session);
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

void NET_sv_weapon_next(pragma::networking::IServerClient &session, NetPacket packet)
{
	auto *pl = ServerState::Get()->GetPlayer(session);
	if(pl == nullptr)
		return;
	auto sCharComponent = static_cast<pragma::SCharacterComponent *>(pl->GetEntity().GetCharacterComponent().get());
	if(sCharComponent == nullptr)
		return;
	sCharComponent->SelectNextWeapon();
}

void NET_sv_weapon_previous(pragma::networking::IServerClient &session, NetPacket packet)
{
	auto *pl = ServerState::Get()->GetPlayer(session);
	if(pl == nullptr)
		return;
	auto sCharComponent = static_cast<pragma::SCharacterComponent *>(pl->GetEntity().GetCharacterComponent().get());
	if(sCharComponent == nullptr)
		return;
	sCharComponent->SelectPreviousWeapon();
}

void NET_sv_give_weapon(pragma::networking::IServerClient &session, NetPacket packet)
{
	if(!ServerState::Get()->CheatsEnabled() || SGame::Get() == nullptr)
		return;
	auto *pl = ServerState::Get()->GetPlayer(session);
	if(pl == nullptr)
		return;
	auto sCharComponent = static_cast<pragma::SCharacterComponent *>(pl->GetEntity().GetCharacterComponent().get());
	if(sCharComponent == nullptr)
		return;
	auto weaponClass = packet->ReadString();
	auto *wep = sCharComponent->GiveWeapon(weaponClass);
	if(wep == nullptr)
		return;
	sCharComponent->DeployWeapon(*wep);
}

void NET_sv_strip_weapons(pragma::networking::IServerClient &session, NetPacket packet)
{
	if(!ServerState::Get()->CheatsEnabled() || SGame::Get() == nullptr)
		return;
	auto *pl = ServerState::Get()->GetPlayer(session);
	if(pl == nullptr)
		return;
	auto sCharComponent = static_cast<pragma::SCharacterComponent *>(pl->GetEntity().GetCharacterComponent().get());
	if(sCharComponent == nullptr)
		return;
	sCharComponent->RemoveWeapons();
}

void NET_sv_give_ammo(pragma::networking::IServerClient &session, NetPacket packet)
{
	if(!ServerState::Get()->CheatsEnabled() || SGame::Get() == nullptr)
		return;
	auto *pl = ServerState::Get()->GetPlayer(session);
	if(pl == nullptr)
		return;
	auto ammoTypeClass = packet->ReadString();
	uint32_t ammoTypeId;
	if(SGame::Get()->GetAmmoType(ammoTypeClass, &ammoTypeId) == nullptr)
		return;
	auto sCharComponent = static_cast<pragma::SCharacterComponent *>(pl->GetEntity().GetCharacterComponent().get());
	if(sCharComponent == nullptr)
		return;
	auto am = packet->Read<uint32_t>();
	am += sCharComponent->GetAmmoCount(ammoTypeId);
	sCharComponent->SetAmmoCount(ammoTypeId, static_cast<uint16_t>(am));
}

void NET_sv_debug_ai_schedule_print(pragma::networking::IServerClient &session, NetPacket packet)
{
	if(!ServerState::Get()->CheatsEnabled() || SGame::Get() == nullptr)
		return;
	auto *pl = ServerState::Get()->GetPlayer(session);
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
	else {
		response->Write<bool>(true);
		std::stringstream ss;
		schedule->DebugPrint(ss);
		response->WriteString(ss.str());
	}
	ServerState::Get()->SendPacket("debug_ai_schedule_print", response, pragma::networking::Protocol::SlowReliable, session);
}

void NET_sv_debug_ai_schedule_tree(pragma::networking::IServerClient &session, NetPacket packet)
{
	if(!ServerState::Get()->CheatsEnabled() || SGame::Get() == nullptr)
		return;
	auto *pl = ServerState::Get()->GetPlayer(session);
	if(pl == nullptr)
		return;
	auto *ent = nwm::read_entity(packet);
	if(ent == nullptr || ent->IsNPC() == false)
		return;
	auto hPl = pl->GetHandle<pragma::SPlayerComponent>();
	auto sAiComponent = ent->GetComponent<pragma::SAIComponent>();
	auto hSAiComponent = (sAiComponent.expired() == false) ? sAiComponent.get()->GetHandle<pragma::SAIComponent>() : pragma::ComponentHandle<pragma::SAIComponent> {};
	auto dbgTree = std::make_shared<DebugBehaviorTreeNode>();
	std::shared_ptr<pragma::ai::Schedule> aiSchedule = nullptr;
	auto tLastScheduleUpdate = 0.f;
	auto hCbTick = FunctionCallback<void>::Create(nullptr);
	auto hCbOnGameEnd = FunctionCallback<void, SGame *>::Create(nullptr);
	auto fClearCallbacks = [hCbTick, hCbOnGameEnd]() mutable {
		if(hCbTick.IsValid())
			hCbTick.Remove();
		if(hCbOnGameEnd.IsValid())
			hCbOnGameEnd.Remove();
	};
	hCbTick.get<Callback<void>>()->SetFunction([hPl, hSAiComponent, hCbTick, hCbOnGameEnd, fClearCallbacks, dbgTree, aiSchedule, tLastScheduleUpdate]() mutable {
		if(hPl.expired() || hSAiComponent.expired()) {
			fClearCallbacks();
			return;
		}
		hSAiComponent.get()->_debugSendScheduleInfo(*hPl.get(), dbgTree, aiSchedule, tLastScheduleUpdate);
	});
	hCbOnGameEnd.get<Callback<void, SGame *>>()->SetFunction([fClearCallbacks](SGame *game) mutable { fClearCallbacks(); });
	SGame::Get()->AddCallback("Tick", hCbTick);
	SGame::Get()->AddCallback("OnGameEnd", hCbOnGameEnd);
}

void NET_sv_debug_ai_navigation(pragma::networking::IServerClient &session, NetPacket packet)
{
	if(!ServerState::Get()->CheatsEnabled() || SGame::Get() == nullptr)
		return;
	auto *pl = ServerState::Get()->GetPlayer(session);
	if(pl == nullptr)
		return;
	auto b = packet->Read<bool>();
	if(b == false) {
		auto it = std::find_if(pragma::SAIComponent::s_plDebugAiNav.begin(), pragma::SAIComponent::s_plDebugAiNav.end(), [pl](const pragma::ComponentHandle<pragma::SPlayerComponent> &sPlComponent) { return (sPlComponent.get() == pl) ? true : false; });
		if(it != pragma::SAIComponent::s_plDebugAiNav.end())
			pragma::SAIComponent::s_plDebugAiNav.erase(it);
		return;
	}
	pragma::SAIComponent::s_plDebugAiNav.push_back(pl->GetHandle<pragma::SPlayerComponent>());
	auto &npcs = pragma::SAIComponent::GetAll();
	for(auto *npc : npcs)
		npc->_debugSendNavInfo(*pl);
}
