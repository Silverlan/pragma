/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/lua/classes/ldef_entity.h"
#include "pragma/ai/s_factions.h"
#include "luasystem.h"
#include "pragma/ai/ai_schedule.h"
#include "pragma/ai/ai_squad.h"
#include "pragma/entities/player.h"
#include "pragma/entities/components/s_ai_component.hpp"
#include "pragma/entities/components/s_player_component.hpp"
#include "pragma/lua/s_lentity_components.hpp"
#include "pragma/lua/s_lentity_handles.hpp"
#include "pragma/lua/policies/game_object_policy.hpp"
#include "pragma/lua/policies/default_parameter_policy.hpp"
#include <pragma/lua/lua_util_component.hpp>
#include <pragma/model/model.h>
#include <pragma/lua/lentity_components_base_types.hpp>
#include <pragma/lua/lua_entity_component.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/components/base_ai_component.hpp>
#include <pragma/lua/converters/pair_converter_t.hpp>
#include <luabind/copy_policy.hpp>

#include <pragma/lua/ostream_operator_alias.hpp>

namespace Lua {
	namespace NPC {
		namespace Server {
			static Lua::mult<bool, Lua::opt<float>> IsInViewCone(lua_State *l, pragma::SAIComponent &hEnt, BaseEntity &entOther);
			static bool HasPrimaryTarget(lua_State *l, pragma::SAIComponent &hEnt);
		};
	};
};
#ifdef __linux__
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, BaseEntityComponent);
#endif
void Lua::register_sv_ai_component(lua_State *l, luabind::module_ &module)
{
	auto def = pragma::lua::create_entity_component_class<pragma::SAIComponent, pragma::BaseAIComponent>("AIComponent");
	def.def("StartSchedule", &pragma::SAIComponent::StartSchedule);
	def.def("CancelSchedule", &pragma::SAIComponent::CancelSchedule);
	def.def("SetSquad", &pragma::SAIComponent::SetSquad);
	def.def("GetSquadName", &pragma::SAIComponent::GetSquadName);
	def.def("SetRelationship", static_cast<void (pragma::SAIComponent::*)(std::string, DISPOSITION, int32_t)>(&pragma::SAIComponent::SetRelationship));
	def.def("SetRelationship", static_cast<void (pragma::SAIComponent::*)(std::string, DISPOSITION, int32_t)>(&pragma::SAIComponent::SetRelationship), luabind::default_parameter_policy<4, int32_t {0}> {});
	def.def("SetRelationship", static_cast<void (pragma::SAIComponent::*)(BaseEntity *, DISPOSITION, bool, int32_t)>(&pragma::SAIComponent::SetRelationship));
	def.def("SetRelationship", static_cast<void (pragma::SAIComponent::*)(BaseEntity *, DISPOSITION, bool, int32_t)>(&pragma::SAIComponent::SetRelationship), luabind::default_parameter_policy<5, int32_t {0}> {});
	def.def("SetRelationship", static_cast<void (pragma::SAIComponent::*)(BaseEntity *, DISPOSITION, bool, int32_t)>(&pragma::SAIComponent::SetRelationship), luabind::meta::join_t<luabind::default_parameter_policy<4, true>, luabind::default_parameter_policy<5, int32_t {0}>> {});
	def.def("SetRelationship", static_cast<void (pragma::SAIComponent::*)(Faction &, DISPOSITION, int32_t)>(&pragma::SAIComponent::SetRelationship));
	def.def("SetRelationship", static_cast<void (pragma::SAIComponent::*)(Faction &, DISPOSITION, int32_t)>(&pragma::SAIComponent::SetRelationship), luabind::default_parameter_policy<4, int32_t {0}> {});
	def.def("GetMaxViewDistance", &pragma::SAIComponent::GetMaxViewDistance);
	def.def("SetMaxViewDistance", &pragma::SAIComponent::SetMaxViewDistance);
	def.def("GetMaxViewAngle", &pragma::SAIComponent::GetMaxViewAngle);
	def.def("SetMaxViewAngle", &pragma::SAIComponent::SetMaxViewAngle);
	def.def("GetSquad", &pragma::SAIComponent::GetSquad, luabind::copy_policy<0> {});
	def.def("ClearRelationship", static_cast<void (pragma::SAIComponent::*)(BaseEntity *)>(&pragma::SAIComponent::ClearRelationship));
	def.def("ClearRelationship", static_cast<void (pragma::SAIComponent::*)(std::string)>(&pragma::SAIComponent::ClearRelationship));
	def.def("ClearRelationship", static_cast<void (pragma::SAIComponent::*)(Faction &)>(&pragma::SAIComponent::ClearRelationship));
	def.def(
	  "GetDisposition", +[](pragma::SAIComponent &c, BaseEntity *ent) -> std::pair<DISPOSITION, int> {
		  int priority;
		  auto disp = c.GetDisposition(ent, &priority);
		  return {disp, priority};
	  });
	def.def(
	  "GetDisposition", +[](pragma::SAIComponent &c, const std::string &className) -> std::pair<DISPOSITION, int> {
		  int priority;
		  auto disp = c.GetDisposition(className, &priority);
		  return {disp, priority};
	  });
	def.def(
	  "GetDisposition", +[](pragma::SAIComponent &c, Faction &faction) -> std::pair<DISPOSITION, int> {
		  int priority;
		  auto disp = c.GetDisposition(faction, &priority);
		  return {disp, priority};
	  });
	def.def("GetCurrentSchedule", &pragma::SAIComponent::GetCurrentSchedule);
	def.def("GetMemory", static_cast<pragma::ai::Memory::Fragment *(pragma::SAIComponent::*)(BaseEntity *)>(&pragma::SAIComponent::GetMemory));
	def.def("GetMemory", static_cast<pragma::ai::Memory &(pragma::SAIComponent::*)()>(&pragma::SAIComponent::GetMemory));
	def.def("Memorize", static_cast<pragma::ai::Memory::Fragment *(pragma::SAIComponent::*)(BaseEntity *, pragma::ai::Memory::MemoryType, const Vector3 &, const Vector3 &)>(&pragma::SAIComponent::Memorize));
	def.def("Memorize", static_cast<pragma::ai::Memory::Fragment *(pragma::SAIComponent::*)(BaseEntity *, pragma::ai::Memory::MemoryType)>(&pragma::SAIComponent::Memorize));
	def.def("Forget", &pragma::SAIComponent::Forget);
	def.def("ClearMemory", &pragma::SAIComponent::ClearMemory);
	def.def("IsInMemory", &pragma::SAIComponent::IsInMemory);
	def.def("IsInViewCone", &Lua::NPC::Server::IsInViewCone);
	def.def("GetMemoryDuration", &pragma::SAIComponent::GetMemoryDuration);
	def.def("SetMemoryDuration", &pragma::SAIComponent::SetMemoryDuration);
	def.def("CanSee", &pragma::SAIComponent::CanSee);
	def.def("SetHearingStrength", &pragma::SAIComponent::SetHearingStrength);
	def.def("GetHearingStrength", &pragma::SAIComponent::GetHearingStrength);
	def.def("CanHear", &pragma::SAIComponent::CanHear);
	def.def("GetMemoryFragmentCount", &pragma::SAIComponent::GetMemoryFragmentCount);
	def.def(
	  "GetPrimaryTarget", +[](pragma::SAIComponent &c) { return const_cast<pragma::ai::Memory::Fragment *>(c.GetPrimaryTarget()); });
	def.def("HasPrimaryTarget", &Lua::NPC::Server::HasPrimaryTarget);
	def.def("GetNPCState", &pragma::SAIComponent::GetNPCState);
	def.def("SetNPCState", &pragma::SAIComponent::SetNPCState);
	def.def("IsMoving", &pragma::SAIComponent::IsMoving);
	def.def("IsAIEnabled", &pragma::SAIComponent::IsAIEnabled);
	def.def("SetAIEnabled", &pragma::SAIComponent::SetAIEnabled);
	def.def("EnableAI", &pragma::SAIComponent::EnableAI);
	def.def("DisableAI", &pragma::SAIComponent::DisableAI);
	def.def("IsControllable", &pragma::SAIComponent::IsControllable);
	def.def("SetControllable", &pragma::SAIComponent::SetControllable);
	def.def("StartControl", &pragma::SAIComponent::StartControl);
	def.def("EndControl", &pragma::SAIComponent::EndControl);
	def.def("IsControlled", &pragma::SAIComponent::IsControlled);
	def.def("GetController", &pragma::SAIComponent::GetController);
	def.def("IsEnemy", &pragma::SAIComponent::IsEnemy);
	def.def("LockAnimation", &pragma::SAIComponent::LockAnimation);
	def.def("IsAnimationLocked", &pragma::SAIComponent::IsAnimationLocked);
	def.def("TurnStep", static_cast<bool (*)(pragma::SAIComponent &, const Vector3 &, float)>([](pragma::SAIComponent &ai, const Vector3 &target, float turnSpeed) { return ai.TurnStep(target, &turnSpeed); }));
	def.def("TurnStep", static_cast<bool (*)(pragma::SAIComponent &, const Vector3 &)>([](pragma::SAIComponent &ai, const Vector3 &target) { return ai.TurnStep(target); }));
	def.def("GetDistanceToMoveTarget", &pragma::SAIComponent::GetDistanceToMoveTarget);
	def.def("GetMoveTarget", &pragma::SAIComponent::GetMoveTarget, luabind::copy_policy<0> {});
	def.def("MoveTo", &pragma::SAIComponent::MoveTo);
	def.def("MoveTo", static_cast<pragma::BaseAIComponent::MoveResult (*)(pragma::SAIComponent &, const Vector3 &)>([](pragma::SAIComponent &aiComponent, const Vector3 &pos) -> pragma::BaseAIComponent::MoveResult { return aiComponent.MoveTo(pos); }));
	def.def("StopMoving", &pragma::SAIComponent::StopMoving);
	def.def("HasReachedDestination", &pragma::SAIComponent::HasReachedDestination);
	def.def("GetMoveActivity", &pragma::SAIComponent::GetMoveActivity);
	def.def("GetControllerActionInput", &pragma::SAIComponent::GetControllerActionInput);
	def.def("TriggerScheduleInterrupt", &pragma::SAIComponent::TriggerScheduleInterrupt);

	def.def("PlayActivity", &pragma::SAIComponent::PlayActivity);
	def.def("PlayAnimation", static_cast<bool (pragma::SAIComponent::*)(int32_t, const pragma::SAIComponent::AIAnimationInfo &)>(&pragma::SAIComponent::PlayAnimation));

	auto defAIAnimInfo = luabind::class_<pragma::SAIComponent::AIAnimationInfo>("AnimationInfo");
	defAIAnimInfo.def(luabind::constructor<>());
	defAIAnimInfo.property("flags", &pragma::SAIComponent::AIAnimationInfo::GetPlayFlags, &pragma::SAIComponent::AIAnimationInfo::SetPlayFlags);
	defAIAnimInfo.property("playAsSchedule", &pragma::SAIComponent::AIAnimationInfo::ShouldPlayAsSchedule, &pragma::SAIComponent::AIAnimationInfo::SetPlayAsSchedule);
	defAIAnimInfo.def("SetFacePrimaryTarget", static_cast<void (*)(lua_State *, pragma::SAIComponent::AIAnimationInfo &)>([](lua_State *l, pragma::SAIComponent::AIAnimationInfo &info) { info.SetFaceTarget(true); }));
	defAIAnimInfo.def("ClearFaceTarget", static_cast<void (*)(lua_State *, pragma::SAIComponent::AIAnimationInfo &)>([](lua_State *l, pragma::SAIComponent::AIAnimationInfo &info) { info.SetFaceTarget(false); }));
	defAIAnimInfo.def("SetFaceTarget", static_cast<void (pragma::SAIComponent::AIAnimationInfo::*)(const Vector3 &)>(&pragma::SAIComponent::AIAnimationInfo::SetFaceTarget));
	defAIAnimInfo.def("SetFaceTarget", static_cast<void (pragma::SAIComponent::AIAnimationInfo::*)(BaseEntity &)>(&pragma::SAIComponent::AIAnimationInfo::SetFaceTarget));
	def.scope[defAIAnimInfo];

	def.add_static_constant("EVENT_SELECT_SCHEDULE", pragma::SAIComponent::EVENT_SELECT_SCHEDULE);
	def.add_static_constant("EVENT_SELECT_CONTROLLER_SCHEDULE", pragma::SAIComponent::EVENT_SELECT_CONTROLLER_SCHEDULE);
	def.add_static_constant("EVENT_ON_SCHEDULE_COMPLETE", pragma::SAIComponent::EVENT_ON_SCHEDULE_COMPLETE);
	def.add_static_constant("EVENT_ON_PRIMARY_TARGET_CHANGED", pragma::SAIComponent::EVENT_ON_PRIMARY_TARGET_CHANGED);
	def.add_static_constant("EVENT_ON_PATH_CHANGED", pragma::SAIComponent::EVENT_ON_PATH_CHANGED);
	def.add_static_constant("EVENT_ON_NPC_STATE_CHANGED", pragma::SAIComponent::EVENT_ON_NPC_STATE_CHANGED);
	def.add_static_constant("EVENT_ON_TARGET_VISIBILITY_LOST", pragma::SAIComponent::EVENT_ON_TARGET_VISIBILITY_LOST);
	def.add_static_constant("EVENT_ON_TARGET_VISIBILITY_REACQUIRED", pragma::SAIComponent::EVENT_ON_TARGET_VISIBILITY_REACQUIRED);
	def.add_static_constant("EVENT_ON_MEMORY_GAINED", pragma::SAIComponent::EVENT_ON_MEMORY_GAINED);
	def.add_static_constant("EVENT_ON_MEMORY_LOST", pragma::SAIComponent::EVENT_ON_MEMORY_LOST);
	def.add_static_constant("EVENT_ON_TARGET_ACQUIRED", pragma::SAIComponent::EVENT_ON_TARGET_ACQUIRED);
	def.add_static_constant("EVENT_ON_SUSPICIOUS_SOUND_HEARED", pragma::SAIComponent::EVENT_ON_SUSPICIOUS_SOUND_HEARED);
	def.add_static_constant("EVENT_ON_CONTROLLER_ACTION_INPUT", pragma::SAIComponent::EVENT_ON_CONTROLLER_ACTION_INPUT);
	def.add_static_constant("EVENT_ON_START_CONTROL", pragma::SAIComponent::EVENT_ON_START_CONTROL);
	def.add_static_constant("EVENT_ON_END_CONTROL", pragma::SAIComponent::EVENT_ON_END_CONTROL);
	def.add_static_constant("EVENT_ON_PATH_NODE_CHANGED", pragma::SAIComponent::EVENT_ON_PATH_NODE_CHANGED);
	def.add_static_constant("EVENT_ON_LOOK_TARGET_CHANGED", pragma::SAIComponent::EVENT_ON_LOOK_TARGET_CHANGED);
	def.add_static_constant("EVENT_ON_SCHEDULE_STARTED", pragma::SAIComponent::EVENT_ON_SCHEDULE_STARTED);
	module[def];
}
Lua::mult<bool, Lua::opt<float>> Lua::NPC::Server::IsInViewCone(lua_State *l, pragma::SAIComponent &hEnt, BaseEntity &entOther)
{
	auto dist = 0.f;
	auto r = hEnt.IsInViewCone(&entOther, &dist);
	if(r == false)
		return luabind::object {l, r};
	return {l, r, Lua::opt<float> {luabind::object {l, dist}}};
}
bool Lua::NPC::Server::HasPrimaryTarget(lua_State *l, pragma::SAIComponent &hEnt) { return hEnt.GetPrimaryTarget() != nullptr; }
