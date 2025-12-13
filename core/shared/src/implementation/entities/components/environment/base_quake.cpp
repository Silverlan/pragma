// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.environment.base_quake;

using namespace pragma;

void BaseEnvQuakeComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	auto &ent = GetEntity();
	ent.AddComponent("toggle");
	BindEvent(ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(pragma::string::compare<std::string>(kvData.key, "frequency", false))
			m_frequency = util::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "amplitude", false))
			m_amplitude = util::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "radius", false))
			m_radius = util::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "duration", false))
			m_duration = util::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "fadein", false))
			m_tFadeIn = util::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "fadeout", false))
			m_tFadeOut = util::to_float(kvData.value);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	BindEvent(baseIOComponent::EVENT_HANDLE_INPUT, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &inputData = static_cast<CEInputData &>(evData.get());
		if(pragma::string::compare<std::string>(inputData.input, "startshake", false)) {
			auto whToggleComponent = GetEntity().FindComponent("toggle");
			if(whToggleComponent.valid())
				static_cast<BaseToggleComponent *>(whToggleComponent.get())->TurnOn();
		}
		else if(pragma::string::compare<std::string>(inputData.input, "stopshake", false)) {
			auto whToggleComponent = GetEntity().FindComponent("toggle");
			if(whToggleComponent.valid())
				static_cast<BaseToggleComponent *>(whToggleComponent.get())->TurnOff();
		}
		else if(pragma::string::compare<std::string>(inputData.input, "setamplitude", false))
			m_amplitude = util::to_float(inputData.data);
		else if(pragma::string::compare<std::string>(inputData.input, "setfrequency", false))
			m_frequency = util::to_float(inputData.data);
		else if(pragma::string::compare<std::string>(inputData.input, "setduration", false))
			m_duration = util::to_float(inputData.data);
		else if(pragma::string::compare<std::string>(inputData.input, "setfadeinduration", false))
			m_tFadeIn = util::to_float(inputData.data);
		else if(pragma::string::compare<std::string>(inputData.input, "setfadeoutduration", false))
			m_tFadeOut = util::to_float(inputData.data);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	ent.AddComponent("io");
	ent.AddComponent("transform");
}

util::EventReply BaseEnvQuakeComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(BaseEntityComponent::HandleEvent(eventId, evData) == util::EventReply::Handled)
		return util::EventReply::Handled;

	if(eventId == baseToggleComponent::EVENT_ON_TURN_ON)
		StartShake();
	else if(eventId == baseToggleComponent::EVENT_ON_TURN_OFF)
		StopShake();
	return util::EventReply::Unhandled;
}

Bool BaseEnvQuakeComponent::IsGlobal() const { return (m_quakeFlags & SF_QUAKE_GLOBAL_SHAKE) != 0; }
Bool BaseEnvQuakeComponent::InAir() const { return (m_quakeFlags & SF_QUAKE_IN_AIR) != 0; }
Bool BaseEnvQuakeComponent::ShouldAffectPhyiscs() const { return (m_quakeFlags & SF_QUAKE_AFFECT_PHYSICS) != 0; }
Bool BaseEnvQuakeComponent::ShouldShakeView() const { return !(m_quakeFlags & SF_QUAKE_DONT_SHAKE_VIEW); }
Bool BaseEnvQuakeComponent::ShouldRemoveOnComplete() const { return (m_quakeFlags & SF_QUAKE_REMOVE_ON_COMPLETE) != 0; }

void BaseEnvQuakeComponent::StartShake() {}
void BaseEnvQuakeComponent::StopShake() { m_tStartShake = 0.f; }
Bool BaseEnvQuakeComponent::IsShakeActive() const { return (m_tStartShake == 0.f) ? false : true; }

void BaseEnvQuakeComponent::InitializeQuake(UInt32 flags) { m_quakeFlags = flags; }

Float BaseEnvQuakeComponent::GetFrequency() const { return m_frequency; }
Float BaseEnvQuakeComponent::GetAmplitude() const { return m_amplitude; }
Float BaseEnvQuakeComponent::GetRadius() const { return m_radius; }
Float BaseEnvQuakeComponent::GetDuration() const { return m_duration; }
Float BaseEnvQuakeComponent::GetFadeInDuration() const { return m_tFadeIn; }
Float BaseEnvQuakeComponent::GetFadeOutDuration() const { return m_tFadeOut; }
void BaseEnvQuakeComponent::SetFrequency(Float freq) { m_frequency = freq; }
void BaseEnvQuakeComponent::SetAmplitude(Float amplitude) { m_amplitude = amplitude; }
void BaseEnvQuakeComponent::SetRadius(Float radius) { m_radius = radius; }
void BaseEnvQuakeComponent::SetDuration(Float duration) { m_duration = duration; }
void BaseEnvQuakeComponent::SetFadeInDuration(Float tFadeIn) { m_tFadeIn = tFadeIn; }
void BaseEnvQuakeComponent::SetFadeOutDuration(Float tFadeOut) { m_tFadeOut = tFadeOut; }
