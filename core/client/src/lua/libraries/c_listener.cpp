#include "stdafx_client.h"
#include "pragma/lua/libraries/c_listener.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/entities/baseentity_luaobject.h"
#include "pragma/lua/c_lentity_handles.hpp"
#include <alsoundsystem.hpp>
#include <pragma/entities/components/velocity_component.hpp>
#include <pragma/entities/components/base_transform_component.hpp>

using namespace pragma;

extern DLLCENGINE CEngine *c_engine;

LINK_ENTITY_TO_CLASS(listener,CListener);

void CListenerComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEventUnhandled(LogicComponent::EVENT_ON_TICK,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		if(m_listener == nullptr)
			return;
		auto &ent = GetEntity();
		auto pTrComponent = ent.GetTransformComponent();
		auto pVelComponent = ent.GetComponent<pragma::VelocityComponent>();
		if(pTrComponent.valid())
			m_listener->SetPosition(pTrComponent->GetPosition());
		if(pVelComponent.valid())
			m_listener->SetVelocity(pVelComponent->GetVelocity());

		if(pTrComponent.valid())
		{
			Vector3 forward,up;
			pTrComponent->GetOrientation(&forward,nullptr,&up);
			m_listener->SetOrientation(forward,up);
		}
	});

	auto &ent = GetEntity();
	ent.AddComponent<pragma::CTransformComponent>();
	ent.AddComponent<LogicComponent>();
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys == nullptr)
	{
		ent.RemoveSafely();
		return;
	}
	m_listener = &soundSys->GetListener();
}

float CListenerComponent::GetGain()
{
	if(m_listener == nullptr)
		return 0.f;
	return m_listener->GetGain();
}

void CListenerComponent::SetGain(float gain)
{
	if(m_listener == nullptr)
		return;
	m_listener->SetGain(gain);
}
luabind::object CListenerComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CListenerComponentHandleWrapper>(l);}

////////////////

void CListener::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CListenerComponent>();
}
