#include "stdafx_shared.h"
#include "pragma/entities/environment/lights/env_light.h"
#include <sharedutils/util.h>
#include "pragma/util/util_handled.hpp"
#include "pragma/entities/components/base_color_component.hpp"
#include "pragma/entities/components/base_radius_component.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include <algorithm>

using namespace pragma;

void BaseEnvLightComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		if(ustring::compare(kvData.key,"distance",false))
			GetEntity().SetKeyValue("radius",kvData.value);
		else if(ustring::compare(kvData.key,"lightcolor",false))
			GetEntity().SetKeyValue("color",kvData.value);
		else if(ustring::compare(kvData.key,"falloff_exponent",false))
			SetFalloffExponent(util::to_float(kvData.value));
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("transform");
	ent.AddComponent("toggle");
	ent.AddComponent("color");
	m_netEvSetShadowType = SetupNetEvent("set_shadow_type");
	m_netEvSetFalloffExponent = SetupNetEvent("set_falloff_exponent");
}
void BaseEnvLightComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	auto flags = GetEntity().GetSpawnFlags();
	if(flags &umath::to_integral(SpawnFlag::DontCastShadows))
		m_shadowType = ShadowType::None;
}
BaseEnvLightComponent::ShadowType BaseEnvLightComponent::GetShadowType() const {return m_shadowType;}
void BaseEnvLightComponent::SetShadowType(ShadowType type) {m_shadowType = type;}
float BaseEnvLightComponent::GetFalloffExponent() const {return m_falloffExponent;}
void BaseEnvLightComponent::SetFalloffExponent(float falloffExponent) {m_falloffExponent = falloffExponent;}
