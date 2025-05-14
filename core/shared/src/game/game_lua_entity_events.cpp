/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include <pragma/game/game.h>
#include "pragma/entities/components/basetoggle.h"
#include "pragma/lua/classes/ldef_vector.h"
#include "pragma/lua/classes/ldef_quaternion.h"
#include "pragma/lua/classes/ldef_entity.h"
#include "pragma/lua/classes/ldef_damageinfo.h"
#include "pragma/lua/classes/ldef_physobj.h"
#include "pragma/lua/classes/lproperty.hpp"
#include "pragma/lua/converters/game_type_converters_t.hpp"
#include "pragma/entities/entity_component_event.hpp"
#include "pragma/lua/classes/ldef_surface_material.h"
#include "pragma/lua/classes/lanimation.h"
#include "pragma/model/animation/play_animation_flags.hpp"
#include "pragma/entities/components/logic_component.hpp"
#include "pragma/entities/components/usable_component.hpp"
#include "pragma/entities/components/base_io_component.hpp"
#include "pragma/entities/components/base_color_component.hpp"
#include "pragma/entities/components/base_animated_component.hpp"
#include "pragma/entities/components/base_sound_emitter_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/entities/components/base_actor_component.hpp"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/base_character_component.hpp"
#include "pragma/entities/trigger/base_trigger_touch.hpp"
#include "pragma/entities/components/damageable_component.hpp"
#include "pragma/entities/components/base_ownable_component.hpp"
#include "pragma/entities/components/base_weapon_component.hpp"
#include "pragma/entities/components/base_health_component.hpp"
#include "pragma/entities/components/base_radius_component.hpp"
#include "pragma/entities/components/base_attachment_component.hpp"
#include "pragma/entities/components/submergible_component.hpp"
#include "pragma/entities/components/liquid/base_liquid_surface_simulation_component.hpp"

bool Game::InjectEntityEvent(pragma::BaseEntityComponent &component, uint32_t eventId, int32_t argsIdx) { return InvokeEntityEvent(component, eventId, argsIdx, true); }
bool Game::BroadcastEntityEvent(pragma::BaseEntityComponent &component, uint32_t eventId, int32_t argsIdx) { return InvokeEntityEvent(component, eventId, argsIdx, false); }
bool Game::InvokeEntityEvent(pragma::BaseEntityComponent &component, uint32_t eventId, int32_t argsIdx, bool bInject)
{
	auto *l = GetLuaState();
	if(eventId == pragma::BaseToggleComponent::EVENT_ON_ENTITY_COMPONENT_ADDED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto &componentHandle = Lua::Check<pragma::BaseEntityComponent>(l, -1);
		Lua::Pop(l, 1);
		pragma::CEOnEntityComponentAdded evData {componentHandle};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::BaseEntityComponent::EVENT_ON_ENTITY_COMPONENT_REMOVED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto &componentHandle = Lua::Check<pragma::BaseEntityComponent>(l, -1);
		Lua::Pop(l, 1);
		pragma::CEOnEntityComponentRemoved evData {componentHandle};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::LogicComponent::EVENT_ON_TICK) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto dt = Lua::CheckNumber(l, -1);
		Lua::Pop(l, 1);
		pragma::CEOnTick evData {dt};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::UsableComponent::EVENT_ON_USE) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto &ent = Lua::Check<BaseEntity>(l, -1);
		Lua::Pop(l, 1);
		pragma::CEOnUseData evData {&ent};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::UsableComponent::EVENT_CAN_USE) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto &ent = Lua::Check<BaseEntity>(l, -1);
		Lua::Pop(l, 1);
		pragma::CECanUseData evData {&ent};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::BaseIOComponent::EVENT_HANDLE_INPUT) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		std::string input = Lua::CheckString(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 2);
		Lua::GetTableValue(l, argsIdx);
		auto &activator = Lua::Check<BaseEntity>(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 3);
		Lua::GetTableValue(l, argsIdx);
		auto &caller = Lua::Check<BaseEntity>(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 4);
		Lua::GetTableValue(l, argsIdx);
		std::string data = Lua::CheckString(l, -1);
		Lua::Pop(l, 1);

		pragma::CEInputData evData {input, &activator, &caller, data};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::BaseColorComponent::EVENT_ON_COLOR_CHANGED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto &oldColor = Lua::Check<Vector4>(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 2);
		Lua::GetTableValue(l, argsIdx);
		auto &color = Lua::Check<Vector4>(l, -1);
		Lua::Pop(l, 1);

		pragma::CEOnColorChanged evData {oldColor, color};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::BaseAnimatedComponent::EVENT_HANDLE_ANIMATION_EVENT) {
		AnimationEvent ev {};

		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		ev.eventID = static_cast<AnimationEvent::Type>(Lua::CheckInt(l, -1));
		Lua::Pop(l, 1);

		Lua::PushInt(l, 2);
		Lua::GetTableValue(l, argsIdx);
		Lua::Animation::GetAnimationEventArguments(l, -1, ev.arguments);
		ev.eventID = static_cast<AnimationEvent::Type>(Lua::CheckInt(l, -1));
		Lua::Pop(l, 1);

		pragma::CEHandleAnimationEvent evData {ev};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::BaseAnimatedComponent::EVENT_ON_PLAY_ANIMATION) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto previousAnimation = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 2);
		Lua::GetTableValue(l, argsIdx);
		auto animation = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 3);
		Lua::GetTableValue(l, argsIdx);
		auto flags = static_cast<pragma::FPlayAnim>(Lua::CheckInt(l, -1));
		Lua::Pop(l, 1);

		pragma::CEOnPlayAnimation evData {static_cast<int32_t>(previousAnimation), static_cast<int32_t>(animation), flags};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::BaseAnimatedComponent::EVENT_ON_PLAY_LAYERED_ANIMATION) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto slot = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 2);
		Lua::GetTableValue(l, argsIdx);
		auto previousAnimation = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 3);
		Lua::GetTableValue(l, argsIdx);
		auto animation = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 4);
		Lua::GetTableValue(l, argsIdx);
		auto flags = static_cast<pragma::FPlayAnim>(Lua::CheckInt(l, -1));
		Lua::Pop(l, 1);

		pragma::CEOnPlayLayeredAnimation evData {static_cast<int32_t>(slot), static_cast<int32_t>(previousAnimation), static_cast<int32_t>(animation), flags};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::BaseAnimatedComponent::EVENT_ON_ANIMATION_COMPLETE) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto animation = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 2);
		Lua::GetTableValue(l, argsIdx);
		auto activity = static_cast<Activity>(Lua::CheckInt(l, -1));
		Lua::Pop(l, 1);

		pragma::CEOnAnimationComplete evData {static_cast<int32_t>(animation), activity};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::BaseAnimatedComponent::EVENT_ON_LAYERED_ANIMATION_START || eventId == pragma::BaseAnimatedComponent::EVENT_ON_LAYERED_ANIMATION_COMPLETE) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto slot = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 2);
		Lua::GetTableValue(l, argsIdx);
		auto animation = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 3);
		Lua::GetTableValue(l, argsIdx);
		auto activity = static_cast<Activity>(Lua::CheckInt(l, -1));
		Lua::Pop(l, 1);

		pragma::CELayeredAnimationInfo evData {static_cast<int32_t>(slot), static_cast<int32_t>(animation), activity};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::BaseAnimatedComponent::EVENT_ON_ANIMATION_START) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto animation = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 2);
		Lua::GetTableValue(l, argsIdx);
		auto activity = static_cast<Activity>(Lua::CheckInt(l, -1));
		Lua::Pop(l, 1);

		Lua::PushInt(l, 3);
		Lua::GetTableValue(l, argsIdx);
		auto flags = static_cast<pragma::FPlayAnim>(Lua::CheckInt(l, -1));
		Lua::Pop(l, 1);

		pragma::CEOnAnimationStart evData {static_cast<int32_t>(animation), activity, flags};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::BaseAnimatedComponent::EVENT_ON_BONE_TRANSFORM_CHANGED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto boneId = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 2);
		Lua::GetTableValue(l, argsIdx);
		auto *pos = Lua::IsSet(l, -1) ? Lua::CheckVector(l, -1) : nullptr;
		Lua::Pop(l, 1);

		Lua::PushInt(l, 3);
		Lua::GetTableValue(l, argsIdx);
		auto *rot = Lua::IsSet(l, -1) ? Lua::CheckQuaternion(l, -1) : nullptr;
		Lua::Pop(l, 1);

		Lua::PushInt(l, 4);
		Lua::GetTableValue(l, argsIdx);
		auto *scale = Lua::IsSet(l, -1) ? Lua::CheckVector(l, -1) : nullptr;
		Lua::Pop(l, 1);

		pragma::CEOnBoneTransformChanged evData {static_cast<uint32_t>(boneId), pos, rot, scale};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::BaseAnimatedComponent::EVENT_ON_PLAY_ACTIVITY) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto activity = static_cast<Activity>(Lua::CheckInt(l, -1));
		Lua::Pop(l, 1);

		Lua::PushInt(l, 2);
		Lua::GetTableValue(l, argsIdx);
		auto flags = static_cast<pragma::FPlayAnim>(Lua::CheckInt(l, -1));
		Lua::Pop(l, 1);

		pragma::CEOnPlayActivity evData {activity, flags};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::BaseAnimatedComponent::EVENT_ON_PLAY_LAYERED_ACTIVITY) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto slot = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 2);
		Lua::GetTableValue(l, argsIdx);
		auto activity = static_cast<Activity>(Lua::CheckInt(l, -1));
		Lua::Pop(l, 1);

		Lua::PushInt(l, 3);
		Lua::GetTableValue(l, argsIdx);
		auto flags = static_cast<pragma::FPlayAnim>(Lua::CheckInt(l, -1));
		Lua::Pop(l, 1);

		pragma::CEOnPlayLayeredActivity evData {static_cast<int32_t>(slot), activity, flags};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::BaseAnimatedComponent::EVENT_MAINTAIN_ANIMATIONS) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto deltaTime = Lua::CheckNumber(l, -1);
		Lua::Pop(l, 1);

		pragma::CEMaintainAnimations evData {deltaTime};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::BaseSoundEmitterComponent::EVENT_ON_SOUND_CREATED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto snd = Lua::Check<std::shared_ptr<ALSound>>(l, -1);
		Lua::Pop(l, 1);

		pragma::CEOnSoundCreated evData {snd};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::BaseModelComponent::EVENT_ON_BODY_GROUP_CHANGED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto groupId = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 2);
		Lua::GetTableValue(l, argsIdx);
		auto typeId = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);

		pragma::CEOnBodyGroupChanged evData {static_cast<uint32_t>(groupId), static_cast<uint32_t>(typeId)};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::BaseModelComponent::EVENT_ON_SKIN_CHANGED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto skinId = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);

		pragma::CEOnSkinChanged evData {static_cast<uint32_t>(skinId)};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::BaseModelComponent::EVENT_ON_MODEL_CHANGED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto &model = Lua::Check<std::shared_ptr<Model>>(l, -1);
		Lua::Pop(l, 1);

		pragma::CEOnModelChanged evData {model};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::BaseModelComponent::EVENT_ON_BODY_GROUP_CHANGED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto groupId = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 2);
		Lua::GetTableValue(l, argsIdx);
		auto typeId = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);

		pragma::CEOnBodyGroupChanged evData {static_cast<uint32_t>(groupId), static_cast<uint32_t>(typeId)};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::BaseModelComponent::EVENT_ON_SKIN_CHANGED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto skinId = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);

		pragma::CEOnSkinChanged evData {static_cast<uint32_t>(skinId)};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::BaseModelComponent::EVENT_ON_MODEL_CHANGED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto model = Lua::Check<std::shared_ptr<Model>>(l, -1);
		Lua::Pop(l, 1);

		pragma::CEOnModelChanged evData {model};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::BaseActorComponent::EVENT_ON_KILLED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto &pDamageInfo = *Lua::CheckDamageInfo(l, -1);
		Lua::Pop(l, 1);
		pragma::CEOnCharacterKilled evData {&pDamageInfo};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::BasePhysicsComponent::EVENT_ON_PHYSICS_UPDATED || eventId == pragma::BasePhysicsComponent::EVENT_ON_DYNAMIC_PHYSICS_UPDATED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto dt = Lua::CheckNumber(l, -1);
		Lua::Pop(l, 1);
		pragma::CEPhysicsUpdateData evData {dt};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::BaseCharacterComponent::EVENT_ON_FOOT_STEP) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto footType = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);
		pragma::CEOnFootStep evData {static_cast<pragma::BaseCharacterComponent::FootType>(footType)};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::BaseCharacterComponent::EVENT_ON_JUMP) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto &velocity = *Lua::CheckVector(l, -1);
		Lua::Pop(l, 1);
		pragma::CEOnJump evData {velocity};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::BaseCharacterComponent::EVENT_PLAY_FOOTSTEP_SOUND) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto footType = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 2);
		Lua::GetTableValue(l, argsIdx);
		auto *surfMat = Lua::CheckSurfaceMaterial(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 3);
		Lua::GetTableValue(l, argsIdx);
		auto scale = Lua::CheckNumber(l, -1);
		Lua::Pop(l, 1);

		pragma::CEPlayFootstepSound evData {static_cast<pragma::BaseCharacterComponent::FootType>(footType), *surfMat, static_cast<float>(scale)};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::BaseCharacterComponent::EVENT_ON_DEPLOY_WEAPON) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto &ent = Lua::Check<BaseEntity>(l, -1);
		Lua::Pop(l, 1);
		pragma::CEOnDeployWeapon evData {ent};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::BaseCharacterComponent::EVENT_ON_SET_ACTIVE_WEAPON) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto &ent = Lua::Check<BaseEntity>(l, -1);
		Lua::Pop(l, 1);
		pragma::CEOnSetActiveWeapon evData {&ent};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::BaseCharacterComponent::EVENT_ON_CHARACTER_ORIENTATION_CHANGED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto &vUp = *Lua::CheckVector(l, -1);
		Lua::Pop(l, 1);
		pragma::CEOnSetCharacterOrientation evData {vUp};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::BaseCharacterComponent::EVENT_IS_MOVING) {
		pragma::CEIsMoving evData {};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::BaseCharacterComponent::EVENT_HANDLE_VIEW_ROTATION) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto &rot = *Lua::CheckQuaternion(l, -1);
		Lua::Pop(l, 1);

		pragma::CEViewRotation evData {rot};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::BaseTouchComponent::EVENT_CAN_TRIGGER) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto &ent = Lua::Check<BaseEntity>(l, -1);
		Lua::Pop(l, 1);

		pragma::CECanTriggerData evData {&ent};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::DamageableComponent::EVENT_ON_TAKE_DAMAGE) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto &damageInfo = *Lua::CheckDamageInfo(l, -1);
		Lua::Pop(l, 1);
		pragma::CEOnTakeDamage evData {damageInfo};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::BaseSoundEmitterComponent::EVENT_ON_SOUND_CREATED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto &sound = Lua::Check<std::shared_ptr<ALSound>>(l, -1);
		Lua::Pop(l, 1);
		pragma::CEOnSoundCreated evData {sound};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::BaseOwnableComponent::EVENT_ON_OWNER_CHANGED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto *ent0 = Lua::IsSet(l, -1) ? &Lua::Check<BaseEntity>(l, -1) : nullptr;
		Lua::Pop(l, 1);

		Lua::PushInt(l, 2);
		Lua::GetTableValue(l, argsIdx);
		auto *ent1 = Lua::IsSet(l, -1) ? &Lua::Check<BaseEntity>(l, -1) : nullptr;
		Lua::Pop(l, 1);

		pragma::CEOnOwnerChanged evData {ent0, ent1};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::BaseWeaponComponent::EVENT_ON_PRIMARY_CLIP_SIZE_CHANGED || eventId == pragma::BaseWeaponComponent::EVENT_ON_SECONDARY_CLIP_SIZE_CHANGED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto oldSize = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 2);
		Lua::GetTableValue(l, argsIdx);
		auto newSize = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);

		pragma::CEOnClipSizeChanged evData {static_cast<uint16_t>(oldSize), static_cast<uint16_t>(newSize)};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::BaseHealthComponent::EVENT_ON_TAKEN_DAMAGE) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto &damageInfo = *Lua::CheckDamageInfo(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 2);
		Lua::GetTableValue(l, argsIdx);
		auto oldHealth = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 3);
		Lua::GetTableValue(l, argsIdx);
		auto newHealth = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);

		pragma::CEOnTakenDamage evData {damageInfo, static_cast<uint16_t>(oldHealth), static_cast<uint16_t>(newHealth)};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::BaseHealthComponent::EVENT_ON_HEALTH_CHANGED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto oldHealth = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 2);
		Lua::GetTableValue(l, argsIdx);
		auto newHealth = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);

		pragma::CEOnHealthChanged evData {static_cast<uint16_t>(oldHealth), static_cast<uint16_t>(newHealth)};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::BaseRadiusComponent::EVENT_ON_RADIUS_CHANGED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto oldRadius = Lua::CheckNumber(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 2);
		Lua::GetTableValue(l, argsIdx);
		auto newRadius = Lua::CheckNumber(l, -1);
		Lua::Pop(l, 1);

		pragma::CEOnRadiusChanged evData {static_cast<float>(oldRadius), static_cast<float>(newRadius)};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::BaseTouchComponent::EVENT_ON_START_TOUCH || eventId == pragma::BaseTouchComponent::EVENT_ON_END_TOUCH || eventId == pragma::BaseTouchComponent::EVENT_ON_TRIGGER) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto &ent = Lua::Check<BaseEntity>(l, -1);
		Lua::Pop(l, 1);

		pragma::CETouchData evData {ent};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::SubmergibleComponent::EVENT_ON_WATER_SUBMERGED || eventId == pragma::SubmergibleComponent::EVENT_ON_WATER_EMERGED || eventId == pragma::SubmergibleComponent::EVENT_ON_WATER_ENTERED || eventId == pragma::SubmergibleComponent::EVENT_ON_WATER_EXITED
	  || eventId == pragma::BaseModelComponent::EVENT_ON_MODEL_CHANGED || eventId == BaseEntity::EVENT_ON_SPAWN || eventId == pragma::BaseActorComponent::EVENT_ON_RESPAWN || eventId == pragma::BasePhysicsComponent::EVENT_ON_PHYSICS_DESTROYED
	  || eventId == pragma::BasePhysicsComponent::EVENT_ON_PHYSICS_INITIALIZED || eventId == pragma::BaseLiquidSurfaceSimulationComponent::EVENT_ON_WATER_SURFACE_SIMULATOR_CHANGED || eventId == pragma::BaseAttachmentComponent::EVENT_ON_ATTACHMENT_UPDATE) {
		if(bInject)
			component.InjectEvent(eventId);
		else
			component.BroadcastEvent(eventId);
	}
	else
		return false;
	return true;
}
