// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.environment.lights.base_spot_vol;

using namespace pragma;

void BaseEnvLightSpotVolComponent::RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = BaseEnvLightSpotVolComponent;
	using TIntensity = float;
	{
		auto memberInfo = create_component_member_info<T, TIntensity, static_cast<void (T::*)(TIntensity)>(&T::SetIntensityFactor), static_cast<TIntensity (T::*)() const>(&T::GetIntensityFactor)>("intensity", 1.f);
		memberInfo.SetMin(0.f);
		memberInfo.SetMax(5.f);
		registerMember(std::move(memberInfo));
	}
}
void BaseEnvLightSpotVolComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(pragma::ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> pragma::util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(pragma::string::compare<std::string>(kvData.key, "cone_height", false))
			GetEntity().SetKeyValue("radius", kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "cone_color", false))
			GetEntity().SetKeyValue("color", kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "cone_start_offset", false))
			m_coneStartOffset = pragma::util::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "spotlight_target", false))
			m_kvSpotlightTargetName = kvData.value;
		else
			return pragma::util::EventReply::Unhandled;
		return pragma::util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("toggle");
	ent.AddComponent("transform");
	ent.AddComponent("color");
	ent.AddComponent("field_angle");
	ent.AddComponent("point_at_target");
	//auto *pRadiusComponent = dynamic_cast<pragma::BaseRadiusComponent*>(ent.AddComponent("radius").get());
	//if(pRadiusComponent != nullptr)
	//	pRadiusComponent->SetRadius(100.f);
	m_netEvSetSpotlightTarget = SetupNetEvent("set_spotlight_target");
}

void BaseEnvLightSpotVolComponent::Save(udm::LinkedPropertyWrapperArg udm)
{
	BaseEntityComponent::Save(udm);
	udm["coneStartOffset"] = m_coneStartOffset;
	udm["spotlightTargetName"] = m_kvSpotlightTargetName;
	pragma::util::write_udm_entity(udm["spotlightTarget"], m_hSpotlightTarget);
}
void BaseEnvLightSpotVolComponent::Load(udm::LinkedPropertyWrapperArg udm, uint32_t version)
{
	BaseEntityComponent::Load(udm, version);
	udm["coneStartOffset"](m_coneStartOffset);
	udm["spotlightTargetName"](m_kvSpotlightTargetName);
	m_hSpotlightTarget = pragma::util::read_udm_entity(*this, udm["spotlightTarget"]);
}

void BaseEnvLightSpotVolComponent::SetIntensityFactor(float intensityFactor) { m_intensityFactor = intensityFactor; }
float BaseEnvLightSpotVolComponent::GetIntensityFactor() const { return m_intensityFactor; }

pragma::ecs::BaseEntity *BaseEnvLightSpotVolComponent::GetSpotlightTarget() const { return const_cast<pragma::ecs::BaseEntity *>(m_hSpotlightTarget.get()); }

void BaseEnvLightSpotVolComponent::SetSpotlightTarget(pragma::ecs::BaseEntity &ent) { m_hSpotlightTarget = ent.GetHandle(); }

void BaseEnvLightSpotVolComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	if(m_kvSpotlightTargetName.empty() == false) {
		pragma::ecs::EntityIterator entIt {*GetEntity().GetNetworkState()->GetGameState(), pragma::ecs::EntityIterator::FilterFlags::Default | pragma::ecs::EntityIterator::FilterFlags::Pending};
		entIt.AttachFilter<EntityIteratorFilterEntity>(m_kvSpotlightTargetName);
		auto it = entIt.begin();
		if(it != entIt.end())
			SetSpotlightTarget(**it);
	}
}

void BaseEnvLightSpotVolComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentAdded(component);
	auto *pRenderComponent = dynamic_cast<pragma::BaseRenderComponent *>(&component);
	if(pRenderComponent != nullptr)
		pRenderComponent->SetCastShadows(false);
}
