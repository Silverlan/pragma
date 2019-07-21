#include "stdafx_client.h"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/audio/alsoundscript.h"
#include "pragma/entities/environment/audio/c_env_soundscape.h"
#include "pragma/entities/c_entityfactories.h"
#include <pragma/networking/nwm_util.h>
#include <pragma/physics/raytraces.h>
#include "pragma/entities/components/c_sound_emitter_component.hpp"
#include "pragma/lua/c_lentity_handles.hpp"
#include <pragma/audio/alsound_type.h>
#include <pragma/entities/components/logic_component.hpp>
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

namespace pragma
{
	using ::operator<<;
};

CSoundScapeComponent *CSoundScapeComponent::s_active = NULL;

LINK_ENTITY_TO_CLASS(env_soundscape,CEnvSoundScape);

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

	BindEventUnhandled(LogicComponent::EVENT_ON_TICK,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		if(m_sound.get() == NULL)
			return;
		auto &entThis = GetEntity();
		auto pTrComponent = entThis.GetTransformComponent();
		auto pLogicComponent = entThis.GetComponent<pragma::LogicComponent>();
		if(pLogicComponent.valid())
			pLogicComponent->SetNextThink(c_game->CurTime() +0.25f);
		if(pTrComponent.valid() && IsPlayerInRange())
		{
			if(s_active != this)
			{
				auto *pl = c_game->GetLocalPlayer();
				auto &ent = pl->GetEntity();
				auto charComponentEnt = ent.GetCharacterComponent();
				auto pTrComponentEnt = ent.GetTransformComponent();
				if(charComponentEnt.valid() || pTrComponentEnt.valid())
				{
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
	});

	auto &ent = GetEntity();
	ent.AddComponent<pragma::LogicComponent>();
}

void CSoundScapeComponent::OnEntitySpawn()
{
	BaseEnvSoundScapeComponent::OnEntitySpawn();
	for(auto &pair : m_positions)
	{
		EntityIterator itEnt{*c_game};
		itEnt.AttachFilter<EntityIteratorFilterEntity>(pair.second);
		auto it = itEnt.begin();
		if(it != itEnt.end())
			m_targets.insert(std::unordered_map<unsigned int,EntityHandle>::value_type(pair.first,(*it)->GetHandle()));
	}
	m_sound = nullptr;
	auto &ent = GetEntity();
	auto pSoundEmitterComponent = ent.GetComponent<pragma::CSoundEmitterComponent>();
	if(pSoundEmitterComponent.valid())
	{
		std::shared_ptr<ALSound> snd = pSoundEmitterComponent->CreateSound(m_kvSoundScape,ALSoundType::Environment);
		if(snd.get() == NULL)
		{
			Con::cwar<<"WARNING: Invalid soundscape '"<<m_kvSoundScape<<"' for entity "<<this<<Con::endl;
			return;
		}
		ALSoundScript *al = dynamic_cast<ALSoundScript*>(snd.get());
		if(al == NULL)
		{
			Con::cwar<<"WARNING: Invalid soundscape '"<<m_kvSoundScape<<"' for entity "<<this<<Con::endl;
			return;
		}
		snd->SetRelative(true);
		m_sound = snd;
	}
	auto pLogicComponent = ent.GetComponent<pragma::LogicComponent>();
	if(pLogicComponent.valid())
		pLogicComponent->SetNextThink(c_game->CurTime() +umath::random(0.f,0.25f)); // Spread out think time between entities
}

void CSoundScapeComponent::ReceiveData(NetPacket &packet)
{
	m_kvSoundScape = packet->ReadString();
	m_kvRadius = packet->Read<float>();
	unsigned int numPositions = packet->Read<unsigned int>();
	for(unsigned int i=0;i<numPositions;i++)
	{
		unsigned int id = packet->Read<unsigned int>();
		std::string name = packet->ReadString();
		m_positions.insert(std::unordered_map<unsigned int,std::string>::value_type(id,name));
	}
}

void CSoundScapeComponent::UpdateTargetPositions()
{
	if(m_sound.get() == NULL)
		return;
	ALSoundScript *al = dynamic_cast<ALSoundScript*>(m_sound.get());
	std::unordered_map<unsigned int,EntityHandle>::iterator it;
	for(it=m_targets.begin();it!=m_targets.end();it++)
	{
		EntityHandle &hEnt = it->second;
		if(hEnt.IsValid())
		{
			auto pTrComponent = hEnt->GetTransformComponent();
			if(pTrComponent.valid())
				al->SetTargetPosition(it->first,pTrComponent->GetPosition());
		}
	}
}

void CSoundScapeComponent::StartSoundScape()
{
	if(s_active == this)
		return;
	if(s_active != NULL)
	{
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
	if(pTrComponent.expired() || pTrComponentPl.expired())
		return false;
	auto &origin = pTrComponent->GetPosition();
	auto &pos = pTrComponentPl->GetPosition();
	return (glm::distance(origin,pos) <= m_kvRadius) ? true : false;
}
luabind::object CSoundScapeComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CSoundScapeComponentHandleWrapper>(l);}

/////////////

void CEnvSoundScape::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CSoundScapeComponent>();
}
