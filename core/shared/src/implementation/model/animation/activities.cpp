// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


module pragma.shared;

import :model.animation.enums;

DLLNETWORK std::unordered_map<int32_t, std::string> ACTIVITY_NAMES = {{umath::to_integral(Activity::Invalid), "ACT_INVALID"},

  {umath::to_integral(Activity::Idle), "ACT_IDLE"}, {umath::to_integral(Activity::Walk), "ACT_WALK"}, {umath::to_integral(Activity::Run), "ACT_RUN"}, {umath::to_integral(Activity::Jump), "ACT_JUMP"}, {umath::to_integral(Activity::Crouch), "ACT_CROUCH"},
  {umath::to_integral(Activity::CrouchIdle), "ACT_CROUCH_IDLE"}, {umath::to_integral(Activity::CrouchWalk), "ACT_CROUCH_WALK"}, {umath::to_integral(Activity::Uncrouch), "ACT_UNCROUCH"}, {umath::to_integral(Activity::Alert), "ACT_ALERT"},

  {umath::to_integral(Activity::FlinchGeneric), "ACT_FLINCH_GENERIC"}, {umath::to_integral(Activity::FlinchHead), "ACT_FLINCH_HEAD"}, {umath::to_integral(Activity::FlinchChest), "ACT_FLINCH_CHEST"}, {umath::to_integral(Activity::FlinchStomach), "ACT_FLINCH_STOMACH"},
  {umath::to_integral(Activity::FlinchLeftArm), "ACT_FLINCH_LEFT_ARM"}, {umath::to_integral(Activity::FlinchRightArm), "ACT_FLINCH_RIGHT_ARM"}, {umath::to_integral(Activity::FlinchLeftLeg), "ACT_FLINCH_LEFT_LEG"}, {umath::to_integral(Activity::FlinchRightLeg), "ACT_FLINCH_RIGHT_LEG"},
  {umath::to_integral(Activity::FlinchPhysics), "ACT_FLINCH_PHYSICS"}, {umath::to_integral(Activity::FlinchLight), "ACT_FLINCH_LIGHT"}, {umath::to_integral(Activity::FlinchMedium), "ACT_FLINCH_MEDIUM"}, {umath::to_integral(Activity::FlinchHeavy), "ACT_FLINCH_HEAVY"},

  {umath::to_integral(Activity::Die), "ACT_DIE"}, {umath::to_integral(Activity::TurnLeft), "ACT_TURN_LEFT"}, {umath::to_integral(Activity::TurnRight), "ACT_TURN_RIGHT"},

  {umath::to_integral(Activity::GestureTurnLeft), "ACT_GESTURE_TURN_LEFT"}, {umath::to_integral(Activity::GestureTurnRight), "ACT_GESTURE_TURN_RIGHT"},

  {umath::to_integral(Activity::MeleeAttack1), "ACT_MELEE_ATTACK1"}, {umath::to_integral(Activity::MeleeAttack2), "ACT_MELEE_ATTACK2"}, {umath::to_integral(Activity::MeleeAttack3), "ACT_MELEE_ATTACK3"}, {umath::to_integral(Activity::MeleeAttack4), "ACT_MELEE_ATTACK4"},
  {umath::to_integral(Activity::MeleeAttack5), "ACT_MELEE_ATTACK5"}, {umath::to_integral(Activity::RangeAttack1), "ACT_RANGE_ATTACK1"}, {umath::to_integral(Activity::RangeAttack2), "ACT_RANGE_ATTACK2"}, {umath::to_integral(Activity::RangeAttack3), "ACT_RANGE_ATTACK3"},
  {umath::to_integral(Activity::RangeAttack4), "ACT_RANGE_ATTACK4"}, {umath::to_integral(Activity::RangeAttack5), "ACT_RANGE_ATTACK5"},

  {umath::to_integral(Activity::VmIdle), "ACT_VM_IDLE"}, {umath::to_integral(Activity::VmDeploy), "ACT_VM_DEPLOY"}, {umath::to_integral(Activity::VmHolster), "ACT_VM_HOLSTER"},

  {umath::to_integral(Activity::VmPrimaryAttack), "ACT_VM_PRIMARY_ATTACK"}, {umath::to_integral(Activity::VmSecondaryAttack), "ACT_VM_SECONDARY_ATTACK"}, {umath::to_integral(Activity::VmTertiaryAttack), "ACT_VM_TERTIARY_ATTACK"}, {umath::to_integral(Activity::VmAttack4), "ACT_VM_ATTACK4"},
  {umath::to_integral(Activity::VmAttack5), "ACT_VM_ATTACK5"}, {umath::to_integral(Activity::VmAttack6), "ACT_VM_ATTACK6"}, {umath::to_integral(Activity::VmAttack7), "ACT_VM_ATTACK7"}, {umath::to_integral(Activity::VmAttack8), "ACT_VM_ATTACK8"},
  {umath::to_integral(Activity::VmAttack9), "ACT_VM_ATTACK9"}, {umath::to_integral(Activity::VmAttack10), "ACT_VM_ATTACK10"},

  {umath::to_integral(Activity::VmReload), "ACT_VM_RELOAD"},

  {umath::to_integral(Activity::VmRun), "ACT_VM_RUN"}, {umath::to_integral(Activity::VmRunPassive), "ACT_VM_RUN_PASSIVE"}, {umath::to_integral(Activity::VmWalk), "ACT_VM_WALK"}, {umath::to_integral(Activity::VmWalkCrouched), "ACT_VM_WALK_CROUCHED"},
  {umath::to_integral(Activity::VmAimToPassive), "ACT_VM_AIM_TO_PASSIVE"},

  {umath::to_integral(Activity::VmPassiveToAim), "ACT_PASSIVE_TO_AIM"}, {umath::to_integral(Activity::VmAimToIronSight), "ACT_VM_AIM_TO_IRONSIGHT"}, {umath::to_integral(Activity::VmIronSightToAim), "ACT_VM_IRONSIGHT_TO_AIM"}};

void pragma::register_engine_activities()
{
	auto &reg = pragma::animation::Animation::GetActivityEnumRegister();
	for(auto i = std::underlying_type_t<Activity> {0}; i < umath::to_integral(Activity::Count); ++i) {
		auto it = ACTIVITY_NAMES.find(i);
		assert(it != ACTIVITY_NAMES.end());
		auto id = reg.RegisterEnum(it->second);
		assert(id == i);
	}
}
