/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/environment/c_env_fog_controller.h"
#include "pragma/entities/components/c_toggle_component.hpp"
#include "pragma/entities/components/c_color_component.hpp"
#include "pragma/entities/c_entityfactories.h"
#include <pragma/networking/nwm_util.h>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

extern DLLCLIENT CGame *c_game;

LINK_ENTITY_TO_CLASS(env_fog_controller, CEnvFogController);

void CFogControllerComponent::Initialize()
{
	BaseEnvFogControllerComponent::Initialize();
	BindEventUnhandled(CToggleComponent::EVENT_ON_TURN_ON, [this](std::reference_wrapper<ComponentEvent> evData) { GetFog().SetEnabled(true); });
	BindEventUnhandled(CToggleComponent::EVENT_ON_TURN_OFF, [this](std::reference_wrapper<ComponentEvent> evData) { GetFog().SetEnabled(false); });
}
void CFogControllerComponent::OnRemove()
{
	BaseEnvFogControllerComponent::OnRemove();
	auto toggleC = GetEntity().GetComponent<pragma::CToggleComponent>();
	if(toggleC.expired() || toggleC->IsTurnedOn())
		GetFog().SetEnabled(false);
}
void CFogControllerComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEnvFogControllerComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(CColorComponent)) {
		static_cast<CColorComponent &>(component).GetColorProperty()->AddCallback([this](std::reference_wrapper<const Vector4> oldColor, std::reference_wrapper<const Vector4> color) { GetFog().SetColor(Color {color}); });
	}
}
void CFogControllerComponent::OnEntitySpawn()
{
	BaseEnvFogControllerComponent::OnEntitySpawn();

	auto &ent = GetEntity();
	auto &fog = GetFog();
	fog.SetType(m_kvFogType);
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
	m_kvFogType = packet->Read<util::FogType>();
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
void CFogControllerComponent::SetFogType(util::FogType type)
{
	BaseEnvFogControllerComponent::SetFogType(type);
	GetFog().SetType(type);
}
void CFogControllerComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

/////////////

void CEnvFogController::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CFogControllerComponent>();
}
