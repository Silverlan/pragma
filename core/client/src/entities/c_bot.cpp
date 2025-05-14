/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/c_bot.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/entities/components/c_sound_emitter_component.hpp"
#include "pragma/entities/components/c_animated_component.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/audio/alsound_type.h>
#include <pragma/entities/components/velocity_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(bot, CBot);

extern ClientState *client;

void CBotComponent::Initialize()
{
	BaseBotComponent::Initialize();

	BindEvent(CAnimatedComponent::EVENT_HANDLE_ANIMATION_EVENT, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		switch(static_cast<CEHandleAnimationEvent &>(evData.get()).animationEvent.eventID) {
		case AnimationEvent::Type::FootstepLeft:
			OnFootStep(BaseCharacterComponent::FootType::Left);
			return util::EventReply::Handled;
		case AnimationEvent::Type::FootstepRight:
			OnFootStep(BaseCharacterComponent::FootType::Right);
			return util::EventReply::Handled;
		}
		return util::EventReply::Unhandled;
	});
}
util::EventReply CBotComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(BaseBotComponent::HandleEvent(eventId, evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(eventId == BaseCharacterComponent::EVENT_ON_FOOT_STEP)
		OnFootStep(static_cast<CEOnFootStep &>(evData).footType);
	return util::EventReply::Unhandled;
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
	pSoundEmitterComponent->EmitSound("fx.fst_concrete", ALSoundType::Effect, scale);
}
void CBotComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

////////////////

void CBot::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CBotComponent>();
}
