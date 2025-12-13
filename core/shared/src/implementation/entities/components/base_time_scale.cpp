// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.base_time_scale;

using namespace pragma;

void BaseTimeScaleComponent::RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = BaseTimeScaleComponent;

	using TTimeScale = float;
	{
		auto memberInfo = create_component_member_info<T, TTimeScale, static_cast<void (T::*)(TTimeScale)>(&T::SetTimeScale), static_cast<TTimeScale (T::*)() const>(&T::GetTimeScale)>("timeScale", 1.f);
		memberInfo.SetMin(0.f);
		registerMember(std::move(memberInfo));
	}
}
BaseTimeScaleComponent::BaseTimeScaleComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent), m_timeScale {util::FloatProperty::Create(1.f)} {}
void BaseTimeScaleComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(pragma::string::compare<std::string>(kvData.key, "time_scale", false))
			*m_timeScale = util::to_float(kvData.value);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	BindEvent(baseIOComponent::EVENT_HANDLE_INPUT, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &inputData = static_cast<CEInputData &>(evData.get());
		if(pragma::string::compare<std::string>(inputData.input, "settimescale", false))
			*m_timeScale = util::to_float(inputData.data);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});

	m_netEvSetTimeScale = SetupNetEvent("set_time_scale");
}
void BaseTimeScaleComponent::SetTimeScale(float timeScale) { *m_timeScale = timeScale; }
float BaseTimeScaleComponent::GetTimeScale() const { return *m_timeScale; }
const util::PFloatProperty &BaseTimeScaleComponent::GetTimeScaleProperty() const { return m_timeScale; }
float BaseTimeScaleComponent::GetEffectiveTimeScale() const { return GetEntity().GetNetworkState()->GetGameState()->GetTimeScale() * GetTimeScale(); }
void BaseTimeScaleComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentAdded(component);
	auto *pAnimatedComponent = dynamic_cast<BaseAnimatedComponent *>(&component);
	if(pAnimatedComponent != nullptr) {
		FlagCallbackForRemoval(pAnimatedComponent->GetPlaybackRateProperty()->AddModifier([this](std::reference_wrapper<float> val) { val.get() *= GetEffectiveTimeScale(); }), CallbackType::Component, &component);
	}
}
void BaseTimeScaleComponent::Save(udm::LinkedPropertyWrapperArg udm)
{
	BaseEntityComponent::Save(udm);
	udm["timeScale"] = **m_timeScale;
}
void BaseTimeScaleComponent::Load(udm::LinkedPropertyWrapperArg udm, uint32_t version)
{
	BaseEntityComponent::Load(udm, version);
	auto timeScale = GetTimeScale();
	udm["timeScale"](timeScale);
	SetTimeScale(timeScale);
}
