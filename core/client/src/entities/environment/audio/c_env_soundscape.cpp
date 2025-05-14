/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/audio/alsoundscript.h"
#include "pragma/entities/environment/audio/c_env_soundscape.h"
#include "pragma/entities/c_entityfactories.h"
#include <pragma/networking/nwm_util.h>
#include <pragma/physics/raytraces.h>
#include "pragma/entities/components/c_sound_emitter_component.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/audio/alsound_type.h>
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/entities/components/base_character_component.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

namespace pragma {
	using ::operator<<;
};

CSoundScapeComponent *CSoundScapeComponent::s_active = NULL;

LINK_ENTITY_TO_CLASS(env_soundscape, CEnvSoundScape);

extern DLLCLIENT CGame *c_game;

CSoundScapeComponent::~CSoundScapeComponent()
{
	if(s_active == this)
		s_active = nullptr;
	StopSoundScape();
}

void CSoundScapeComponent::Initialize()
{
	BaseEnvSoundScapeComponent::Initialize();
	SetTickPolicy(TickPolicy::Always); // TODO
}

void CSoundScapeComponent::OnTick(double dt)
{
	if(m_sound.get() == NULL)
		return;
	auto &entThis = GetEntity();
	auto pTrComponent = entThis.GetTransformComponent();
	SetNextTick(c_game->CurTime() + 0.25f);
	if(pTrComponent != nullptr && IsPlayerInRange()) {
		if(s_active != this) {
			auto *pl = c_game->GetLocalPlayer();
			auto &ent = pl->GetEntity();
			auto charComponentEnt = ent.GetCharacterComponent();
			auto pTrComponentEnt = ent.GetTransformComponent();
			if(charComponentEnt.valid() || pTrComponentEnt) {
				TraceData tr;
				tr.SetSource(charComponentEnt.valid() ? charComponentEnt->GetEyePosition() : pTrComponentEnt->GetPosition());
				tr.SetTarget(pTrComponent->GetPosition());
				tr.SetFlags(RayCastFlags::Default | RayCastFlags::IgnoreDynamic);
				auto result = c_game->RayCast(tr);
				if(result.hitType == RayCastHitType::None)
					StartSoundScape();
			}
		}
	}
	if(s_active == this)
		UpdateTargetPositions();
}

void CSoundScapeComponent::OnEntitySpawn()
{
	BaseEnvSoundScapeComponent::OnEntitySpawn();
	for(auto &pair : m_positions) {
		EntityIterator itEnt {*c_game};
		itEnt.AttachFilter<EntityIteratorFilterEntity>(pair.second);
		auto it = itEnt.begin();
		if(it != itEnt.end())
			m_targets.insert(std::unordered_map<unsigned int, EntityHandle>::value_type(pair.first, (*it)->GetHandle()));
	}
	m_sound = nullptr;
	auto &ent = GetEntity();
	auto pSoundEmitterComponent = ent.GetComponent<pragma::CSoundEmitterComponent>();
	if(pSoundEmitterComponent.valid()) {
		std::shared_ptr<ALSound> snd = pSoundEmitterComponent->CreateSound(m_kvSoundScape, ALSoundType::Environment);
		if(snd.get() == NULL) {
			Con::cwar << "Invalid soundscape '" << m_kvSoundScape << "' for entity " << this << Con::endl;
			return;
		}
		ALSoundScript *al = dynamic_cast<ALSoundScript *>(snd.get());
		if(al == NULL) {
			Con::cwar << "Invalid soundscape '" << m_kvSoundScape << "' for entity " << this << Con::endl;
			return;
		}
		snd->SetRelative(true);
		m_sound = snd;
	}
	SetNextTick(c_game->CurTime() + umath::random(0.f, 0.25f)); // Spread out think time between entities
}

void CSoundScapeComponent::ReceiveData(NetPacket &packet)
{
	m_kvSoundScape = packet->ReadString();
	m_kvRadius = packet->Read<float>();
	unsigned int numPositions = packet->Read<unsigned int>();
	for(unsigned int i = 0; i < numPositions; i++) {
		unsigned int id = packet->Read<unsigned int>();
		std::string name = packet->ReadString();
		m_positions.insert(std::unordered_map<unsigned int, std::string>::value_type(id, name));
	}
}

void CSoundScapeComponent::UpdateTargetPositions()
{
	if(m_sound.get() == NULL)
		return;
	ALSoundScript *al = dynamic_cast<ALSoundScript *>(m_sound.get());
	std::unordered_map<unsigned int, EntityHandle>::iterator it;
	for(it = m_targets.begin(); it != m_targets.end(); it++) {
		EntityHandle &hEnt = it->second;
		if(hEnt.valid()) {
			auto pTrComponent = hEnt->GetTransformComponent();
			if(pTrComponent != nullptr)
				al->SetTargetPosition(it->first, pTrComponent->GetPosition());
		}
	}
}

void CSoundScapeComponent::StartSoundScape()
{
	if(s_active == this)
		return;
	if(s_active != NULL) {
		if(s_active->IsPlayerInRange())
			return;
		s_active->StopSoundScape();
	}
	s_active = this;
	m_sound->SetGain(1);
	m_sound->FadeIn(2);
}

void CSoundScapeComponent::StopSoundScape()
{
	if(s_active != this)
		return;
	s_active = NULL;
	m_sound->FadeOut(2);
}

bool CSoundScapeComponent::IsPlayerInRange()
{
	auto *pl = c_game->GetLocalPlayer();
	if(pl == NULL)
		return false;
	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	auto pTrComponentPl = pl->GetEntity().GetTransformComponent();
	if(pTrComponent == nullptr || !pTrComponentPl)
		return false;
	auto &origin = pTrComponent->GetPosition();
	auto &pos = pTrComponentPl->GetPosition();
	return (glm::distance(origin, pos) <= m_kvRadius) ? true : false;
}
void CSoundScapeComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

/////////////

void CEnvSoundScape::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CSoundScapeComponent>();
}
