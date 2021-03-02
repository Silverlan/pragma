/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/environment/c_env_fog_controller.h"
#include "pragma/entities/c_entityfactories.h"
#include <pragma/networking/nwm_util.h>
#include "pragma/lua/c_lentity_handles.hpp"
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

extern DLLCLIENT CGame *c_game;

LINK_ENTITY_TO_CLASS(env_fog_controller,CEnvFogController);

void CFogControllerComponent::Initialize()
{
	BaseEnvFogControllerComponent::Initialize();
	BindEventUnhandled(CToggleComponent::EVENT_ON_TURN_ON,[this](std::reference_wrapper<ComponentEvent> evData) {
		GetFog().SetEnabled(true);
	});
	BindEventUnhandled(CToggleComponent::EVENT_ON_TURN_OFF,[this](std::reference_wrapper<ComponentEvent> evData) {
		GetFog().SetEnabled(false);
	});
}
void CFogControllerComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEnvFogControllerComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(CColorComponent))
	{
		static_cast<CColorComponent&>(component).GetColorProperty()->AddCallback([this](std::reference_wrapper<const Color> oldColor,std::reference_wrapper<const Color> color) {
			GetFog().SetColor(color);
		});
	}
}
void CFogControllerComponent::OnEntitySpawn()
{
	BaseEnvFogControllerComponent::OnEntitySpawn();

	auto &ent = GetEntity();
	auto &fog = GetFog();
	fog.SetType(static_cast<WorldEnvironment::Fog::Type>(m_kvFogType));
	auto pColComponent = ent.GetComponent<CColorComponent>();
	if(pColComponent.valid())
		fog.SetColor(pColComponent->GetColor());
	fog.SetStart(m_kvFogStart);
	fog.SetEnd(m_kvFogEnd);
	fog.SetMaxDensity(m_kvMaxDensity);
	auto pToggleComponent = ent.GetComponent<CToggleComponent>();
	if(pToggleComponent.expired() || pToggleComponent->IsTurnedOn())
		fog.SetEnabled(true);
}
void CFogControllerComponent::ReceiveData(NetPacket &packet)
{
	m_kvFogType = packet->Read<unsigned char>();
	m_kvFogStart = packet->Read<float>();
	m_kvFogEnd = packet->Read<float>();
	m_kvMaxDensity = packet->Read<float>();
}
WorldEnvironment::Fog &CFogControllerComponent::GetFog()
{
	auto &worldEnv = c_game->GetWorldEnvironment();
	return worldEnv.GetFogSettings();
}
void CFogControllerComponent::SetFogStart(float start)
{
	BaseEnvFogControllerComponent::SetFogStart(start);
	GetFog().SetStart(start);
}
void CFogControllerComponent::SetFogEnd(float end)
{
	BaseEnvFogControllerComponent::SetFogEnd(end);
	GetFog().SetEnd(end);
}
void CFogControllerComponent::SetMaxDensity(float density)
{
	BaseEnvFogControllerComponent::SetMaxDensity(density);
	GetFog().SetMaxDensity(density);
}
luabind::object CFogControllerComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CFogControllerComponentHandleWrapper>(l);}

/////////////

void CEnvFogController::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CFogControllerComponent>();
}
