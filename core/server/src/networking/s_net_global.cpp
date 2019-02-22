#include "stdafx_server.h"
#include "pragma/serverstate/serverutil.h"
#include "pragma/networking/s_net_global.h"
#include "pragma/game/s_game.h"
#include "pragma/networking/resourcemanager.h"
#include "pragma/entities/player.h"
#include "pragma/physics/movetypes.h"
#include <pragma/networking/nwm_util.h>
#include "pragma/networking/wvserverclient.h"
#include "pragma/networking/wvlocalclient.h"
#include <pragma/engine.h>
#include "pragma/ai/ai_schedule.h"
#include "pragma/entities/components/s_player_component.hpp"
#include "pragma/entities/components/s_character_component.hpp"
#include "pragma/entities/components/s_ai_component.hpp"
#include <pragma/debug/debugbehaviortree.h>
#include <pragma/entities/components/base_player_component.hpp>
#include <pragma/entities/components/base_character_component.hpp>
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/entities/components/base_physics_component.hpp>
#include <pragma/entities/components/damageable_component.hpp>

extern DLLSERVER ServerState *server;
extern DLLSERVER SGame *s_game;
extern DLLNETWORK Engine *engine;
DLLSERVER void NET_sv_disconnect(WVServerClient *session,NetPacket packet)
{
#ifdef DEBUG_SOCKET
	Con::csv<<"Client '"<<session->GetIP()<<"' has disconnected."<<Con::endl;
#endif
	server->DropClient(session);
}

DLLSERVER void NET_sv_userinput(WVServerClient *session,NetPacket packet)
{
	server->ReceiveUserInput(session,packet);
}

DLLSERVER void NET_sv_ent_event(WVServerClient *session,NetPacket packet)
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

DLLSERVER void NET_sv_clientinfo(WVServerClient *session,NetPacket packet)
{
	if(!server->IsGameActive())
		return;
	SGame *game = server->GetGameState();
	game->ReceiveUserInfo(session,packet);
}

void NET_sv_game_ready(WVServerClient *session,NetPacket packet)
{
	if(!server->IsGameActive())
		return;
	auto *game = server->GetGameState();
	game->ReceiveGameReady(session,packet);
}

DLLSERVER void NET_sv_cmd_setpos(WVServerClient *session,NetPacket packet)
{
	if(server->CheatsEnabled() == false)
		return;
	if(!server->IsGameActive())
		return;
	auto *pl = s_game->GetPlayer(session);
	if(pl == NULL)
		return;
	auto pTrComponent = pl->GetEntity().GetTransformComponent();
	if(pTrComponent.expired())
		return;
	Vector3 pos = nwm::read_vector(packet);
	pTrComponent->SetPosition(pos);
}

DLLSERVER void NET_sv_cmd_call(WVServerClient *session,NetPacket packet)
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
	server->SendPacketTCP("cmd_call_response",p,session);
}

DLLSERVER void NET_sv_rcon(WVServerClient *session,NetPacket packet)
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
	Con::csv<<"Remote console input from "<<session->GetIP()<<": '"<<cvar<<"'"<<Con::endl;
	engine->ConsoleInput(cvar.c_str());
}

DLLSERVER void NET_sv_serverinfo_request(WVServerClient *session,NetPacket packet)
{
	std::string password = packet->ReadString();
	std::string passSv = server->GetConVarString("sv_password");
	if(passSv != "" && passSv != password && dynamic_cast<WVLocalClient*>(session) == nullptr)
	{
		NetPacket p;
		server->SendPacketTCP("invalidpassword",p,session);
		server->DropClient(session);
		return;
	}
	NetPacket p;
	if(server->IsUDPOpen())
	{
		p->Write<unsigned char>(1);
		p->Write<unsigned short>(server->GetUDPPort());
	}
	else
		p->Write<unsigned char>((unsigned char)(0));
	unsigned int numResources = ResourceManager::GetResourceCount();
	p->Write<unsigned int>(numResources);
	server->SendPacketTCP("serverinfo",p,session);
}

DLLSERVER void NET_sv_cvar_set(WVServerClient *session,NetPacket packet)
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

DLLSERVER void NET_sv_noclip(WVServerClient *session,NetPacket packet)
{
	if(!server->CheatsEnabled())
		return;
	auto *pl = server->GetPlayer(session);
	if(pl == NULL)
		return;
	auto pPhysComponent = pl->GetEntity().GetPhysicsComponent();
	if(pPhysComponent.expired())
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
	server->BroadcastTCP("pl_toggle_noclip",p);
}

void NET_sv_notarget(WVServerClient *session,NetPacket packet)
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

void NET_sv_godmode(WVServerClient *session,NetPacket packet)
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

void NET_sv_suicide(WVServerClient *session,NetPacket packet)
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

void NET_sv_hurtme(WVServerClient *session,NetPacket packet)
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

void NET_sv_weapon_next(WVServerClient *session,NetPacket packet)
{
	auto *pl = server->GetPlayer(session);
	if(pl == nullptr)
		return;
	auto sCharComponent = static_cast<pragma::SCharacterComponent*>(pl->GetEntity().GetCharacterComponent().get());
	if(sCharComponent == nullptr)
		return;
	sCharComponent->SelectNextWeapon();
}

void NET_sv_weapon_previous(WVServerClient *session,NetPacket packet)
{
	auto *pl = server->GetPlayer(session);
	if(pl == nullptr)
		return;
	auto sCharComponent = static_cast<pragma::SCharacterComponent*>(pl->GetEntity().GetCharacterComponent().get());
	if(sCharComponent == nullptr)
		return;
	sCharComponent->SelectPreviousWeapon();
}

void NET_sv_give_weapon(WVServerClient *session,NetPacket packet)
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

void NET_sv_give_ammo(WVServerClient *session,NetPacket packet)
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

void NET_sv_debug_ai_schedule_print(WVServerClient *session,NetPacket packet)
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
	server->SendPacketTCP("debug_ai_schedule_print",response,session);
}

void NET_sv_debug_ai_schedule_tree(WVServerClient *session,NetPacket packet)
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
