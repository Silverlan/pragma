// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include <cassert>

module pragma.shared;

import :model.animation.enums;

#ifdef WINDOWS_CLANG_COMPILER_FIX
std::unordered_map<int32_t, std::string> &GET_ACTIVITY_NAMES()
{
  static std::unordered_map<int32_t, std::string> names {{pragma::math::to_integral(pragma::Activity::Invalid), "ACT_INVALID"},

    {pragma::math::to_integral(pragma::Activity::Idle), "ACT_IDLE"}, {pragma::math::to_integral(pragma::Activity::Walk), "ACT_WALK"}, {pragma::math::to_integral(pragma::Activity::Run), "ACT_RUN"}, {pragma::math::to_integral(pragma::Activity::Jump), "ACT_JUMP"},
    {pragma::math::to_integral(pragma::Activity::Crouch), "ACT_CROUCH"}, {pragma::math::to_integral(pragma::Activity::CrouchIdle), "ACT_CROUCH_IDLE"}, {pragma::math::to_integral(pragma::Activity::CrouchWalk), "ACT_CROUCH_WALK"}, {pragma::math::to_integral(pragma::Activity::Uncrouch), "ACT_UNCROUCH"},
    {pragma::math::to_integral(pragma::Activity::Alert), "ACT_ALERT"},

    {pragma::math::to_integral(pragma::Activity::FlinchGeneric), "ACT_FLINCH_GENERIC"}, {pragma::math::to_integral(pragma::Activity::FlinchHead), "ACT_FLINCH_HEAD"}, {pragma::math::to_integral(pragma::Activity::FlinchChest), "ACT_FLINCH_CHEST"},
    {pragma::math::to_integral(pragma::Activity::FlinchStomach), "ACT_FLINCH_STOMACH"}, {pragma::math::to_integral(pragma::Activity::FlinchLeftArm), "ACT_FLINCH_LEFT_ARM"}, {pragma::math::to_integral(pragma::Activity::FlinchRightArm), "ACT_FLINCH_RIGHT_ARM"},
    {pragma::math::to_integral(pragma::Activity::FlinchLeftLeg), "ACT_FLINCH_LEFT_LEG"}, {pragma::math::to_integral(pragma::Activity::FlinchRightLeg), "ACT_FLINCH_RIGHT_LEG"}, {pragma::math::to_integral(pragma::Activity::FlinchPhysics), "ACT_FLINCH_PHYSICS"},
    {pragma::math::to_integral(pragma::Activity::FlinchLight), "ACT_FLINCH_LIGHT"}, {pragma::math::to_integral(pragma::Activity::FlinchMedium), "ACT_FLINCH_MEDIUM"}, {pragma::math::to_integral(pragma::Activity::FlinchHeavy), "ACT_FLINCH_HEAVY"},

    {pragma::math::to_integral(pragma::Activity::Die), "ACT_DIE"}, {pragma::math::to_integral(pragma::Activity::TurnLeft), "ACT_TURN_LEFT"}, {pragma::math::to_integral(pragma::Activity::TurnRight), "ACT_TURN_RIGHT"},

    {pragma::math::to_integral(pragma::Activity::GestureTurnLeft), "ACT_GESTURE_TURN_LEFT"}, {pragma::math::to_integral(pragma::Activity::GestureTurnRight), "ACT_GESTURE_TURN_RIGHT"},

    {pragma::math::to_integral(pragma::Activity::MeleeAttack1), "ACT_MELEE_ATTACK1"}, {pragma::math::to_integral(pragma::Activity::MeleeAttack2), "ACT_MELEE_ATTACK2"}, {pragma::math::to_integral(pragma::Activity::MeleeAttack3), "ACT_MELEE_ATTACK3"},
    {pragma::math::to_integral(pragma::Activity::MeleeAttack4), "ACT_MELEE_ATTACK4"}, {pragma::math::to_integral(pragma::Activity::MeleeAttack5), "ACT_MELEE_ATTACK5"}, {pragma::math::to_integral(pragma::Activity::RangeAttack1), "ACT_RANGE_ATTACK1"},
    {pragma::math::to_integral(pragma::Activity::RangeAttack2), "ACT_RANGE_ATTACK2"}, {pragma::math::to_integral(pragma::Activity::RangeAttack3), "ACT_RANGE_ATTACK3"}, {pragma::math::to_integral(pragma::Activity::RangeAttack4), "ACT_RANGE_ATTACK4"},
    {pragma::math::to_integral(pragma::Activity::RangeAttack5), "ACT_RANGE_ATTACK5"},

    {pragma::math::to_integral(pragma::Activity::VmIdle), "ACT_VM_IDLE"}, {pragma::math::to_integral(pragma::Activity::VmDeploy), "ACT_VM_DEPLOY"}, {pragma::math::to_integral(pragma::Activity::VmHolster), "ACT_VM_HOLSTER"},

    {pragma::math::to_integral(pragma::Activity::VmPrimaryAttack), "ACT_VM_PRIMARY_ATTACK"}, {pragma::math::to_integral(pragma::Activity::VmSecondaryAttack), "ACT_VM_SECONDARY_ATTACK"}, {pragma::math::to_integral(pragma::Activity::VmTertiaryAttack), "ACT_VM_TERTIARY_ATTACK"},
    {pragma::math::to_integral(pragma::Activity::VmAttack4), "ACT_VM_ATTACK4"}, {pragma::math::to_integral(pragma::Activity::VmAttack5), "ACT_VM_ATTACK5"}, {pragma::math::to_integral(pragma::Activity::VmAttack6), "ACT_VM_ATTACK6"}, {pragma::math::to_integral(pragma::Activity::VmAttack7), "ACT_VM_ATTACK7"},
    {pragma::math::to_integral(pragma::Activity::VmAttack8), "ACT_VM_ATTACK8"}, {pragma::math::to_integral(pragma::Activity::VmAttack9), "ACT_VM_ATTACK9"}, {pragma::math::to_integral(pragma::Activity::VmAttack10), "ACT_VM_ATTACK10"},

    {pragma::math::to_integral(pragma::Activity::VmReload), "ACT_VM_RELOAD"},

    {pragma::math::to_integral(pragma::Activity::VmRun), "ACT_VM_RUN"}, {pragma::math::to_integral(pragma::Activity::VmRunPassive), "ACT_VM_RUN_PASSIVE"}, {pragma::math::to_integral(pragma::Activity::VmWalk), "ACT_VM_WALK"}, {pragma::math::to_integral(pragma::Activity::VmWalkCrouched), "ACT_VM_WALK_CROUCHED"},
    {pragma::math::to_integral(pragma::Activity::VmAimToPassive), "ACT_VM_AIM_TO_PASSIVE"},

    {pragma::math::to_integral(pragma::Activity::VmPassiveToAim), "ACT_PASSIVE_TO_AIM"}, {pragma::math::to_integral(pragma::Activity::VmAimToIronSight), "ACT_VM_AIM_TO_IRONSIGHT"}, {pragma::math::to_integral(pragma::Activity::VmIronSightToAim), "ACT_VM_IRONSIGHT_TO_AIM"}};
  return names;
}
#else
DLLNETWORK std::unordered_map<int32_t, std::string> ACTIVITY_NAMES = {{pragma::math::to_integral(pragma::Activity::Invalid), "ACT_INVALID"},

  {pragma::math::to_integral(pragma::Activity::Idle), "ACT_IDLE"}, {pragma::math::to_integral(pragma::Activity::Walk), "ACT_WALK"}, {pragma::math::to_integral(pragma::Activity::Run), "ACT_RUN"}, {pragma::math::to_integral(pragma::Activity::Jump), "ACT_JUMP"},
  {pragma::math::to_integral(pragma::Activity::Crouch), "ACT_CROUCH"}, {pragma::math::to_integral(pragma::Activity::CrouchIdle), "ACT_CROUCH_IDLE"}, {pragma::math::to_integral(pragma::Activity::CrouchWalk), "ACT_CROUCH_WALK"}, {pragma::math::to_integral(pragma::Activity::Uncrouch), "ACT_UNCROUCH"},
  {pragma::math::to_integral(pragma::Activity::Alert), "ACT_ALERT"},

  {pragma::math::to_integral(pragma::Activity::FlinchGeneric), "ACT_FLINCH_GENERIC"}, {pragma::math::to_integral(pragma::Activity::FlinchHead), "ACT_FLINCH_HEAD"}, {pragma::math::to_integral(pragma::Activity::FlinchChest), "ACT_FLINCH_CHEST"},
  {pragma::math::to_integral(pragma::Activity::FlinchStomach), "ACT_FLINCH_STOMACH"}, {pragma::math::to_integral(pragma::Activity::FlinchLeftArm), "ACT_FLINCH_LEFT_ARM"}, {pragma::math::to_integral(pragma::Activity::FlinchRightArm), "ACT_FLINCH_RIGHT_ARM"},
  {pragma::math::to_integral(pragma::Activity::FlinchLeftLeg), "ACT_FLINCH_LEFT_LEG"}, {pragma::math::to_integral(pragma::Activity::FlinchRightLeg), "ACT_FLINCH_RIGHT_LEG"}, {pragma::math::to_integral(pragma::Activity::FlinchPhysics), "ACT_FLINCH_PHYSICS"},
  {pragma::math::to_integral(pragma::Activity::FlinchLight), "ACT_FLINCH_LIGHT"}, {pragma::math::to_integral(pragma::Activity::FlinchMedium), "ACT_FLINCH_MEDIUM"}, {pragma::math::to_integral(pragma::Activity::FlinchHeavy), "ACT_FLINCH_HEAVY"},

  {pragma::math::to_integral(pragma::Activity::Die), "ACT_DIE"}, {pragma::math::to_integral(pragma::Activity::TurnLeft), "ACT_TURN_LEFT"}, {pragma::math::to_integral(pragma::Activity::TurnRight), "ACT_TURN_RIGHT"},

  {pragma::math::to_integral(pragma::Activity::GestureTurnLeft), "ACT_GESTURE_TURN_LEFT"}, {pragma::math::to_integral(pragma::Activity::GestureTurnRight), "ACT_GESTURE_TURN_RIGHT"},

  {pragma::math::to_integral(pragma::Activity::MeleeAttack1), "ACT_MELEE_ATTACK1"}, {pragma::math::to_integral(pragma::Activity::MeleeAttack2), "ACT_MELEE_ATTACK2"}, {pragma::math::to_integral(pragma::Activity::MeleeAttack3), "ACT_MELEE_ATTACK3"},
  {pragma::math::to_integral(pragma::Activity::MeleeAttack4), "ACT_MELEE_ATTACK4"}, {pragma::math::to_integral(pragma::Activity::MeleeAttack5), "ACT_MELEE_ATTACK5"}, {pragma::math::to_integral(pragma::Activity::RangeAttack1), "ACT_RANGE_ATTACK1"},
  {pragma::math::to_integral(pragma::Activity::RangeAttack2), "ACT_RANGE_ATTACK2"}, {pragma::math::to_integral(pragma::Activity::RangeAttack3), "ACT_RANGE_ATTACK3"}, {pragma::math::to_integral(pragma::Activity::RangeAttack4), "ACT_RANGE_ATTACK4"},
  {pragma::math::to_integral(pragma::Activity::RangeAttack5), "ACT_RANGE_ATTACK5"},

  {pragma::math::to_integral(pragma::Activity::VmIdle), "ACT_VM_IDLE"}, {pragma::math::to_integral(pragma::Activity::VmDeploy), "ACT_VM_DEPLOY"}, {pragma::math::to_integral(pragma::Activity::VmHolster), "ACT_VM_HOLSTER"},

  {pragma::math::to_integral(pragma::Activity::VmPrimaryAttack), "ACT_VM_PRIMARY_ATTACK"}, {pragma::math::to_integral(pragma::Activity::VmSecondaryAttack), "ACT_VM_SECONDARY_ATTACK"}, {pragma::math::to_integral(pragma::Activity::VmTertiaryAttack), "ACT_VM_TERTIARY_ATTACK"},
  {pragma::math::to_integral(pragma::Activity::VmAttack4), "ACT_VM_ATTACK4"}, {pragma::math::to_integral(pragma::Activity::VmAttack5), "ACT_VM_ATTACK5"}, {pragma::math::to_integral(pragma::Activity::VmAttack6), "ACT_VM_ATTACK6"}, {pragma::math::to_integral(pragma::Activity::VmAttack7), "ACT_VM_ATTACK7"},
  {pragma::math::to_integral(pragma::Activity::VmAttack8), "ACT_VM_ATTACK8"}, {pragma::math::to_integral(pragma::Activity::VmAttack9), "ACT_VM_ATTACK9"}, {pragma::math::to_integral(pragma::Activity::VmAttack10), "ACT_VM_ATTACK10"},

  {pragma::math::to_integral(pragma::Activity::VmReload), "ACT_VM_RELOAD"},

  {pragma::math::to_integral(pragma::Activity::VmRun), "ACT_VM_RUN"}, {pragma::math::to_integral(pragma::Activity::VmRunPassive), "ACT_VM_RUN_PASSIVE"}, {pragma::math::to_integral(pragma::Activity::VmWalk), "ACT_VM_WALK"}, {pragma::math::to_integral(pragma::Activity::VmWalkCrouched), "ACT_VM_WALK_CROUCHED"},
  {pragma::math::to_integral(pragma::Activity::VmAimToPassive), "ACT_VM_AIM_TO_PASSIVE"},

  {pragma::math::to_integral(pragma::Activity::VmPassiveToAim), "ACT_PASSIVE_TO_AIM"}, {pragma::math::to_integral(pragma::Activity::VmAimToIronSight), "ACT_VM_AIM_TO_IRONSIGHT"}, {pragma::math::to_integral(pragma::Activity::VmIronSightToAim), "ACT_VM_IRONSIGHT_TO_AIM"}};
#endif

void pragma::register_engine_activities()
{
	auto &reg = animation::Animation::GetActivityEnumRegister();
	for(auto i = std::underlying_type_t<Activity> {0}; i < math::to_integral(Activity::Count); ++i) {
		auto it = ACTIVITY_NAMES.find(i);
		assert(it != ACTIVITY_NAMES.end());
		auto id = reg.RegisterEnum(it->second);
		assert(id == i);
	}
}
