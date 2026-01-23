// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.env_fog_controller;
import :entities.components.color;
import :entities.components.toggle;
import :game;

using namespace pragma;

void CFogControllerComponent::Initialize()
{
	BaseEnvFogControllerComponent::Initialize();
	BindEventUnhandled(cToggleComponent::EVENT_ON_TURN_ON, [this](std::reference_wrapper<ComponentEvent> evData) { GetFog().SetEnabled(true); });
	BindEventUnhandled(cToggleComponent::EVENT_ON_TURN_OFF, [this](std::reference_wrapper<ComponentEvent> evData) { GetFog().SetEnabled(false); });
}
void CFogControllerComponent::OnRemove()
{
	BaseEnvFogControllerComponent::OnRemove();
	auto toggleC = GetEntity().GetComponent<CToggleComponent>();
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
rendering::WorldEnvironment::Fog &CFogControllerComponent::GetFog()
{
	auto &worldEnv = get_cgame()->GetWorldEnvironment();
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
void CFogControllerComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

/////////////

void CEnvFogController::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CFogControllerComponent>();
}
