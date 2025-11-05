// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/lua/core.hpp"

module pragma.server;
import :game;

import :ai;
import :entities.components;

bool SGame::InvokeEntityEvent(pragma::BaseEntityComponent &component, uint32_t eventId, int32_t argsIdx, bool bInject)
{
	if(pragma::Game::InvokeEntityEvent(component, eventId, argsIdx, bInject))
		return true;
	auto *l = GetLuaState();
	if(eventId == pragma::sAIComponent::EVENT_ON_PRIMARY_TARGET_CHANGED || eventId == pragma::sAIComponent::EVENT_ON_TARGET_VISIBILITY_LOST || eventId == pragma::sAIComponent::EVENT_ON_TARGET_VISIBILITY_REACQUIRED || eventId == pragma::sAIComponent::EVENT_ON_MEMORY_GAINED
	  || eventId == pragma::sAIComponent::EVENT_ON_MEMORY_LOST) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto &memoryFragment = Lua::Check<pragma::ai::Memory::Fragment>(l, -1);
		Lua::Pop(l, 1);

		pragma::CEMemoryData evData {&memoryFragment};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::sAIComponent::EVENT_ON_NPC_STATE_CHANGED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto oldState = static_cast<NPCSTATE>(Lua::CheckInt(l, -1));
		Lua::Pop(l, 1);

		Lua::PushInt(l, 2);
		Lua::GetTableValue(l, argsIdx);
		auto newState = static_cast<NPCSTATE>(Lua::CheckInt(l, -1));
		Lua::Pop(l, 1);

		pragma::CEOnNPCStateChanged evData {oldState, newState};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::sAIComponent::EVENT_ON_TARGET_ACQUIRED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto &ent = Lua::Check<EntityHandle>(l, -1);
		Lua::Pop(l, 1);
		if(ent.expired())
			return false;

		Lua::PushInt(l, 2);
		Lua::GetTableValue(l, argsIdx);
		auto distance = Lua::CheckNumber(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 3);
		Lua::GetTableValue(l, argsIdx);
		auto isFirstNewTarget = Lua::CheckBool(l, -1);
		Lua::Pop(l, 1);

		pragma::CEOnTargetAcquired evData {ent.get(), static_cast<float>(distance), isFirstNewTarget};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::sAIComponent::EVENT_ON_SUSPICIOUS_SOUND_HEARED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto &sound = Lua::Check<std::shared_ptr<ALSound>>(l, -1);
		Lua::Pop(l, 1);

		pragma::CEOnSuspiciousSoundHeared evData {sound};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::sAIComponent::EVENT_ON_CONTROLLER_ACTION_INPUT) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto action = static_cast<pragma::Action>(Lua::CheckInt(l, -1));
		Lua::Pop(l, 1);

		Lua::PushInt(l, 2);
		Lua::GetTableValue(l, argsIdx);
		auto pressed = Lua::CheckBool(l, -1);
		Lua::Pop(l, 1);

		pragma::CEOnControllerActionInput evData {action, pressed};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::sAIComponent::EVENT_ON_START_CONTROL) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto &hPl = Lua::Check<pragma::SPlayerComponent>(l, -1);
		Lua::Pop(l, 1);

		pragma::CEOnStartControl evData {hPl};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::sAIComponent::EVENT_ON_PATH_NODE_CHANGED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto nodeIndex = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);

		pragma::CEOnPathNodeChanged evData {static_cast<uint32_t>(nodeIndex)};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else if(eventId == pragma::sAIComponent::EVENT_ON_SCHEDULE_COMPLETE || eventId == pragma::sAIComponent::EVENT_ON_SCHEDULE_STARTED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto schedule = Lua::Check<std::shared_ptr<pragma::ai::Schedule>>(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 2);
		Lua::GetTableValue(l, argsIdx);
		auto resultCode = Lua::CheckInt(l, -1);
		Lua::Pop(l, 1);

		pragma::CEOnScheduleStateChanged evData {schedule, static_cast<pragma::ai::BehaviorNode::Result>(resultCode)};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else
		return false;
	return true;
}
