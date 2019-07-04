#include "stdafx_server.h"
#include "pragma/console/s_cvar_server.h"
#include <pragma/engine.h>
#include <pragma/serverstate/serverstate.h>
#include "pragma/entities/player.h"
#include <pragma/physics/raytraces.h>
#include <pragma/ai/navsystem.h>
#include <pragma/networking/nwm_util.h>
#include "pragma/networking/wvserver.h"
#include <pragma/networking/enums.hpp>
#include <pragma/networking/iserver.hpp>
#include "pragma/entities/components/s_name_component.hpp"
#include "pragma/entities/components/s_io_component.hpp"
#include <pragma/entities/components/base_player_component.hpp>
#include <pragma/entities/components/base_character_component.hpp>
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/entities/components/base_physics_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/console/sh_cmd.h>
#include <pragma/networking/netmessages.h>

extern DLLENGINE Engine *engine;
extern ServerState *server;
extern SGame *s_game;
DLLSERVER void CMD_sv_send(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string> &argv)
{
	if(argv.empty()) return;
	NetPacket packet;
	packet->WriteString(argv[(argv.size() == 1) ? 0 : 1]);
	if(argv.size() == 1)
		server->SendPacket("sv_send",packet,pragma::networking::Protocol::SlowReliable);
	else
	{
		/*server->
		ClientSession *cs = GetSessionByPlayerID(atoi(argv[0]));
		if(!cs)
		{
			Con::cout<<"No player with ID "<<atoi(argv[0])<<" found!"<<Con::endl;
			return;
		}
		server->SendTCPMessage("sv_send",&packet,cs);*/
	}
}

DLLSERVER void CMD_sv_send_udp(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string> &argv)
{
	if(argv.empty()) return;
	NetPacket packet;
	packet->WriteString(argv[(argv.size() == 1) ? 0 : 1]);
	if(argv.size() == 1)
		server->SendPacket("sv_send",packet,pragma::networking::Protocol::FastUnreliable);
	else
	{
		/*ClientSession *cs = GetSessionByPlayerID(atoi(argv[0]));
		if(!cs)
		{
			Con::cout<<"No player with ID "<<atoi(argv[0])<<" found!"<<Con::endl;
			return;
		}
		server->SendUDPMessage("sv_send",&packet,cs);*/
	}
}

DLLSERVER void CMD_ent_input(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv)
{
	CHECK_CHEATS("ent_input",state,);
	if(s_game == NULL)
		return;
	auto *activator = (pl != nullptr) ? &pl->GetEntity() : nullptr;
	if(argv.size() >= 2)
	{
		auto ents = command::find_named_targets(state,argv[0]);
		auto bFound = false;
		for(auto *ent : ents)
		{
			auto pInputComponent = ent->GetComponent<pragma::SIOComponent>();
			if(pInputComponent.valid())
			{
				bFound = true;
				pInputComponent->Input(argv[1],activator,activator,"");
			}
		}
		if(bFound == false)
			Con::cwar<<"WARNING: No targets found by name '"<<argv[0]<<"'!"<<Con::endl;
		return;
	}
	if(argv.size() < 1 || activator == nullptr || activator->IsCharacter() == false)
		return;
	auto charComponent = activator->GetCharacterComponent();
	auto ents = command::find_trace_targets(state,*charComponent);
	std::vector<std::string> substrings;
	ustring::explode_whitespace(argv.front(),substrings);
	if(substrings.empty() == true)
		return;
	auto input = substrings.front();
	auto data = (substrings.size() > 1) ? substrings.at(1) : "";
	auto bFound = false;
	for(auto *ent : ents)
	{
		auto pInputComponent = ent->GetComponent<pragma::SIOComponent>();
		if(pInputComponent.valid())
		{
			bFound = true;
			pInputComponent->Input(input,activator,activator,data);
		}
	}
	if(bFound == false)
		Con::cwar<<"WARNING: No raycast target found!"<<Con::endl;
}

void CMD_ent_scale(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv)
{
	CHECK_CHEATS("ent_scale",state,);
	if(s_game == nullptr)
		return;
	if(argv.size() >= 2)
	{
		auto ents = command::find_named_targets(state,argv[0]);
		auto scale = atof(argv[1].c_str());
		for(auto *ent : ents)
		{
			auto pTransformComponent = ent->GetTransformComponent();
			if(pTransformComponent.valid())
				pTransformComponent->SetScale(static_cast<float>(scale));
		}
		return;
	}
	if(argv.size() < 1 || pl == nullptr)
		return;
	auto &ent = pl->GetEntity();
	if(ent.IsCharacter() == false)
		return;
	auto charComponent = ent.GetCharacterComponent();
	auto scale = atof(argv.front().c_str());
	auto ents = command::find_trace_targets(state,*charComponent);
	for(auto *ent : ents)
	{
		auto pTransformComponent = ent->GetTransformComponent();
		if(pTransformComponent.valid())
			pTransformComponent->SetScale(static_cast<float>(scale));
	}
}

DLLSERVER void CMD_ent_remove(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv)
{
	CHECK_CHEATS("ent_remove",state,);
	if(s_game == NULL || pl == NULL)
		return;
	auto &ent = pl->GetEntity();
	if(ent.IsCharacter() == false)
		return;
	auto charComponent = ent.GetCharacterComponent();
	auto ents = command::find_target_entity(state,*charComponent,argv);
	if(ents.empty())
	{
		Con::cwar<<"WARNING: No entity found to remove!"<<Con::endl;
		return;
	}
	for(auto *ent : ents)
		ent->Remove();
}

DLLSERVER void CMD_ent_create(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv)
{
	CHECK_CHEATS("ent_create",state,);
	if(s_game == NULL)
		return;
	if(argv.empty() || pl == NULL)
		return;
	auto &ent = pl->GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	if(pTrComponent.expired())
		return;
	auto charComponent = ent.GetCharacterComponent();
	Vector3 origin = pTrComponent->GetEyePosition();
	Vector3 dir = charComponent.valid() ? charComponent->GetViewForward() : pTrComponent->GetForward();
	TraceData trData;
	trData.SetSource(origin);
	trData.SetTarget(origin +dir *std::numeric_limits<Float>::max());
#ifdef ENABLE_DEPRECATED_PHYSICS
	trData.SetFilter(ent.GetHandle());
#endif
	trData.SetFlags(RayCastFlags::InvertFilter | RayCastFlags::IgnoreDynamic);
	auto r = s_game->RayCast(trData);
	if(r.hitType == RayCastHitType::None)
	{
		Con::cwar<<"WARNING: No place to spawn entity!"<<Con::endl;
		return;
	}
	std::string className = argv[0];
	BaseEntity *entNew = s_game->CreateEntity(className);
	if(entNew == NULL)
		return;
	auto pTrComponentNew = entNew->GetTransformComponent();
	if(pTrComponentNew.valid())
	{
		auto posSpawn = r.position;
		auto pPhysComponent = entNew->GetPhysicsComponent();
		if(pPhysComponent.valid())
			posSpawn += r.normal *pPhysComponent->GetCollisionRadius();
		else
			posSpawn += r.normal *1.f;
		pTrComponentNew->SetPosition(posSpawn);
	}
	if(argv.size() > 1)
	{
		auto pNameComponent = entNew->GetComponent<pragma::SNameComponent>();
		if(pNameComponent.valid())
			pNameComponent->SetName(argv[1]);
	}
	entNew->Spawn();
}

void CMD_nav_reload(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv)
{
	if(s_game == NULL)
		return;
	s_game->LoadNavMesh(true);
}

DLLSERVER void CMD_nav_generate(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string>&)
{
	if(s_game == NULL)
		return;
	std::string map = s_game->GetMapName();
	if(map.empty())
		return;
	std::string err;
	Con::cout<<"Generating navigation mesh..."<<Con::endl;
	const pragma::nav::Config navCfg {
		32.f /* walkableRadius */,
		64.f, /* characterHeight */
		20.f, /* maxClimbHeight */
		45.f /* walkableSlopeAngle */
	};
	auto rcNavMesh = pragma::nav::generate(*s_game,navCfg,&err);
	if(rcNavMesh == nullptr)
		Con::cwar<<"WARNING: Unable to generate navigation mesh: "<<err<<Con::endl;
	else
	{
		auto navMesh = pragma::nav::Mesh::Create(rcNavMesh,navCfg);
		Con::cout<<"Navigation mesh has been generated!"<<Con::endl;
		std::string path = "maps\\" +map;
		path += ".wnav";
		if(navMesh->Save(*s_game,path.c_str()) == false)
			Con::cwar<<"WARNING: Unable to save navigation mesh as '"<<path<<"'!"<<Con::endl;
	}
}

void CMD_heartbeat(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string>&)
{
	if(server == nullptr)
		return;
	auto *sv = server->GetServer();
	if(sv == nullptr)
		return;
	sv->Heartbeat();
}

void CMD_sv_debug_netmessages(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv)
{
	auto *sv = server->GetServer();
	if(sv == nullptr)
	{
		Con::cwar<<"WARNING: No server is active!"<<Con::endl;
		return;
	}
	if(argv.size() > 0)
	{
		auto numBacklog = ustring::to_int(argv.front());
		sv->SetMemoryCount(numBacklog);
		Con::cout<<"Debug backlog has been set to "<<numBacklog<<Con::endl;
		return;
	}
	auto *svMap = GetServerMessageMap();
	std::unordered_map<std::string,uint32_t> *svMsgs;
	svMap->GetNetMessages(&svMsgs);

	auto *clMap = GetClientMessageMap();
	std::unordered_map<std::string,uint32_t> *clMsgs;
	clMap->GetNetMessages(&clMsgs);

	sv->DebugPrint(*svMsgs,*clMsgs);
	sv->DebugDump("sv_netmessages.dump",*svMsgs,*clMsgs);
}

