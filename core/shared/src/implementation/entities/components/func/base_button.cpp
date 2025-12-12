// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.func.base_button;

using namespace pragma;

void BaseFuncButtonComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(pragma::ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> pragma::util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(pragma::string::compare<std::string>(kvData.key, "use_sound", false))
			m_kvUseSound = kvData.value;
		else if(pragma::string::compare<std::string>(kvData.key, "wait", false))
			m_kvWaitTime = pragma::util::to_float(kvData.value);
		else
			return pragma::util::EventReply::Unhandled;
		return pragma::util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("physics");
	auto whRenderComponent = ent.AddComponent("render");
	if(whRenderComponent.valid())
		static_cast<BaseRenderComponent *>(whRenderComponent.get())->SetCastShadows(true);
	ent.AddComponent("io");
	ent.AddComponent("model");
	ent.AddComponent("sound_emitter");
	ent.AddComponent<pragma::UsableComponent>();
}

pragma::util::EventReply BaseFuncButtonComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(BaseEntityComponent::HandleEvent(eventId, evData) == pragma::util::EventReply::Handled)
		return pragma::util::EventReply::Handled;
	if(eventId == usableComponent::EVENT_ON_USE) {
		if(m_useSound != nullptr)
			m_useSound->Play();
		auto &ent = GetEntity();
		if(m_kvWaitTime > 0.f)
			m_tNextUse = CFloat(ent.GetNetworkState()->GetGameState()->CurTime()) + m_kvWaitTime;
		auto *ioComponent = static_cast<pragma::BaseIOComponent *>(GetEntity().FindComponent("io").get());
		if(ioComponent != nullptr)
			ioComponent->TriggerOutput("OnPressed", static_cast<const CEOnUseData &>(evData).entity);
	}
	return pragma::util::EventReply::Unhandled;
}

void BaseFuncButtonComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent != nullptr)
		pPhysComponent->InitializePhysics(pragma::physics::PhysicsType::Static);
	if(!m_kvUseSound.empty()) {
		ent.GetNetworkState()->PrecacheSound(m_kvUseSound);
		m_useSound = nullptr;
		auto pSoundEmitterComponent = static_cast<pragma::BaseSoundEmitterComponent *>(ent.FindComponent("sound_emitter").get());
		if(pSoundEmitterComponent != nullptr)
			m_useSound = pSoundEmitterComponent->CreateSound(m_kvUseSound, pragma::audio::ALSoundType::Effect);
	}
}
