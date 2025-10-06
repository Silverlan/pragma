// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_server.h"

import pragma.server.entities;
import pragma.server.entities.components;
import pragma.server.game;
import pragma.server.server_state;

void CMD_sv_send(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	if(argv.empty())
		return;
	NetPacket packet;
	packet->WriteString(argv[(argv.size() == 1) ? 0 : 1]);
	if(argv.size() == 1)
		ServerState::Get()->SendPacket("sv_send", packet, pragma::networking::Protocol::SlowReliable);
	else {
		/*ServerState::Get()->
		ClientSession *cs = GetSessionByPlayerID(atoi(argv[0]));
		if(!cs)
		{
			Con::cout<<"No player with ID "<<atoi(argv[0])<<" found!"<<Con::endl;
			return;
		}
		ServerState::Get()->SendTCPMessage("sv_send",&packet,cs);*/
	}
}

void CMD_sv_send_udp(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	if(argv.empty())
		return;
	NetPacket packet;
	packet->WriteString(argv[(argv.size() == 1) ? 0 : 1]);
	if(argv.size() == 1)
		ServerState::Get()->SendPacket("sv_send", packet, pragma::networking::Protocol::FastUnreliable);
	else {
		/*ClientSession *cs = GetSessionByPlayerID(atoi(argv[0]));
		if(!cs)
		{
			Con::cout<<"No player with ID "<<atoi(argv[0])<<" found!"<<Con::endl;
			return;
		}
		ServerState::Get()->SendUDPMessage("sv_send",&packet,cs);*/
	}
}

void CMD_ent_input(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	if(!check_cheats("ent_input", state))
		return;
	if(SGame::Get() == NULL)
		return;
	auto *activator = (pl != nullptr) ? &pl->GetEntity() : nullptr;
	if(argv.size() >= 2) {
		auto ents = command::find_named_targets(state, argv[0]);
		auto bFound = false;
		for(auto *ent : ents) {
			auto pInputComponent = ent->GetComponent<pragma::SIOComponent>();
			if(pInputComponent.valid()) {
				bFound = true;
				pInputComponent->Input(argv[1], activator, activator, "");
			}
		}
		if(bFound == false)
			Con::cwar << "No targets found by name '" << argv[0] << "'!" << Con::endl;
		return;
	}
	if(argv.size() < 1 || activator == nullptr || activator->IsCharacter() == false)
		return;
	auto charComponent = activator->GetCharacterComponent();
	auto ents = command::find_trace_targets(state, *charComponent);
	std::vector<std::string> substrings;
	ustring::explode_whitespace(argv.front(), substrings);
	if(substrings.empty() == true)
		return;
	auto input = substrings.front();
	auto data = (substrings.size() > 1) ? substrings.at(1) : "";
	auto bFound = false;
	for(auto *ent : ents) {
		auto pInputComponent = ent->GetComponent<pragma::SIOComponent>();
		if(pInputComponent.valid()) {
			bFound = true;
			pInputComponent->Input(input, activator, activator, data);
		}
	}
	if(bFound == false)
		Con::cwar << "No raycast target found!" << Con::endl;
}

void CMD_ent_scale(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	if(!check_cheats("ent_scale", state))
		return;
	if(SGame::Get() == nullptr)
		return;
	if(argv.size() >= 2) {
		auto ents = command::find_named_targets(state, argv[0]);
		auto scale = atof(argv[1].c_str());
		for(auto *ent : ents) {
			auto pTransformComponent = ent->GetTransformComponent();
			if(pTransformComponent)
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
	auto ents = command::find_trace_targets(state, *charComponent);
	for(auto *ent : ents) {
		auto pTransformComponent = ent->GetTransformComponent();
		if(pTransformComponent)
			pTransformComponent->SetScale(static_cast<float>(scale));
	}
}

void CMD_ent_remove(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	if(!check_cheats("ent_remove", state))
		return;
	if(SGame::Get() == NULL || pl == NULL)
		return;
	auto &ent = pl->GetEntity();
	if(ent.IsCharacter() == false)
		return;
	auto charComponent = ent.GetCharacterComponent();
	auto ents = command::find_target_entity(state, *charComponent, argv);
	if(ents.empty()) {
		Con::cwar << "No entity found to remove!" << Con::endl;
		return;
	}
	for(auto *ent : ents)
		ent->Remove();
}

void CMD_ent_create(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	if(!check_cheats("ent_create", state))
		return;
	if(SGame::Get() == NULL)
		return;
	if(argv.empty() || pl == NULL)
		return;
	auto &ent = pl->GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	if(pTrComponent == nullptr)
		return;
	auto charComponent = ent.GetCharacterComponent();
	Vector3 origin = pTrComponent->GetEyePosition();
	Vector3 dir = charComponent.valid() ? charComponent->GetViewForward() : pTrComponent->GetForward();
	TraceData trData;
	trData.SetSource(origin);
	trData.SetTarget(origin + dir * static_cast<float>(GameLimits::MaxRayCastRange));
	trData.SetFilter(ent);
	trData.SetFlags(RayCastFlags::Default | RayCastFlags::InvertFilter | RayCastFlags::IgnoreDynamic);
	auto r = SGame::Get()->RayCast(trData);
	if(r.hitType == RayCastHitType::None) {
		Con::cwar << "No place to spawn entity!" << Con::endl;
		return;
	}
	std::string className = argv[0];
	BaseEntity *entNew = SGame::Get()->CreateEntity(className);
	if(entNew == NULL)
		return;
	auto pTrComponentNew = entNew->GetTransformComponent();
	if(pTrComponentNew) {
		auto posSpawn = r.position;
		auto pPhysComponent = entNew->GetPhysicsComponent();
		if(pPhysComponent != nullptr)
			posSpawn += r.normal * pPhysComponent->GetCollisionRadius();
		else
			posSpawn += r.normal * 1.f;
		pTrComponentNew->SetPosition(posSpawn);
	}
	if(argv.size() > 1) {
		auto pNameComponent = entNew->GetComponent<pragma::SNameComponent>();
		if(pNameComponent.valid())
			pNameComponent->SetName(argv[1]);
	}
	entNew->Spawn();
}

void CMD_nav_reload(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	if(SGame::Get() == NULL)
		return;
	SGame::Get()->LoadNavMesh(true);
}

void CMD_nav_generate(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(SGame::Get() == NULL)
		return;
	std::string map = SGame::Get()->GetMapName();
	if(map.empty())
		return;
	std::string err;
	Con::cout << "Generating navigation mesh..." << Con::endl;
	const pragma::nav::Config navCfg {
	  32.f /* walkableRadius */, 64.f, /* characterHeight */
	  20.f,                            /* maxClimbHeight */
	  45.f                             /* walkableSlopeAngle */
	};
	auto rcNavMesh = pragma::nav::generate(*SGame::Get(), navCfg, &err);
	if(rcNavMesh == nullptr)
		Con::cwar << "Unable to generate navigation mesh: " << err << Con::endl;
	else {
		auto navMesh = pragma::nav::Mesh::Create(rcNavMesh, navCfg);
		Con::cout << "Navigation mesh has been generated!" << Con::endl;
		std::string path = "maps\\" + map;
		path += "." + std::string {pragma::nav::PNAV_EXTENSION_BINARY};
		std::string err;
		if(navMesh->Save(*SGame::Get(), path, err) == false)
			Con::cwar << "Unable to save navigation mesh as '" << path << "': " << err << "!" << Con::endl;
	}
}

void CMD_heartbeat(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(ServerState::Get() == nullptr)
		return;
	auto *sv = ServerState::Get()->GetServer();
	if(sv == nullptr)
		return;
	sv->Heartbeat();
}

void CMD_sv_debug_netmessages(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	auto *sv = ServerState::Get()->GetServer();
	if(sv == nullptr) {
		Con::cwar << "No server is active!" << Con::endl;
		return;
	}
	if(argv.size() > 0) {
		auto numBacklog = ustring::to_int(argv.front());
		sv->SetMemoryCount(numBacklog);
		Con::cout << "Debug backlog has been set to " << numBacklog << Con::endl;
		return;
	}
	auto *svMap = GetServerMessageMap();
	std::unordered_map<std::string, uint32_t> *svMsgs;
	svMap->GetNetMessages(&svMsgs);

	auto *clMap = GetClientMessageMap();
	std::unordered_map<std::string, uint32_t> *clMsgs;
	clMap->GetNetMessages(&clMsgs);

	sv->DebugPrint(*svMsgs, *clMsgs);
	sv->DebugDump("sv_netmessages.dump", *svMsgs, *clMsgs);
}
