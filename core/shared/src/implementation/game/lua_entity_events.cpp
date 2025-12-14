// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :game.game;

bool pragma::Game::InjectEntityEvent(BaseEntityComponent &component, uint32_t eventId, int32_t argsIdx) { return InvokeEntityEvent(component, eventId, argsIdx, true); }
bool pragma::Game::BroadcastEntityEvent(BaseEntityComponent &component, uint32_t eventId, int32_t argsIdx) { return InvokeEntityEvent(component, eventId, argsIdx, false); }
bool pragma::Game::InvokeEntityEvent(BaseEntityComponent &component, uint32_t eventId, int32_t argsIdx, bool bInject)
{
	auto *l = GetLuaState();
	if(eventId == baseEntityComponent::EVENT_ON_ENTITY_COMPONENT_ADDED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto &componentHandle = Lua::Check<BaseEntityComponent>(l, -1);
		Lua::Pop(l, 1);
		CEOnEntityComponentAdded evData {componentHandle};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == baseEntityComponent::EVENT_ON_ENTITY_COMPONENT_REMOVED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto &componentHandle = Lua::Check<BaseEntityComponent>(l, -1);
		Lua::Pop(l, 1);
		CEOnEntityComponentRemoved evData {componentHandle};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == logicComponent::EVENT_ON_TICK) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto dt = Lua::CheckNumber(l, -1);
		Lua::Pop(l, 1);
		CEOnTick evData {dt};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == usableComponent::EVENT_ON_USE) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto &ent = Lua::Check<ecs::BaseEntity>(l, -1);
		Lua::Pop(l, 1);
		CEOnUseData evData {&ent};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == usableComponent::EVENT_CAN_USE) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto &ent = Lua::Check<ecs::BaseEntity>(l, -1);
		Lua::Pop(l, 1);
		CECanUseData evData {&ent};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == baseIOComponent::EVENT_HANDLE_INPUT) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		std::string input = Lua::CheckString(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 2);
		Lua::GetTableValue(l, argsIdx);
		auto &activator = Lua::Check<ecs::BaseEntity>(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 3);
		Lua::GetTableValue(l, argsIdx);
		auto &caller = Lua::Check<ecs::BaseEntity>(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 4);
		Lua::GetTableValue(l, argsIdx);
		std::string data = Lua::CheckString(l, -1);
		Lua::Pop(l, 1);

		CEInputData evData {input, &activator, &caller, data};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == baseColorComponent::EVENT_ON_COLOR_CHANGED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto &oldColor = Lua::Check<Vector4>(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 2);
		Lua::GetTableValue(l, argsIdx);
		auto &color = Lua::Check<Vector4>(l, -1);
		Lua::Pop(l, 1);

		CEOnColorChanged evData {oldColor, color};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == baseAnimatedComponent::EVENT_HANDLE_ANIMATION_EVENT) {
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

		CEHandleAnimationEvent evData {ev};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == baseAnimatedComponent::EVENT_ON_PLAY_ANIMATION) {
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
		auto flags = static_cast<FPlayAnim>(Lua::CheckInt(l, -1));
		Lua::Pop(l, 1);

		CEOnPlayAnimation evData {static_cast<int32_t>(previousAnimation), static_cast<int32_t>(animation), flags};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == baseAnimatedComponent::EVENT_ON_PLAY_LAYERED_ANIMATION) {
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
		auto flags = static_cast<FPlayAnim>(Lua::CheckInt(l, -1));
		Lua::Pop(l, 1);

		CEOnPlayLayeredAnimation evData {static_cast<int32_t>(slot), static_cast<int32_t>(previousAnimation), static_cast<int32_t>(animation), flags};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == baseAnimatedComponent::EVENT_ON_ANIMATION_COMPLETE) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto animation = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 2);
		Lua::GetTableValue(l, argsIdx);
		auto activity = static_cast<Activity>(Lua::CheckInt(l, -1));
		Lua::Pop(l, 1);

		CEOnAnimationComplete evData {static_cast<int32_t>(animation), activity};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == baseAnimatedComponent::EVENT_ON_LAYERED_ANIMATION_START || eventId == baseAnimatedComponent::EVENT_ON_LAYERED_ANIMATION_COMPLETE) {
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

		CELayeredAnimationInfo evData {static_cast<int32_t>(slot), static_cast<int32_t>(animation), activity};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == baseAnimatedComponent::EVENT_ON_ANIMATION_START) {
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
		auto flags = static_cast<FPlayAnim>(Lua::CheckInt(l, -1));
		Lua::Pop(l, 1);

		CEOnAnimationStart evData {static_cast<int32_t>(animation), activity, flags};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == baseAnimatedComponent::EVENT_ON_BONE_TRANSFORM_CHANGED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto boneId = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 2);
		Lua::GetTableValue(l, argsIdx);
		auto *pos = Lua::IsSet(l, -1) ? &Lua::Check<Vector3>(l, -1) : nullptr;
		Lua::Pop(l, 1);

		Lua::PushInt(l, 3);
		Lua::GetTableValue(l, argsIdx);
		auto *rot = Lua::IsSet(l, -1) ? &Lua::Check<Quat>(l, -1) : nullptr;
		Lua::Pop(l, 1);

		Lua::PushInt(l, 4);
		Lua::GetTableValue(l, argsIdx);
		auto *scale = Lua::IsSet(l, -1) ? &Lua::Check<Vector3>(l, -1) : nullptr;
		Lua::Pop(l, 1);

		CEOnBoneTransformChanged evData {static_cast<uint32_t>(boneId), pos, rot, scale};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == baseAnimatedComponent::EVENT_ON_PLAY_ACTIVITY) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto activity = static_cast<Activity>(Lua::CheckInt(l, -1));
		Lua::Pop(l, 1);

		Lua::PushInt(l, 2);
		Lua::GetTableValue(l, argsIdx);
		auto flags = static_cast<FPlayAnim>(Lua::CheckInt(l, -1));
		Lua::Pop(l, 1);

		CEOnPlayActivity evData {activity, flags};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == baseAnimatedComponent::EVENT_ON_PLAY_LAYERED_ACTIVITY) {
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
		auto flags = static_cast<FPlayAnim>(Lua::CheckInt(l, -1));
		Lua::Pop(l, 1);

		CEOnPlayLayeredActivity evData {static_cast<int32_t>(slot), activity, flags};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == baseAnimatedComponent::EVENT_MAINTAIN_ANIMATIONS) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto deltaTime = Lua::CheckNumber(l, -1);
		Lua::Pop(l, 1);

		CEMaintainAnimations evData {deltaTime};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == baseSoundEmitterComponent::EVENT_ON_SOUND_CREATED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto snd = Lua::Check<std::shared_ptr<audio::ALSound>>(l, -1);
		Lua::Pop(l, 1);

		CEOnSoundCreated evData {snd};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == baseModelComponent::EVENT_ON_BODY_GROUP_CHANGED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto groupId = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 2);
		Lua::GetTableValue(l, argsIdx);
		auto typeId = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);

		CEOnBodyGroupChanged evData {static_cast<uint32_t>(groupId), static_cast<uint32_t>(typeId)};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == baseModelComponent::EVENT_ON_SKIN_CHANGED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto skinId = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);

		CEOnSkinChanged evData {static_cast<uint32_t>(skinId)};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == baseModelComponent::EVENT_ON_MODEL_CHANGED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto &model = Lua::Check<std::shared_ptr<asset::Model>>(l, -1);
		Lua::Pop(l, 1);

		CEOnModelChanged evData {model};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == baseModelComponent::EVENT_ON_BODY_GROUP_CHANGED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto groupId = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 2);
		Lua::GetTableValue(l, argsIdx);
		auto typeId = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);

		CEOnBodyGroupChanged evData {static_cast<uint32_t>(groupId), static_cast<uint32_t>(typeId)};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == baseModelComponent::EVENT_ON_SKIN_CHANGED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto skinId = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);

		CEOnSkinChanged evData {static_cast<uint32_t>(skinId)};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == baseModelComponent::EVENT_ON_MODEL_CHANGED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto model = Lua::Check<std::shared_ptr<asset::Model>>(l, -1);
		Lua::Pop(l, 1);

		CEOnModelChanged evData {model};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == baseActorComponent::EVENT_ON_KILLED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto &pDamageInfo = Lua::Check<game::DamageInfo>(l, -1);
		Lua::Pop(l, 1);
		CEOnCharacterKilled evData {&pDamageInfo};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == basePhysicsComponent::EVENT_ON_PHYSICS_UPDATED || eventId == basePhysicsComponent::EVENT_ON_DYNAMIC_PHYSICS_UPDATED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto dt = Lua::CheckNumber(l, -1);
		Lua::Pop(l, 1);
		CEPhysicsUpdateData evData {dt};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == baseCharacterComponent::EVENT_ON_FOOT_STEP) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto footType = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);
		CEOnFootStep evData {static_cast<BaseCharacterComponent::FootType>(footType)};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == baseCharacterComponent::EVENT_ON_JUMP) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto &velocity = Lua::Check<Vector3>(l, -1);
		Lua::Pop(l, 1);
		CEOnJump evData {velocity};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == baseCharacterComponent::EVENT_PLAY_FOOTSTEP_SOUND) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto footType = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 2);
		Lua::GetTableValue(l, argsIdx);
		auto &surfMat = Lua::Check<physics::SurfaceMaterial>(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 3);
		Lua::GetTableValue(l, argsIdx);
		auto scale = Lua::CheckNumber(l, -1);
		Lua::Pop(l, 1);

		CEPlayFootstepSound evData {static_cast<BaseCharacterComponent::FootType>(footType), surfMat, static_cast<float>(scale)};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == baseCharacterComponent::EVENT_ON_DEPLOY_WEAPON) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto &ent = Lua::Check<ecs::BaseEntity>(l, -1);
		Lua::Pop(l, 1);
		CEOnDeployWeapon evData {ent};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == baseCharacterComponent::EVENT_ON_SET_ACTIVE_WEAPON) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto &ent = Lua::Check<ecs::BaseEntity>(l, -1);
		Lua::Pop(l, 1);
		CEOnSetActiveWeapon evData {&ent};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == baseCharacterComponent::EVENT_ON_CHARACTER_ORIENTATION_CHANGED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto &vUp = Lua::Check<Vector3>(l, -1);
		Lua::Pop(l, 1);
		CEOnSetCharacterOrientation evData {vUp};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == baseCharacterComponent::EVENT_IS_MOVING) {
		CEIsMoving evData {};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == baseCharacterComponent::EVENT_HANDLE_VIEW_ROTATION) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto &rot = Lua::Check<Quat>(l, -1);
		Lua::Pop(l, 1);

		CEViewRotation evData {rot};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == baseTouchComponent::EVENT_CAN_TRIGGER) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto &ent = Lua::Check<ecs::BaseEntity>(l, -1);
		Lua::Pop(l, 1);

		CECanTriggerData evData {&ent};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == damageableComponent::EVENT_ON_TAKE_DAMAGE) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto &damageInfo = Lua::Check<game::DamageInfo>(l, -1);
		Lua::Pop(l, 1);
		CEOnTakeDamage evData {damageInfo};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == baseSoundEmitterComponent::EVENT_ON_SOUND_CREATED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto &sound = Lua::Check<std::shared_ptr<audio::ALSound>>(l, -1);
		Lua::Pop(l, 1);
		CEOnSoundCreated evData {sound};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == baseOwnableComponent::EVENT_ON_OWNER_CHANGED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto *ent0 = Lua::IsSet(l, -1) ? &Lua::Check<ecs::BaseEntity>(l, -1) : nullptr;
		Lua::Pop(l, 1);

		Lua::PushInt(l, 2);
		Lua::GetTableValue(l, argsIdx);
		auto *ent1 = Lua::IsSet(l, -1) ? &Lua::Check<ecs::BaseEntity>(l, -1) : nullptr;
		Lua::Pop(l, 1);

		CEOnOwnerChanged evData {ent0, ent1};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == baseWeaponComponent::EVENT_ON_PRIMARY_CLIP_SIZE_CHANGED || eventId == baseWeaponComponent::EVENT_ON_SECONDARY_CLIP_SIZE_CHANGED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto oldSize = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 2);
		Lua::GetTableValue(l, argsIdx);
		auto newSize = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);

		CEOnClipSizeChanged evData {static_cast<uint16_t>(oldSize), static_cast<uint16_t>(newSize)};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == baseHealthComponent::EVENT_ON_TAKEN_DAMAGE) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto &damageInfo = Lua::Check<game::DamageInfo>(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 2);
		Lua::GetTableValue(l, argsIdx);
		auto oldHealth = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 3);
		Lua::GetTableValue(l, argsIdx);
		auto newHealth = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);

		CEOnTakenDamage evData {damageInfo, static_cast<uint16_t>(oldHealth), static_cast<uint16_t>(newHealth)};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == baseHealthComponent::EVENT_ON_HEALTH_CHANGED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto oldHealth = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 2);
		Lua::GetTableValue(l, argsIdx);
		auto newHealth = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);

		CEOnHealthChanged evData {static_cast<uint16_t>(oldHealth), static_cast<uint16_t>(newHealth)};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == baseRadiusComponent::EVENT_ON_RADIUS_CHANGED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto oldRadius = Lua::CheckNumber(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 2);
		Lua::GetTableValue(l, argsIdx);
		auto newRadius = Lua::CheckNumber(l, -1);
		Lua::Pop(l, 1);

		CEOnRadiusChanged evData {static_cast<float>(oldRadius), static_cast<float>(newRadius)};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == baseTouchComponent::EVENT_ON_START_TOUCH || eventId == baseTouchComponent::EVENT_ON_END_TOUCH || eventId == baseTouchComponent::EVENT_ON_TRIGGER) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto &ent = Lua::Check<ecs::BaseEntity>(l, -1);
		Lua::Pop(l, 1);

		CETouchData evData {ent};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == submergibleComponent::EVENT_ON_WATER_SUBMERGED || eventId == submergibleComponent::EVENT_ON_WATER_EMERGED || eventId == submergibleComponent::EVENT_ON_WATER_ENTERED || eventId == submergibleComponent::EVENT_ON_WATER_EXITED
	  || eventId == baseModelComponent::EVENT_ON_MODEL_CHANGED || eventId == ecs::baseEntity::EVENT_ON_SPAWN || eventId == baseActorComponent::EVENT_ON_RESPAWN || eventId == basePhysicsComponent::EVENT_ON_PHYSICS_DESTROYED
	  || eventId == basePhysicsComponent::EVENT_ON_PHYSICS_INITIALIZED || eventId == baseLiquidSurfaceSimulationComponent::EVENT_ON_WATER_SURFACE_SIMULATOR_CHANGED || eventId == baseAttachmentComponent::EVENT_ON_ATTACHMENT_UPDATE) {
		if(bInject)
			component.InjectEvent(eventId);
		else
			component.BroadcastEvent(eventId);
	}
	else
		return false;
	return true;
}
