// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.client;

import :entities.components.bot;
import :client_state;
import :entities.components.animated;
import :entities.components.sound_emitter;

using namespace pragma;

void CBotComponent::Initialize()
{
	BaseBotComponent::Initialize();

	BindEvent(cAnimatedComponent::EVENT_HANDLE_ANIMATION_EVENT, [this](std::reference_wrapper<ComponentEvent> evData) -> pragma::util::EventReply {
		switch(static_cast<CEHandleAnimationEvent &>(evData.get()).animationEvent.eventID) {
		case AnimationEvent::Type::FootstepLeft:
			OnFootStep(BaseCharacterComponent::FootType::Left);
			return pragma::util::EventReply::Handled;
		case AnimationEvent::Type::FootstepRight:
			OnFootStep(BaseCharacterComponent::FootType::Right);
			return pragma::util::EventReply::Handled;
		}
		return pragma::util::EventReply::Unhandled;
	});
}
pragma::util::EventReply CBotComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(BaseBotComponent::HandleEvent(eventId, evData) == pragma::util::EventReply::Handled)
		return pragma::util::EventReply::Handled;
	if(eventId == baseCharacterComponent::EVENT_ON_FOOT_STEP)
		OnFootStep(static_cast<CEOnFootStep &>(evData).footType);
	return pragma::util::EventReply::Unhandled;
}
void CBotComponent::OnFootStep(BaseCharacterComponent::FootType footType)
{
	auto &ent = GetEntity();
	auto pSoundEmitterComponent = ent.GetComponent<pragma::CSoundEmitterComponent>();
	if(pSoundEmitterComponent.expired())
		return;
	auto aiComponent = ent.GetAIComponent();
	auto pVelComponent = ent.GetComponent<pragma::VelocityComponent>();
	auto vel = pVelComponent.valid() ? pVelComponent->GetVelocity() : Vector3 {};
	float speed = uvec::length(vel);
	float scale = 0.f;
	float speedMax = aiComponent.valid() ? aiComponent->GetMaxSpeed() : 0.f;
	if(speedMax > 0.f) {
		scale = speed / speedMax;
		if(scale > 1.f)
			scale = 1.f;
	}
	if(scale == 0.f)
		return;
	pSoundEmitterComponent->EmitSound("fx.fst_concrete", pragma::audio::ALSoundType::Effect, scale);
}
void CBotComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

////////////////

void CBot::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CBotComponent>();
}
