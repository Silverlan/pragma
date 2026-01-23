// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.flashlight;
import :client_state;
import :engine;
import :entities.components.attachment;
import :entities.components.radius;
import :entities.components.sound_emitter;

using namespace pragma;

void CFlashlightComponent::Initialize()
{
	BaseFlashlightComponent::Initialize();
	get_client_state()->PrecacheSound("fx\\flashlight_on.wav");
	get_client_state()->PrecacheSound("fx\\flashlight_off.wav");

	auto &ent = GetEntity();
	BindEvent(cLightComponent::EVENT_SHOULD_PASS_ENTITY, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto pAttComponent = GetEntity().GetComponent<CAttachmentComponent>();
		auto *pParent = pAttComponent.valid() ? pAttComponent->GetParent() : nullptr;
		if(pParent != nullptr && &static_cast<CEShouldPassEntity &>(evData.get()).entity == pParent) {
			static_cast<CEShouldPassEntity &>(evData.get()).shouldPass = false;
			return util::EventReply::Handled;
		}
		return util::EventReply::Unhandled;
	});
}

util::EventReply CFlashlightComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(BaseFlashlightComponent::HandleEvent(eventId, evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	auto &ent = GetEntity();
	if(eventId == baseToggleComponent::EVENT_ON_TURN_ON) {
		auto pSoundEmitterCompnent = ent.GetComponent<CSoundEmitterComponent>();
		if(pSoundEmitterCompnent.valid())
			pSoundEmitterCompnent->EmitSound("fx\\flashlight_on.wav", audio::ALSoundType::Effect, 1.f);
	}
	else if(eventId == baseToggleComponent::EVENT_ON_TURN_OFF) {
		auto pSoundEmitterCompnent = ent.GetComponent<CSoundEmitterComponent>();
		if(pSoundEmitterCompnent.valid())
			pSoundEmitterCompnent->EmitSound("fx\\flashlight_off.wav", audio::ALSoundType::Effect, 1.f);
	}
	return util::EventReply::Unhandled;
}
void CFlashlightComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CFlashlight::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CFlashlightComponent>();
}
