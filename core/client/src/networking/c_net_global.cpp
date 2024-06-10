/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/clientstate/clientutil.h"
#include "pragma/c_engine.h"
#include "pragma/networking/c_net_global.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/physics/movetypes.h"
#include "pragma/physics/collisiontypes.h"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/entities/components/c_animated_component.hpp"
#include "pragma/entities/components/c_character_component.hpp"
#include "pragma/entities/components/c_ownable_component.hpp"
#include "pragma/entities/components/c_generic_component.hpp"
#include "pragma/entities/components/c_transform_component.hpp"
#include "pragma/entities/components/c_io_component.hpp"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/entities/environment/lights/c_env_light.h"
#include "pragma/entities/environment/lights/c_env_light_spot.h"
#include "pragma/entities/c_flashlight.h"
#include <pragma/entities/components/basetoggle.h>
#include "pragma/entities/environment/c_env_fog_controller.h"
#include "pragma/entities/environment/effects/c_env_explosion.h"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include <pragma/networking/nwm_util.h>
#include "pragma/entities/environment/c_env_quake.h"
#include "pragma/debug/c_debugoverlay.h"
#include "pragma/debug/c_debug_game_gui.h"
#include "pragma/gui/witreelist.h"
#include "pragma/entities/components/c_weapon_component.hpp"
#include "pragma/entities/c_player.hpp"
#include "pragma/entities/components/c_sound_emitter_component.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_name_component.hpp"
#include "pragma/entities/components/c_health_component.hpp"
#include "pragma/entities/components/c_attachable_component.hpp"
#include "pragma/gui/wiluabase.h"
#include <pragma/math/intersection.h>
#include <pragma/entities/baseentity_trace.hpp>
#include <pragma/entities/components/map_component.hpp>
#include <pragma/audio/alsound_type.h>
#include <pragma/debug/debugbehaviortree.h>
#include <wgui/types/witext.h>
#include <pragma/console/sh_cmd.h>
#include <pragma/physics/raytraces.h>
#include <pragma/entities/components/basetriggergravity.hpp>
#include <pragma/entities/components/base_physics_component.hpp>
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/networking/enums.hpp>
#include <pragma/util/giblet_create_info.hpp>

enum class CLIENT_DROPPED;
extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

DLLCLIENT void NET_cl_serverinfo(NetPacket packet) { client->HandleClientReceiveServerInfo(packet); }
DLLCLIENT void NET_cl_start_resource_transfer(NetPacket packet) { client->HandleClientStartResourceTransfer(packet); }

extern ClientEntityNetworkMap *g_ClEntityNetworkMap;

CBaseEntity *NET_cl_ent_create(NetPacket &packet, bool bSpawn, bool bIgnoreMapInit = false)
{
	if(!client->IsGameActive())
		return NULL;
	CGame *game = client->GetGameState();
	unsigned int factoryID = packet->Read<unsigned int>();
	CBaseEntity *(*factory)(unsigned int) = g_ClEntityNetworkMap->GetFactory(factoryID);
	if(factory == NULL) {
		Con::cwar << "Unable to create entity with factory ID '" << factoryID << "': Factory not found!" << Con::endl;
		return NULL;
	}
	unsigned int idx = packet->Read<unsigned int>();
	unsigned int mapIdx = packet->Read<unsigned int>();
	CBaseEntity *ent = factory(idx);
	ent->ReceiveData(packet);
	if(mapIdx == 0) {
		if(bSpawn)
			ent->Spawn();
	}
	else if(bIgnoreMapInit == false && game->IsMapInitialized()) {
		Con::cwar << "Map-entity created after map initialization. Removing..." << Con::endl;
		ent->RemoveSafely();
		return NULL;
	}
	else {
		auto pMapComponent = ent->AddComponent<pragma::MapComponent>();
		if(pMapComponent.valid())
			pMapComponent->SetMapIndex(mapIdx);
	}
	return ent;
}

DLLCLIENT void NET_cl_ent_create(NetPacket packet) { NET_cl_ent_create(packet, true); }

DLLCLIENT void NET_cl_ent_remove(NetPacket packet)
{
	if(!client->IsGameActive())
		return;
	auto *ent = static_cast<CBaseEntity *>(nwm::read_entity(packet));
	if(ent == NULL)
		return;
	ent->Remove();
}

CBaseEntity *NET_cl_ent_create_lua(NetPacket &packet, bool bSpawn, bool bIgnoreMapInit = false)
{
	if(!client->IsGameActive())
		return nullptr;

	CGame *game = client->GetGameState();
	std::string classname = packet->ReadString();
	unsigned int idx = packet->Read<unsigned int>();
	unsigned int mapIdx = packet->Read<unsigned int>();
	CBaseEntity *ent = game->CreateLuaEntity(classname, idx, true);
	if(ent == NULL) {
		Con::cwar << "Attempted to create unregistered entity '" << classname << "'!" << Con::endl;
		return nullptr;
	}
	ent->ReceiveData(packet);

	if(mapIdx == 0) {
		if(bSpawn)
			ent->Spawn();
	}
	else if(bIgnoreMapInit == false && game->IsMapInitialized()) {
		Con::cwar << "Map-entity created after map initialization. Removing..." << Con::endl;
		ent->RemoveSafely();
		return NULL;
	}
	else {
		auto pMapComponent = ent->AddComponent<pragma::MapComponent>();
		if(pMapComponent.valid())
			pMapComponent->SetMapIndex(mapIdx);
	}
	return ent;
}

DLLCLIENT void NET_cl_ent_create_lua(NetPacket packet) { NET_cl_ent_create_lua(packet, true); }

DLLCLIENT void NET_game_timescale(NetPacket packet)
{
	if(!client->IsGameActive())
		return;
	float timeScale = packet->Read<float>();
	CGame *game = client->GetGameState();
	game->SetTimeScale(timeScale);
}

DLLCLIENT void NET_cl_create_giblet(NetPacket packet)
{
	if(!client->IsGameActive())
		return;
	auto info = packet->Read<GibletCreateInfo>();
	c_game->CreateGiblet(info);
}

DLLCLIENT void NET_cl_register_entity_component(NetPacket packet)
{
	if(!client->IsGameActive())
		return;
	auto svComponentId = packet->Read<pragma::ComponentId>();
	auto name = packet->ReadString();
	auto &componentManager = static_cast<pragma::CEntityComponentManager &>(c_game->GetEntityComponentManager());
	auto &svComponentToClComponent = componentManager.GetServerComponentIdToClientComponentIdTable();
	auto clComponentId = componentManager.PreRegisterComponentType(name);
	if(svComponentId >= svComponentToClComponent.size())
		svComponentToClComponent.resize(svComponentId + 1u, pragma::CEntityComponentManager::INVALID_COMPONENT);
	svComponentToClComponent.at(svComponentId) = clComponentId;
}

DLLCLIENT void NET_cl_fire_bullet(NetPacket packet)
{
	if(!client->IsGameActive())
		return;
	auto numTracer = packet->Read<uint8_t>();
	auto tracerSettings = packet->Read<uint8_t>();
	auto trRadius = (tracerSettings & 1) ? packet->Read<float>() : 1.f;
	Color trColor {255, 255, 90, 255};
	if(tracerSettings & 2) {
		trColor.r = packet->Read<uint8_t>();
		trColor.g = packet->Read<uint8_t>();
		trColor.b = packet->Read<uint8_t>();
		trColor.a = packet->Read<uint8_t>();
	}
	auto trLength = (tracerSettings & 4) ? packet->Read<float>() : 200.f;
	auto trSpeed = (tracerSettings & 8) ? packet->Read<float>() : 6'000.f;
	auto trMat = (tracerSettings & 16) ? packet->ReadString() : "particles/beam_tracer";
	auto trBloom = (tracerSettings & 32) ? packet->Read<float>() : 0.25f;

	auto start = packet->Read<Vector3>();
	auto numHits = packet->Read<uint8_t>();
	for(auto i = decltype(numHits) {0}; i < numHits; ++i) {
		auto p = packet->Read<Vector3>();
		auto n = packet->Read<Vector3>();
		auto surfaceMaterialId = packet->Read<int32_t>();
		if(numTracer > 0 && (i % numTracer) == 0)
			c_game->CreateParticleTracer(start, p, trRadius, trColor, trLength, trSpeed, trMat, trBloom);

		auto *surfaceMaterial = (surfaceMaterialId != -1) ? c_game->GetSurfaceMaterial(surfaceMaterialId) : nullptr;
		auto *surfaceMaterialGeneric = c_game->GetSurfaceMaterial(0);
		if(surfaceMaterial != nullptr || surfaceMaterialGeneric != nullptr) {
			auto particleEffect = (surfaceMaterial != nullptr) ? surfaceMaterial->GetImpactParticleEffect() : "";
			if(particleEffect.empty() && surfaceMaterialGeneric != nullptr)
				particleEffect = surfaceMaterialGeneric->GetImpactParticleEffect();
			if(!particleEffect.empty()) {
				auto *pt = pragma::CParticleSystemComponent::Create(particleEffect);
				if(pt != nullptr) {
					auto pTrComponent = pt->GetEntity().GetTransformComponent();
					if(pTrComponent != nullptr) {
						pTrComponent->SetPosition(p);

						auto ang = uvec::to_angle(n);
						auto rot = uquat::create(ang);
						pTrComponent->SetRotation(rot);
					}
					pt->SetRemoveOnComplete(true);
					pt->Start();
				}
			}

			auto sndEffect = (surfaceMaterial != nullptr) ? surfaceMaterial->GetBulletImpactSound() : "";
			if(sndEffect.empty() && surfaceMaterialGeneric != nullptr)
				sndEffect = surfaceMaterialGeneric->GetBulletImpactSound();
			if(!sndEffect.empty()) {
				auto snd = client->CreateSound(sndEffect, ALSoundType::Effect | ALSoundType::Physics, ALCreateFlags::Mono);
				if(snd != nullptr) {
					snd->SetPosition(p);
					snd->Play();
				}
			}
		}
	}
}

DLLCLIENT void NET_cl_ent_sound(NetPacket packet)
{
	if(!client->IsGameActive())
		return;
	auto *ent = static_cast<CBaseEntity *>(nwm::read_entity(packet));
	if(ent == NULL)
		return;
	unsigned int sndID = packet->Read<unsigned int>();
	std::shared_ptr<ALSound> snd = client->GetSoundByIndex(sndID);
	if(snd == NULL)
		return;
	CBaseEntity *cent = static_cast<CBaseEntity *>(ent);
	auto pSoundEmitterComponent = cent->GetComponent<pragma::CSoundEmitterComponent>();
	if(pSoundEmitterComponent.valid())
		pSoundEmitterComponent->AddSound(snd);
}

DLLCLIENT void NET_cl_ent_setunlit(NetPacket packet)
{
	if(!client->IsGameActive())
		return;
	auto *ent = static_cast<CBaseEntity *>(nwm::read_entity(packet));
	if(ent == NULL)
		return;
	auto pRenderComponent = ent->GetRenderComponent();
	if(!pRenderComponent)
		return;
	bool b = packet->Read<bool>();
	pRenderComponent->SetUnlit(b);
}

DLLCLIENT void NET_cl_ent_setcastshadows(NetPacket packet)
{
	if(!client->IsGameActive())
		return;
	auto *ent = static_cast<CBaseEntity *>(nwm::read_entity(packet));
	if(ent == NULL)
		return;
	auto pRenderComponent = ent->GetRenderComponent();
	if(!pRenderComponent)
		return;
	bool b = packet->Read<bool>();
	pRenderComponent->SetCastShadows(b);
}

DLLCLIENT void NET_cl_ent_sethealth(NetPacket packet)
{
	if(!client->IsGameActive())
		return;
	auto *ent = static_cast<CBaseEntity *>(nwm::read_entity(packet));
	if(ent == NULL)
		return;
	auto pHealthComponent = ent->GetComponent<pragma::CHealthComponent>();
	if(pHealthComponent.expired())
		return;
	unsigned short health = packet->Read<unsigned short>();
	pHealthComponent->SetHealth(health);
}

DLLCLIENT void NET_cl_ent_setname(NetPacket packet)
{
	if(!client->IsGameActive())
		return;
	auto *ent = static_cast<CBaseEntity *>(nwm::read_entity(packet));
	if(ent == NULL)
		return;
	auto pNameComponent = ent->GetComponent<pragma::CNameComponent>();
	if(pNameComponent.expired())
		return;
	std::string name = packet->ReadString();
	pNameComponent->SetName(name);
}

DLLCLIENT void NET_cl_ent_model(NetPacket packet)
{
	if(!client->IsGameActive())
		return;
	auto *ent = static_cast<CBaseEntity *>(nwm::read_entity(packet));
	if(ent == NULL)
		return;
	std::string mdl = packet->ReadString();
	CBaseEntity *cent = static_cast<CBaseEntity *>(ent);
	auto mdlComponent = cent->GetModelComponent();
	if(mdlComponent)
		mdlComponent->SetModel(mdl.c_str());
}

DLLCLIENT void NET_cl_ent_skin(NetPacket packet)
{
	if(!client->IsGameActive())
		return;
	auto *ent = static_cast<CBaseEntity *>(nwm::read_entity(packet));
	if(ent == NULL)
		return;
	unsigned int skin = packet->Read<unsigned int>();
	auto mdlComponent = ent->GetModelComponent();
	if(mdlComponent)
		mdlComponent->SetSkin(skin);
}

DLLCLIENT void NET_cl_ent_anim_play(NetPacket packet)
{
	if(!client->IsGameActive())
		return;
	auto *ent = static_cast<CBaseEntity *>(nwm::read_entity(packet));
	if(ent == NULL)
		return;
	int anim = packet->Read<int>();
	auto pAnimComponent = ent->GetAnimatedComponent();
	if(pAnimComponent.valid())
		pAnimComponent->PlayAnimation(anim);
}

DLLCLIENT void NET_cl_ent_anim_gesture_play(NetPacket packet)
{
	if(!client->IsGameActive())
		return;
	auto *ent = static_cast<CBaseEntity *>(nwm::read_entity(packet));
	if(ent == NULL)
		return;
	int slot = packet->Read<int>();
	int animation = packet->Read<int>();
	auto pAnimComponent = ent->GetAnimatedComponent();
	if(pAnimComponent.valid())
		pAnimComponent->PlayLayeredAnimation(slot, animation);
}

DLLCLIENT void NET_cl_ent_anim_gesture_stop(NetPacket packet)
{
	if(!client->IsGameActive())
		return;
	auto *ent = static_cast<CBaseEntity *>(nwm::read_entity(packet));
	if(ent == NULL)
		return;
	int slot = packet->Read<int>();
	auto pAnimComponent = ent->GetAnimatedComponent();
	if(pAnimComponent.valid())
		pAnimComponent->StopLayeredAnimation(slot);
}

DLLCLIENT void NET_cl_ent_setparent(NetPacket packet)
{
	if(!client->IsGameActive())
		return;
	auto *ent = static_cast<CBaseEntity *>(nwm::read_entity(packet));
	if(ent == NULL)
		return;
	BaseEntity *parent = nwm::read_entity(packet);
	if(parent == NULL)
		return;
	auto flags = packet->Read<FAttachmentMode>();
	auto offset = packet->Read<Vector3>();
	auto rot = packet->Read<Quat>();
	auto pAttComponent = ent->GetComponent<pragma::CAttachableComponent>();
	if(pAttComponent.valid()) {
		AttachmentInfo attInfo {};
		attInfo.flags |= flags;
		attInfo.offset = offset;
		attInfo.rotation = rot;
		pAttComponent->AttachToEntity(parent, attInfo);
	}
}

DLLCLIENT void NET_cl_ent_setparentmode(NetPacket packet)
{
	if(!client->IsGameActive())
		return;
	auto *ent = static_cast<CBaseEntity *>(nwm::read_entity(packet));
	if(ent == NULL)
		return;
	auto flags = packet->Read<FAttachmentMode>();
	auto pAttComponent = ent->GetComponent<pragma::CAttachableComponent>();
	if(pAttComponent.valid())
		pAttComponent->SetAttachmentFlags(flags);
}

DLLCLIENT void NET_cl_ent_phys_init(NetPacket packet)
{
	if(!client->IsGameActive())
		return;
	auto *ent = static_cast<CBaseEntity *>(nwm::read_entity(packet));
	if(ent == NULL)
		return;
	auto pPhysComponent = ent->GetPhysicsComponent();
	if(pPhysComponent == nullptr)
		return;
	unsigned int type = packet->Read<unsigned int>();
	pPhysComponent->InitializePhysics(PHYSICSTYPE(type));
}

DLLCLIENT void NET_cl_ent_phys_destroy(NetPacket packet)
{
	if(!client->IsGameActive())
		return;
	auto *ent = static_cast<CBaseEntity *>(nwm::read_entity(packet));
	if(ent == NULL)
		return;
	auto pPhysComponent = ent->GetPhysicsComponent();
	if(pPhysComponent == nullptr)
		return;
	pPhysComponent->DestroyPhysicsObject();
}

void NET_cl_ent_event(NetPacket packet)
{
	if(!client->IsGameActive())
		return;
	packet->SetOffset(packet->GetDataSize() - sizeof(UInt32) - sizeof(unsigned int));
	auto *ent = static_cast<CBaseEntity *>(nwm::read_entity(packet));
	if(ent == nullptr)
		return;
	auto eventId = packet->Read<UInt32>();
	auto localId = c_game->SharedNetEventIdToLocal(eventId);
	if(localId == std::numeric_limits<pragma::NetEventId>::max()) {
		Con::cwar << "Unknown net event with shared id " << eventId << "!" << Con::endl;
		return;
	}
	packet->SetOffset(0);
	ent->ReceiveNetEvent(localId, packet);
}

DLLCLIENT void NET_cl_ent_movetype(NetPacket packet)
{
	if(!client->IsGameActive())
		return;
	auto *ent = static_cast<CBaseEntity *>(nwm::read_entity(packet));
	if(ent == NULL)
		return;
	auto pPhysComponent = ent->GetPhysicsComponent();
	if(pPhysComponent == nullptr)
		return;
	MOVETYPE movetype = MOVETYPE(packet->Read<unsigned char>());
	pPhysComponent->SetMoveType(movetype);
}

DLLCLIENT void NET_cl_pl_toggle_noclip(NetPacket packet)
{
	if(!client->IsGameActive())
		return;
	auto *ent = static_cast<CBaseEntity *>(nwm::read_entity(packet));
	if(ent == NULL)
		return;
	auto bNoclip = packet->Read<bool>();
	auto pPhysComponent = ent->GetPhysicsComponent();
	if(pPhysComponent == nullptr)
		return;
	if(bNoclip == false) {
		pPhysComponent->SetMoveType(MOVETYPE::WALK);
		pPhysComponent->SetCollisionFilterGroup(CollisionMask::Player);
	}
	else {
		pPhysComponent->SetMoveType(MOVETYPE::NOCLIP);
		pPhysComponent->SetCollisionFilterGroup(CollisionMask::NoCollision);
		//pl->SetCollisionsEnabled(false); // Bugged due to CCD
	}
}

DLLCLIENT void NET_cl_ent_collisiontype(NetPacket packet)
{
	if(!client->IsGameActive())
		return;
	auto *ent = static_cast<CBaseEntity *>(nwm::read_entity(packet));
	if(ent == NULL)
		return;
	auto pPhysComponent = ent->GetPhysicsComponent();
	if(pPhysComponent == nullptr)
		return;
	COLLISIONTYPE collisiontype = COLLISIONTYPE(packet->Read<unsigned char>());
	pPhysComponent->SetCollisionType(collisiontype);
}

DLLCLIENT void NET_cl_ent_eyeoffset(NetPacket packet)
{
	if(!client->IsGameActive())
		return;
	auto *ent = static_cast<CBaseEntity *>(nwm::read_entity(packet));
	if(ent == NULL)
		return;
	auto pTrComponent = ent->GetTransformComponent();
	if(pTrComponent == nullptr)
		return;
	Vector3 offset = nwm::read_vector(packet);
	pTrComponent->SetEyeOffset(offset);
}

DLLCLIENT void NET_cl_game_start(NetPacket packet)
{
	float timeScale = packet->Read<float>();
	auto gameMode = packet->ReadString();
	client->StartNewGame(gameMode);
	if(!client->IsGameActive())
		return;
	CGame *game = client->GetGameState();
	game->SetTimeScale(timeScale);
}

DLLCLIENT void NET_cl_game_timescale(NetPacket packet)
{
	if(!client->IsGameActive())
		return;
	float timeScale = packet->Read<float>();
	CGame *game = client->GetGameState();
	game->SetTimeScale(timeScale);
}

// Map has been changed by server
DLLCLIENT void NET_cl_map_ready(NetPacket packet) { client->RequestServerInfo(); }

DLLCLIENT void NET_cl_map_load(NetPacket packet)
{
	if(c_game == nullptr)
		return;
	auto mapName = packet->ReadString();
	auto origin = packet->Read<Vector3>();
	//auto startIdx = packet->Read<uint32_t>();
	//c_game->SetEntityMapIndexStart(startIdx);
	client->ReadEntityData(packet);
	auto bNewWorld = packet->Read<bool>();
	auto r = false;
	if(bNewWorld == false)
		r = c_game->Game::LoadMap(mapName.c_str(), origin);
	else
		r = c_game->LoadMap(mapName.c_str(), origin);
	if(r == false)
		Con::cwar << "Unable to load map '" << mapName << "'! Ignoring..." << Con::endl;
}

DLLCLIENT void NET_cl_pl_local(NetPacket packet)
{
	if(!client->IsGameActive())
		return;
	auto *pl = static_cast<pragma::CPlayerComponent *>(nwm::read_player(packet));
	if(pl == NULL)
		return;
	CGame *game = client->GetGameState();
	game->SetLocalPlayer(pl);
}

DLLCLIENT void NET_cl_game_ready(NetPacket packet)
{
	if(!client->IsGameActive())
		return;
	client->SetGameReady();
}

DLLCLIENT void NET_cl_snapshot(NetPacket packet)
{
	if(!client->IsGameActive())
		return;
	CGame *game = client->GetGameState();
	game->ReceiveSnapshot(packet);
}

DLLCLIENT void NET_cl_cvar_set(NetPacket packet)
{
	std::string cvar = packet->ReadString();
	std::string val = packet->ReadString();
	c_engine->SetReplicatedConVar(cvar, val);
}

DLLCLIENT void NET_cl_luacmd_reg(NetPacket packet)
{
	std::string scmd = packet->ReadString();
	unsigned int id = packet->Read<unsigned int>();
	client->RegisterServerConVar(scmd, id);
}

DLLCLIENT void NET_cl_playerinput(NetPacket packet)
{
	if(packet->GetDataSize() == sizeof(uint8_t)) {
		auto id = packet->Read<uint8_t>();
		c_game->OnReceivedPlayerInputResponse(id);
		return;
	}
	auto *pl = nwm::read_player(packet);
	auto orientation = nwm::read_quat(packet);
	if(pl != nullptr) {
		auto &ent = pl->GetEntity();
		if(ent.IsCharacter())
			ent.GetCharacterComponent()->SetViewOrientation(orientation);
	}
	auto actions = packet->Read<Action>();
	auto bController = packet->Read<bool>();
	if(bController == true) {
		auto actionValues = umath::get_power_of_2_values(umath::to_integral(actions));
		for(auto v : actionValues) {
			auto magnitude = packet->Read<float>();
			if(pl != nullptr)
				pl->SetActionInputAxisMagnitude(static_cast<Action>(v), magnitude);
		}
	}
	if(pl != nullptr)
		pl->SetActionInputs(actions, true);
}

DLLCLIENT void NET_cl_pl_speed_walk(NetPacket packet)
{
	auto *pl = nwm::read_entity(packet);
	if(pl == NULL || pl->IsPlayer() == false)
		return;
	float speed = packet->Read<float>();
	pl->GetPlayerComponent()->SetWalkSpeed(speed);
}

DLLCLIENT void NET_cl_pl_slopelimit(NetPacket packet)
{
	auto *pl = nwm::read_entity(packet);
	if(pl == NULL || pl->IsCharacter() == false)
		return;
	float limit = packet->Read<float>();
	pl->GetCharacterComponent()->SetSlopeLimit(limit);
}

void NET_cl_pl_observermode(NetPacket packet)
{
	auto *pl = nwm::read_entity(packet);
	if(pl == nullptr || pl->IsPlayer() == false)
		return;
	auto mode = packet->Read<UChar>();
	pl->GetPlayerComponent()->SetObserverMode(static_cast<OBSERVERMODE>(mode));
}

DLLCLIENT void NET_cl_pl_stepoffset(NetPacket packet)
{
	auto *pl = nwm::read_entity(packet);
	if(pl == NULL || pl->IsCharacter() == false)
		return;
	float offset = packet->Read<float>();
	pl->GetCharacterComponent()->SetStepOffset(offset);
}

DLLCLIENT void NET_cl_pl_speed_run(NetPacket packet)
{
	auto *pl = nwm::read_entity(packet);
	if(pl == NULL || pl->IsPlayer() == false)
		return;
	float speed = packet->Read<float>();
	pl->GetPlayerComponent()->SetRunSpeed(speed);
}

DLLCLIENT void NET_cl_pl_speed_sprint(NetPacket packet)
{
	auto *pl = nwm::read_entity(packet);
	if(pl == NULL || pl->IsPlayer() == false)
		return;
	float speed = packet->Read<float>();
	pl->GetPlayerComponent()->SetSprintSpeed(speed);
}

DLLCLIENT void NET_cl_pl_speed_crouch_walk(NetPacket packet)
{
	auto *pl = nwm::read_entity(packet);
	if(pl == NULL || pl->IsPlayer() == false)
		return;
	float speed = packet->Read<float>();
	pl->GetPlayerComponent()->SetCrouchedWalkSpeed(speed);
}

DLLCLIENT void NET_cl_pl_height_stand(NetPacket packet)
{
	auto *pl = nwm::read_entity(packet);
	if(pl == NULL || pl->IsPlayer() == false)
		return;
	float height = packet->Read<float>();
	pl->GetPlayerComponent()->SetStandHeight(height);
}

DLLCLIENT void NET_cl_pl_height_crouch(NetPacket packet)
{
	auto *pl = nwm::read_entity(packet);
	if(pl == NULL || pl->IsPlayer() == false)
		return;
	float height = packet->Read<float>();
	pl->GetPlayerComponent()->SetCrouchHeight(height);
}

DLLCLIENT void NET_cl_pl_eyelevel_stand(NetPacket packet)
{
	auto *pl = nwm::read_entity(packet);
	if(pl == NULL || pl->IsPlayer() == false)
		return;
	float eyelevel = packet->Read<float>();
	pl->GetPlayerComponent()->SetStandEyeLevel(eyelevel);
}

DLLCLIENT void NET_cl_pl_eyelevel_crouch(NetPacket packet)
{
	auto *pl = nwm::read_entity(packet);
	if(pl == NULL || pl->IsPlayer() == false)
		return;
	float eyelevel = packet->Read<float>();
	pl->GetPlayerComponent()->SetCrouchEyeLevel(eyelevel);
}

DLLCLIENT void NET_cl_pl_updirection(NetPacket packet)
{
	auto *pl = nwm::read_entity(packet);
	if(pl == NULL || pl->IsCharacter() == false)
		return;
	Vector3 direction = nwm::read_vector(packet);
	pl->GetCharacterComponent()->SetUpDirection(direction);
}

void NET_cl_pl_changedname(NetPacket packet)
{
	auto *pl = nwm::read_player(packet);
	if(pl == nullptr)
		return;
	auto name = packet->ReadString();
	auto nameC = pl->GetEntity().GetNameComponent();
	if(nameC.valid())
		nameC->SetName(name);
}

DLLCLIENT void NET_cl_wep_deploy(NetPacket packet)
{
	auto *wep = nwm::read_entity(packet);
	if(wep == NULL || wep->IsWeapon() == false)
		return;
	auto wepComponent = wep->GetWeaponComponent();
	auto whOwnerComponent = wep->GetComponent<pragma::COwnableComponent>();
	BaseEntity *owner = whOwnerComponent.valid() ? whOwnerComponent->GetOwner() : nullptr;
	if(owner != NULL && owner->IsCharacter())
		owner->GetCharacterComponent()->SetActiveWeapon(wep);
	wepComponent->Deploy();
}

DLLCLIENT void NET_cl_wep_holster(NetPacket packet)
{
	auto *wep = nwm::read_entity(packet);
	if(wep == NULL || wep->IsWeapon() == false)
		return;
	auto wepComponent = wep->GetWeaponComponent();
	auto whOwnerComponent = wep->GetComponent<pragma::COwnableComponent>();
	BaseEntity *owner = whOwnerComponent.valid() ? whOwnerComponent->GetOwner() : nullptr;
	if(owner != NULL && owner->IsCharacter())
		owner->GetCharacterComponent()->SetActiveWeapon(NULL);
	wepComponent->Holster();
}

DLLCLIENT void NET_cl_wep_primaryattack(NetPacket packet)
{
	auto *wep = nwm::read_entity(packet);
	if(wep == NULL || wep->IsWeapon() == false)
		return;
	wep->GetWeaponComponent()->PrimaryAttack();
}

DLLCLIENT void NET_cl_wep_secondaryattack(NetPacket packet)
{
	auto *wep = nwm::read_entity(packet);
	if(wep == NULL || wep->IsWeapon() == false)
		return;
	wep->GetWeaponComponent()->SecondaryAttack();
}

void NET_cl_wep_attack3(NetPacket packet)
{
	auto *wep = nwm::read_entity(packet);
	if(wep == nullptr || wep->IsWeapon() == false)
		return;
	wep->GetWeaponComponent()->TertiaryAttack();
}

void NET_cl_wep_attack4(NetPacket packet)
{
	auto *wep = nwm::read_entity(packet);
	if(wep == nullptr || wep->IsWeapon() == false)
		return;
	wep->GetWeaponComponent()->Attack4();
}

DLLCLIENT void NET_cl_wep_reload(NetPacket packet)
{
	auto *wep = nwm::read_entity(packet);
	if(wep == NULL || wep->IsWeapon() == false)
		return;
	wep->GetWeaponComponent()->Reload();
}

void NET_cl_wep_prim_clip_size(NetPacket packet)
{
	auto *wep = nwm::read_entity(packet);
	if(wep == nullptr || wep->IsWeapon() == false)
		return;
	auto size = packet->Read<UInt16>();
	wep->GetWeaponComponent()->SetPrimaryClipSize(size);
}

void NET_cl_wep_sec_clip_size(NetPacket packet)
{
	auto *wep = nwm::read_entity(packet);
	if(wep == nullptr || wep->IsWeapon() == false)
		return;
	auto size = packet->Read<UInt16>();
	wep->GetWeaponComponent()->SetSecondaryClipSize(size);
}

void NET_cl_wep_prim_max_clip_size(NetPacket packet)
{
	auto *wep = nwm::read_entity(packet);
	if(wep == nullptr || wep->IsWeapon() == false)
		return;
	auto size = packet->Read<UInt16>();
	wep->GetWeaponComponent()->SetMaxPrimaryClipSize(size);
}

void NET_cl_wep_sec_max_clip_size(NetPacket packet)
{
	auto *wep = nwm::read_entity(packet);
	if(wep == nullptr || wep->IsWeapon() == false)
		return;
	auto size = packet->Read<UInt16>();
	wep->GetWeaponComponent()->SetMaxSecondaryClipSize(size);
}

void NET_cl_wep_prim_ammo_type(NetPacket packet)
{
	auto *wep = nwm::read_entity(packet);
	if(wep == nullptr || wep->IsWeapon() == false)
		return;
	auto type = packet->Read<UInt32>();
	wep->GetWeaponComponent()->SetPrimaryAmmoType(type);
}

void NET_cl_wep_sec_ammo_type(NetPacket packet)
{
	auto *wep = nwm::read_entity(packet);
	if(wep == nullptr || wep->IsWeapon() == false)
		return;
	auto type = packet->Read<UInt32>();
	wep->GetWeaponComponent()->SetSecondaryAmmoType(type);
}

DLLCLIENT void NET_cl_gameinfo(NetPacket packet) { client->HandleReceiveGameInfo(packet); }

DLLCLIENT void NET_cl_sv_send(NetPacket packet)
{
	std::string msg = packet->ReadString();
	Con::ccl << "Received message from server: " << msg << Con::endl;
}

DLLCLIENT void NET_cl_env_light_spot_outercutoff_angle(NetPacket packet)
{
	auto *light = nwm::read_entity(packet);
	auto pLightSpotComponent = (light != nullptr) ? light->GetComponent<pragma::CLightSpotComponent>() : pragma::ComponentHandle<pragma::CLightSpotComponent>();
	if(pLightSpotComponent.expired())
		return;
	float cutoffAngle = packet->Read<float>();
	pLightSpotComponent->SetOuterConeAngle(cutoffAngle);
}

DLLCLIENT void NET_cl_env_light_spot_innercutoff_angle(NetPacket packet)
{
	auto *light = nwm::read_entity(packet);
	auto pLightSpotComponent = (light != nullptr) ? light->GetComponent<pragma::CLightSpotComponent>() : pragma::ComponentHandle<pragma::CLightSpotComponent>();
	if(pLightSpotComponent.expired())
		return;
	float blendFraction = packet->Read<float>();
	pLightSpotComponent->SetBlendFraction(blendFraction);
}

DLLCLIENT void NET_cl_envlight_setstate(NetPacket packet)
{
	auto *light = nwm::read_entity(packet);
	auto *pToggleComponent = (light != nullptr) ? static_cast<pragma::BaseToggleComponent *>(light->FindComponent("toggle").get()) : nullptr;
	if(pToggleComponent == NULL)
		return;
	bool b = packet->Read<bool>();
	if(b)
		pToggleComponent->TurnOn();
	else
		pToggleComponent->TurnOff();
}

DLLCLIENT void NET_cl_envexplosion_explode(NetPacket packet)
{
	auto *exp = nwm::read_entity(packet);
	auto *pExplosionComponent = (exp != nullptr) ? static_cast<pragma::CExplosionComponent *>(exp->FindComponent("explosion").get()) : nullptr;
	if(pExplosionComponent == NULL)
		return;
	pExplosionComponent->Explode();
}

DLLCLIENT void NET_cl_ent_toggle(NetPacket packet)
{
	auto *ent = nwm::read_entity(packet);
	if(ent == NULL)
		return;
	bool bOn = packet->Read<bool>();
	auto *pToggleComponent = static_cast<pragma::BaseToggleComponent *>(ent->FindComponent("toggle").get());
	if(pToggleComponent == NULL)
		return;
	if(bOn == true)
		pToggleComponent->TurnOn();
	else
		pToggleComponent->TurnOff();
}

DLLCLIENT void NET_cl_ent_setcollisionfilter(NetPacket packet)
{
	auto *ent = nwm::read_entity(packet);
	if(ent == NULL)
		return;
	auto pPhysComponent = ent->GetPhysicsComponent();
	if(pPhysComponent == nullptr)
		return;
	CollisionMask filterGroup = static_cast<CollisionMask>(packet->Read<unsigned int>());
	CollisionMask filterMask = static_cast<CollisionMask>(packet->Read<unsigned int>());
	pPhysComponent->SetCollisionFilter(filterGroup, filterMask);
}

DLLCLIENT void NET_cl_ent_setkinematic(NetPacket packet)
{
	auto *ent = nwm::read_entity(packet);
	if(ent == NULL)
		return;
	auto pPhysComponent = ent->GetPhysicsComponent();
	if(pPhysComponent == nullptr)
		return;
	bool bKinematic = packet->Read<bool>();
	pPhysComponent->SetKinematic(bKinematic);
}

DLLCLIENT void NET_cl_env_fogcon_setstartdist(NetPacket packet)
{
	auto *fog = nwm::read_entity(packet);
	auto *pFogComponent = (fog != nullptr) ? static_cast<pragma::BaseEnvFogControllerComponent *>(fog->FindComponent("fog_controller").get()) : nullptr;
	if(pFogComponent == NULL)
		return;
	pFogComponent->SetFogStart(packet->Read<float>());
}

DLLCLIENT void NET_cl_env_fogcon_setenddist(NetPacket packet)
{
	auto *fog = nwm::read_entity(packet);
	auto *pFogComponent = (fog != nullptr) ? static_cast<pragma::BaseEnvFogControllerComponent *>(fog->FindComponent("fog_controller").get()) : nullptr;
	if(pFogComponent == NULL)
		return;
	pFogComponent->SetFogEnd(packet->Read<float>());
}

DLLCLIENT void NET_cl_env_fogcon_setmaxdensity(NetPacket packet)
{
	auto *fog = nwm::read_entity(packet);
	auto *pFogComponent = (fog != nullptr) ? static_cast<pragma::BaseEnvFogControllerComponent *>(fog->FindComponent("fog_controller").get()) : nullptr;
	if(pFogComponent == NULL)
		return;
	pFogComponent->SetMaxDensity(packet->Read<float>());
}

DLLCLIENT void NET_cl_env_prtsys_setcontinuous(NetPacket packet)
{
	auto *prt = nwm::read_entity(packet);
	auto *pParticleComponent = (prt != nullptr) ? static_cast<pragma::BaseEnvParticleSystemComponent *>(prt->FindComponent("particle_system").get()) : nullptr;
	if(pParticleComponent == nullptr)
		return;
	auto b = packet->Read<bool>();
	pParticleComponent->SetContinuous(b);
}

void NET_cl_client_dropped(NetPacket packet)
{
	if(!client->IsGameActive())
		return;
	auto *pl = nwm::read_player(packet);
	if(pl == nullptr)
		return;
	auto reason = packet->Read<int32_t>();
	client->GetGameState()->OnPlayerDropped(*pl, static_cast<pragma::networking::DropReason>(reason));
}

void NET_cl_client_ready(NetPacket packet)
{
	if(!client->IsGameActive())
		return;
	auto *pl = nwm::read_player(packet);
	if(pl == nullptr)
		return;
	client->GetGameState()->OnPlayerReady(*pl);
}

void NET_cl_client_joined(NetPacket packet)
{
	if(!client->IsGameActive())
		return;
	auto *pl = nwm::read_player(packet);
	if(pl == nullptr)
		return;
	client->GetGameState()->OnPlayerJoined(*pl);
}

void NET_cl_create_explosion(NetPacket packet)
{
	if(!client->IsGameActive())
		return;
	auto origin = packet->Read<Vector3>();
	auto radius = packet->Read<float>();

	auto *pt = pragma::CParticleSystemComponent::Create("explosion");
	if(pt != nullptr) {
		auto pTrComponent = pt->GetEntity().GetTransformComponent();
		if(pTrComponent != nullptr)
			pTrComponent->SetPosition(origin);
		pt->SetRemoveOnComplete(true);
		pt->Start();
	}

	auto snd = client->CreateSound("fx.explosion", ALSoundType::Effect, ALCreateFlags::Mono);
	if(snd != nullptr) {
		snd->SetPosition(origin);
		snd->SetType(ALSoundType::Effect);
		snd->Play();
	}

	auto *entQuake = c_game->CreateEntity<CEnvQuake>();
	if(entQuake != nullptr) {
		auto *pQuakeComponent = static_cast<pragma::BaseEnvQuakeComponent *>(entQuake->FindComponent("quake").get());
		auto pTrComponent = entQuake->GetTransformComponent();
		if(pTrComponent != nullptr)
			pTrComponent->SetPosition(origin);
		if(pQuakeComponent != nullptr) {
			pQuakeComponent->SetFrequency(50.f);
			pQuakeComponent->SetAmplitude(50.f);
			pQuakeComponent->SetRadius(radius);
		}
		entQuake->SetKeyValue("spawnflags", std::to_string(SF_QUAKE_IN_AIR | SF_QUAKE_REMOVE_ON_COMPLETE));
		entQuake->Spawn();
		auto pInputComponent = entQuake->GetComponent<pragma::CIOComponent>();
		if(pInputComponent.valid())
			pInputComponent->Input("StartShake");
	}
}

void NET_cl_ent_trigger_gravity_onstarttouch(NetPacket packet)
{
	using namespace pragma;
	auto *ent = nwm::read_entity(packet);
	if(ent == nullptr)
		return;
	auto netFlags = static_cast<pragma::Entity::TriggerGravity::NetFlags>(packet->Read<uint8_t>());
	auto flags = packet->Read<uint32_t>();
	auto gravityDir = packet->Read<Vector3>();
	auto gravityForce = packet->Read<float>();
	auto bUseForce = (netFlags & pragma::Entity::TriggerGravity::NetFlags::UseForce) != pragma::Entity::TriggerGravity::NetFlags::None;
	if((netFlags & pragma::Entity::TriggerGravity::NetFlags::StartTouch) != pragma::Entity::TriggerGravity::NetFlags::None)
		pragma::Entity::TriggerGravity::apply_gravity(ent, flags, -gravityDir, gravityDir, bUseForce, gravityForce);
	else {
		auto dirMove = packet->Read<Vector3>();
		pragma::Entity::TriggerGravity::apply_gravity(ent, flags, gravityDir, dirMove, bUseForce, gravityForce);
	}
}

struct NavDebugInfo {
	NavDebugInfo(const std::shared_ptr<DebugRenderer::BaseObject> &path = nullptr, const std::shared_ptr<DebugRenderer::BaseObject> &spline = nullptr, const std::shared_ptr<DebugRenderer::BaseObject> &node = nullptr) : dbgPath(path), dbgSpline(spline), dbgNode(node) {}
	std::shared_ptr<DebugRenderer::BaseObject> dbgPath;
	std::shared_ptr<DebugRenderer::BaseObject> dbgSpline;
	std::shared_ptr<DebugRenderer::BaseObject> dbgNode;
};
static std::unordered_map<const CBaseEntity *, NavDebugInfo> s_aiNavDebugObjects {};
void NET_cl_debug_ai_navigation(NetPacket packet)
{
	if(!client->IsGameActive())
		return;
	auto *npc = static_cast<CBaseEntity *>(nwm::read_entity(packet));
	if(npc == nullptr)
		return;
	std::vector<Vector3> points;
	auto numNodes = packet->Read<uint32_t>();
	auto currentNodeIdx = packet->Read<uint32_t>();
	points.reserve(numNodes * 2 - 2);
	std::shared_ptr<DebugRenderer::BaseObject> dbgNode = nullptr;
	Con::ccl << "New path for " << npc->GetClass() << ":" << Con::endl;
	for(auto nodeId = decltype(numNodes) {0}; nodeId < numNodes; ++nodeId) {
		auto pos = packet->Read<Vector3>() + Vector3(0.f, 20.f, 0.f);
		points.push_back(pos);
		Con::ccl << "#" << (nodeId + 1) << ": " << pos << Con::endl;

		if(nodeId == currentNodeIdx)
			dbgNode = DebugRenderer::DrawLine(pos, pos + Vector3 {0.f, 100.f, 0.f}, Color::Lime);
	}
	Con::ccl << Con::endl;

	auto pGenericComponent = npc->GetComponent<pragma::CGenericComponent>();
	if(pGenericComponent.valid()) {
		pGenericComponent->BindEventUnhandled(BaseEntity::EVENT_ON_REMOVE, [pGenericComponent](std::reference_wrapper<pragma::ComponentEvent> evData) {
			auto it = s_aiNavDebugObjects.find(static_cast<const CBaseEntity *>(&pGenericComponent->GetEntity()));
			if(it == s_aiNavDebugObjects.end())
				return;
			s_aiNavDebugObjects.erase(it);
		});
	}
	s_aiNavDebugObjects[npc] = {DebugRenderer::DrawPath(points, Color::Yellow), DebugRenderer::DrawSpline(points, 10, 1.f, {Color::OrangeRed}), dbgNode};
}

void NET_cl_debug_ai_schedule_print(NetPacket packet)
{
	if(!client->IsGameActive())
		return;
	auto b = packet->Read<bool>();
	if(b == false)
		Con::cout << "> NPC has no active schedule!" << Con::endl;
	else {
		auto msg = packet->ReadString();
		Con::cout << "> Active NPC Schedule:" << Con::endl;
		Con::cout << msg << Con::endl;
	}
}

static std::unique_ptr<DebugGameGUI> dbgAiSchedule = nullptr;
void CMD_debug_ai_schedule(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	dbgAiSchedule = nullptr;
	CHECK_CHEATS("debug_ai_schedule", state, );
	if(c_game == nullptr || pl == nullptr)
		return;
	auto &ent = pl->GetEntity();
	if(ent.IsCharacter() == false)
		return;
	auto charComponent = ent.GetCharacterComponent();
	auto ents = command::find_target_entity(state, *charComponent, argv);
	BaseEntity *npc = nullptr;
	for(auto *ent : ents) {
		if(ent->IsNPC() == false)
			continue;
		npc = ent;
		break;
	}
	if(npc == nullptr) {
		Con::cwar << "No valid NPC target found!" << Con::endl;
		return;
	}
	Con::cout << "Querying schedule data for NPC " << *npc << "..." << Con::endl;
	NetPacket p;
	nwm::write_entity(p, npc);
	client->SendPacket("debug_ai_schedule_tree", p, pragma::networking::Protocol::SlowReliable);
}

void CMD_debug_draw_line(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	if(argv.empty()) {
		Con::cwar << "No position has been specified!" << Con::endl;
		return;
	}
	if(!c_game) {
		Con::cwar << "No active game!" << Con::endl;
		return;
	}
	auto *cam = c_game->GetRenderCamera();
	if(!cam)
		cam = c_game->GetPrimaryCamera();
	if(!cam) {
		Con::cwar << "No active camera found!" << Con::endl;
		return;
	}
	auto srcPos = cam->GetEntity().GetPosition();
	auto tgtPos = uvec::create(argv.front());
	c_game->DrawLine(srcPos, tgtPos, Color::White, 12.f);
	c_game->DrawLine(tgtPos, tgtPos + uvec::RIGHT, Color::Red, 12.f);
	c_game->DrawLine(tgtPos, tgtPos + uvec::UP, Color::Lime, 12.f);
	c_game->DrawLine(tgtPos, tgtPos + uvec::FORWARD, Color::Blue, 12.f);
}

void CMD_debug_aim_info(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	if(pl == nullptr)
		return;
	auto &entPl = pl->GetEntity();
	if(entPl.IsCharacter() == false)
		return;
	auto charComponent = entPl.GetCharacterComponent();
	auto ents = command::find_target_entity(state, *charComponent, argv);
	pragma::CCameraComponent *cam = nullptr;
	if(ents.empty() == false) {
		for(auto *ent : ents) {
			auto sceneC = ent->GetComponent<pragma::CSceneComponent>();
			if(sceneC.expired() || sceneC->GetActiveCamera().expired())
				continue;
			cam = sceneC->GetActiveCamera().get();
			break;
		}
	}
	cam = cam ? cam : c_game->GetPrimaryCamera();
	if(cam == nullptr)
		return;
	auto trC = cam->GetEntity().GetComponent<pragma::CTransformComponent>();
	if(trC.expired())
		return;
	auto trData = util::get_entity_trace_data(*trC);
	trData.SetFlags(RayCastFlags::InvertFilter);
	trData.SetFilter(entPl);

	TraceResult res {};
	EntityIterator entIt {*c_game};
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CRenderComponent>>();
	std::optional<Intersection::LineMeshResult> closestMesh {};
	BaseEntity *entClosest = nullptr;
	for(auto *ent : entIt) {
		if(ent == &entPl)
			continue;
		auto renderC = ent->GetComponent<pragma::CRenderComponent>();
		auto lineMeshResult = renderC->CalcRayIntersection(trData.GetSourceOrigin(), trData.GetTargetOrigin(), true);
		if(lineMeshResult.has_value() == false || lineMeshResult->result != umath::intersection::Result::Intersect)
			continue;
		if(closestMesh.has_value() && lineMeshResult->hitValue > closestMesh->hitValue)
			continue;
		closestMesh = lineMeshResult;
		entClosest = ent;
	}
	if(closestMesh.has_value()) {
		res.hitType = RayCastHitType::Block;
		res.entity = entClosest->GetHandle();
		res.position = closestMesh->hitPos;
		res.normal = {};
		res.distance = uvec::distance(closestMesh->hitPos, trData.GetSourceOrigin());
		if(closestMesh->precise) {
			res.meshInfo = std::make_shared<TraceResult::MeshInfo>();
			res.meshInfo->mesh = closestMesh->precise->mesh.get();
			res.meshInfo->subMesh = closestMesh->precise->subMesh.get();
		}
	}
	else
		res = c_game->RayCast(trData);

	if(res.hitType == RayCastHitType::None) {
		Con::cout << "Nothing found in player aim direction!" << Con::endl;
		return;
	}
	Con::cout << "Hit Entity: ";
	if(res.entity.valid() == false)
		Con::cout << "NULL";
	else
		res.entity->print(Con::cout);
	Con::cout << Con::endl;
	Con::cout << "Hit Position: (" << res.position.x << "," << res.position.y << "," << res.position.z << ")" << Con::endl;
	Con::cout << "Hit Normal: (" << res.normal.x << "," << res.normal.y << "," << res.normal.z << ")" << Con::endl;
	Con::cout << "Hit Distance: " << res.distance << Con::endl;
	std::string mat;
	auto b = res.GetMaterial(mat);
	Con::cout << "Hit Material: " << (b ? mat : "Nothing") << Con::endl;
}

void NET_cl_debug_ai_schedule_tree(NetPacket packet)
{
	if(!client->IsGameActive())
		return;
	const auto bUseGraphicVisualization = true;
	auto updateState = packet->Read<uint8_t>();
	std::shared_ptr<DebugBehaviorTreeNode> dbgTree = nullptr;
	BaseEntity *ent = nullptr;

	static const auto fGetStateInfo = [](const DebugBehaviorTreeNode &node, Color &col, std::string &text) {
		col = Color::White;
		text = node.name + " (";
		switch(node.state) {
		case DebugBehaviorTreeNode::State::Pending:
			//text += "pending";
			col = Color::Aqua;
			break;
		case DebugBehaviorTreeNode::State::Failed:
			//text += "failed";
			col = Color::Maroon;
			break;
		case DebugBehaviorTreeNode::State::Initial:
			//text += "initial";
			col = Color::Gray;
			break;
		default:
			//text += "succeeded";
			col = Color::Lime;
			break;
		}
		//text += ", ";
		switch(node.nodeType) {
		case DebugBehaviorTreeNode::BehaviorNodeType::Selector:
			text += "selector";
			break;
		case DebugBehaviorTreeNode::BehaviorNodeType::Sequence:
			text += "sequence";
			break;
		default:
			text = "unknown";
			break;
		}
		text += ", ";
		switch(node.selectorType) {
		case DebugBehaviorTreeNode::SelectorType::Sequential:
			text += "sequential";
			break;
		case DebugBehaviorTreeNode::SelectorType::RandomShuffle:
			text += "random shuffle";
			break;
		default:
			text = "unknown";
			break;
		}
		text += ")";
	};

	const auto fUpdateGraphicalGUI = [](DebugBehaviorTreeNode &dbgTree, WILuaBase &pElLua) {
		auto *l = c_game->GetLuaState();
		auto t = Lua::CreateTable(l);
		std::function<void(DebugBehaviorTreeNode &)> fPushNode = nullptr;
		fPushNode = [l, &fPushNode](DebugBehaviorTreeNode &node) {
			auto tNode = Lua::CreateTable(l);

			Lua::PushString(l, "name");
			Lua::PushString(l, node.name);
			Lua::SetTableValue(l, tNode);

			Lua::PushString(l, "nodeType");
			Lua::PushInt(l, umath::to_integral(node.nodeType));
			Lua::SetTableValue(l, tNode);

			Lua::PushString(l, "selectorType");
			Lua::PushInt(l, umath::to_integral(node.selectorType));
			Lua::SetTableValue(l, tNode);

			Lua::PushString(l, "lastUpdate");
			Lua::PushNumber(l, node.lastUpdate);
			Lua::SetTableValue(l, tNode);

			Lua::PushString(l, "lastStartTime");
			Lua::PushNumber(l, node.lastStartTime);
			Lua::SetTableValue(l, tNode);

			Lua::PushString(l, "lastEndTime");
			Lua::PushNumber(l, node.lastEndTime);
			Lua::SetTableValue(l, tNode);

			Lua::PushString(l, "executionIndex");
			Lua::PushInt(l, node.executionIndex);
			Lua::SetTableValue(l, tNode);

			Lua::PushString(l, "active");
			Lua::PushBool(l, node.active);
			Lua::SetTableValue(l, tNode);

			Lua::PushString(l, "state");
			Lua::PushInt(l, umath::to_integral(node.state));
			Lua::SetTableValue(l, tNode);

			Lua::PushString(l, "children");
			auto tChildren = Lua::CreateTable(l);
			auto idx = 1;
			for(auto &pChild : node.children) {
				Lua::PushInt(l, idx++);
				fPushNode(*pChild);
				Lua::SetTableValue(l, tChildren);
			}
			Lua::SetTableValue(l, tNode);
		};
		auto o = pElLua.GetLuaObject();
		o.push(l); /* 1 */
		auto tIdx = Lua::GetStackTop(l);
		Lua::PushString(l, "BuildTree"); /* 2 */
		Lua::GetTableValue(l, tIdx);     /* 2 */
		if(Lua::IsFunction(l, -1)) {
			o.push(l);                   /* 3 */
			fPushNode(dbgTree);          /* 4 */
			Lua::ProtectedCall(l, 2, 0); /* 1 */
		}
		else
			Lua::Pop(l, 1); /* 1 */
		Lua::Pop(l, 1);     /* 0 */
	};

	if(dbgAiSchedule != nullptr && bUseGraphicVisualization == true && (updateState == 0 || updateState == 1)) {
		auto *pEl = dbgAiSchedule->GetGUIElement();
		if(pEl != nullptr) {
			auto *pElLua = static_cast<WILuaBase *>(pEl);
			auto *l = c_game->GetLuaState();
			auto o = pElLua->GetLuaObject();
			o.push(l); /* 1 */
			auto tIdx = Lua::GetStackTop(l);
			Lua::PushString(l, "ClearNodes"); /* 2 */
			Lua::GetTableValue(l, tIdx);      /* 2 */
			if(Lua::IsFunction(l, -1)) {
				o.push(l);                   /* 3 */
				Lua::ProtectedCall(l, 1, 0); /* 1 */
			}
			else
				Lua::Pop(l, 1); /* 1 */
			Lua::Pop(l, 1);     /* 0 */
		}
	}

	if(updateState == 0) {
		dbgAiSchedule = nullptr;
		return;
	}
	else if(updateState == 1) // Full update
	{
		dbgAiSchedule = nullptr;
		ent = nwm::read_entity(packet);
		if(ent == nullptr)
			return;
		dbgTree = std::make_shared<DebugBehaviorTreeNode>();
		std::function<void(NetPacket &, DebugBehaviorTreeNode &)> fReadTree = nullptr;
		fReadTree = [&fReadTree](NetPacket &p, DebugBehaviorTreeNode &node) {
			node.name = p->ReadString();
			node.nodeType = static_cast<DebugBehaviorTreeNode::BehaviorNodeType>(p->Read<uint32_t>());
			node.selectorType = static_cast<DebugBehaviorTreeNode::SelectorType>(p->Read<uint32_t>());
			node.lastStartTime = p->Read<float>();
			node.lastEndTime = p->Read<float>();
			node.executionIndex = p->Read<uint64_t>();
			node.active = p->Read<bool>();
			node.state = p->Read<DebugBehaviorTreeNode::State>();
			auto numChildren = p->Read<uint32_t>();
			node.children.reserve(numChildren);
			for(auto i = decltype(numChildren) {0}; i < numChildren; ++i) {
				node.children.push_back(std::make_shared<DebugBehaviorTreeNode>());
				fReadTree(p, *node.children.back());
			}
		};
		fReadTree(packet, *dbgTree);
	}
	else {
		if(dbgAiSchedule == nullptr)
			return;
		auto *pEl = dbgAiSchedule->GetGUIElement();
		if(pEl == nullptr)
			return;
		dbgTree = std::static_pointer_cast<DebugBehaviorTreeNode>(dbgAiSchedule->GetUserData(0));
		auto *hEnt = static_cast<EntityHandle *>(dbgAiSchedule->GetUserData(1).get());
		if(dbgTree == nullptr || hEnt == nullptr || hEnt->valid() == false) {
			dbgAiSchedule = nullptr;
			return;
		}
		ent = hEnt->get();

		std::function<void(NetPacket &, DebugBehaviorTreeNode &, WITreeListElement *)> fReadUpdates = nullptr;
		fReadUpdates = [&fReadUpdates, bUseGraphicVisualization](NetPacket &p, DebugBehaviorTreeNode &node, WITreeListElement *pEl) {
			if(bUseGraphicVisualization == false && pEl == nullptr)
				return;
			auto state = p->Read<DebugBehaviorTreeNode::State>();
			if(state == DebugBehaviorTreeNode::State::Invalid)
				return;
			node.state = state;
			node.active = p->Read<bool>();
			node.lastStartTime = p->Read<float>();
			node.lastEndTime = p->Read<float>();
			node.executionIndex = p->Read<uint64_t>();

			if(bUseGraphicVisualization == false) {
				Color col;
				std::string text;
				fGetStateInfo(node, col, text);
				auto *pText = pEl->GetTextElement();
				if(pText != nullptr) {
					pText->SetColor(col);
					//pText->SetText(text);
				}

				auto &guiChildren = pEl->GetItems();
				for(auto i = decltype(node.children.size()) {0}; i < node.children.size(); ++i) {
					if(i >= guiChildren.size())
						break;
					auto &child = node.children[i];
					auto &guiChild = guiChildren[i];
					fReadUpdates(p, *child, const_cast<WITreeListElement *>(static_cast<const WITreeListElement *>(guiChild.get())));
				}
			}
			else {
				for(auto &child : node.children)
					fReadUpdates(p, *child, nullptr);
			}
		};
		auto *pRoot = (bUseGraphicVisualization == false) ? static_cast<WITreeList *>(pEl)->GetRootItem() : nullptr;
		if(bUseGraphicVisualization == false && pRoot == nullptr)
			return;
		if(bUseGraphicVisualization == false) {
			for(uint8_t i = 0; i < 2; ++i) {
				auto &children = pRoot->GetItems();
				if(children.empty() || children.front().IsValid() == false)
					return;
				pRoot = const_cast<WITreeListElement *>(static_cast<const WITreeListElement *>(children.front().get()));
			}
		}

		fReadUpdates(packet, *dbgTree, pRoot);
		if(bUseGraphicVisualization == true)
			fUpdateGraphicalGUI(*dbgTree, static_cast<WILuaBase &>(*pEl));
		return;
	}

	WIBase *pEl = nullptr;
	if(bUseGraphicVisualization == false) {
		auto *pTreeList = WGUI::GetInstance().Create<WITreeList>();
		pEl = pTreeList;
		if(pTreeList == nullptr)
			return;
		pTreeList->SetVisible(false);
		std::function<void(const DebugBehaviorTreeNode &, WITreeListElement *)> fAddItems = nullptr;
		fAddItems = [&fAddItems](const DebugBehaviorTreeNode &node, WITreeListElement *pEl) {
			Color col;
			std::string text;
			fGetStateInfo(node, col, text);

			auto *pChild = pEl->AddItem(text);
			auto *pText = pChild->GetTextElement();
			if(pText != nullptr) {
				pText->SetColor(col);
				pText->EnableShadow(true);
				pText->SetShadowColor(Color::Black);
				pText->SetShadowOffset(1, 1);
			}
			for(auto &dbgChild : node.children)
				fAddItems(*dbgChild, pChild);
		};
		auto *pRoot = pTreeList->AddItem("");
		fAddItems(*dbgTree, pRoot);
		pTreeList->SetSize(1024, 1024); // TODO: Size to contents
	}
	else {
		auto *pGraphics = dynamic_cast<WILuaBase *>(c_game->CreateGUIElement("WIDebugBehaviorTree"));
		if(pGraphics == nullptr)
			return;
		fUpdateGraphicalGUI(*dbgTree, *pGraphics);
		pEl = pGraphics;
	}

	auto pGenericComponent = ent->GetComponent<pragma::CGenericComponent>();
	CallbackHandle cbOnRemove {};
	if(pGenericComponent.valid()) {
		cbOnRemove = pGenericComponent->BindEventUnhandled(BaseEntity::EVENT_ON_REMOVE, [](std::reference_wrapper<pragma::ComponentEvent> evData) { dbgAiSchedule = nullptr; });
	}

	dbgAiSchedule = std::make_unique<DebugGameGUI>([pEl]() { return pEl->GetHandle(); });
	dbgAiSchedule->CallOnRemove([cbOnRemove]() mutable {
		if(cbOnRemove.IsValid())
			cbOnRemove.Remove();
	});
	dbgAiSchedule->SetUserData(0, dbgTree);
	dbgAiSchedule->SetUserData(1, std::make_shared<EntityHandle>(ent->GetHandle()));
}

void NET_cl_cmd_call_response(NetPacket packet)
{
	auto resultFlags = packet->Read<uint8_t>();
	if(resultFlags == 0)
		Con::cout << "> Serverside command execution has failed." << Con::endl;
	else {
		if(resultFlags == 1)
			Con::cout << "> Serverside command has been executed successfully." << Con::endl;
		else {
			auto val = packet->ReadString();
			Con::cout << "> Serverside command has been executed successfully. New value: " << val << Con::endl;
		}
	}
}

void NET_cl_add_shared_component(NetPacket packet)
{
	auto *ent = nwm::read_entity(packet);
	if(ent == nullptr || c_game == nullptr)
		return;
	auto componentId = packet->Read<pragma::ComponentId>();
	auto &componentManager = static_cast<pragma::CEntityComponentManager &>(c_game->GetEntityComponentManager());
	auto &componentTypes = componentManager.GetRegisteredComponentTypes();
	auto &svComponentToClComponentTable = componentManager.GetServerComponentIdToClientComponentIdTable();
	if(componentId >= svComponentToClComponentTable.size())
		return;
	ent->AddComponent(svComponentToClComponentTable.at(componentId));
}

REGISTER_CONVAR_CALLBACK_CL(debug_ai_navigation, [](NetworkState *state, const ConVar &, bool, bool val) {
	CHECK_CHEATS("debug_ai_navigation", state, );
	if(c_game == nullptr)
		return;
	if(val == false)
		s_aiNavDebugObjects.clear();
	NetPacket p {};
	p->Write<bool>(val);
	client->SendPacket("debug_ai_navigation", p, pragma::networking::Protocol::SlowReliable);
});
