#ifndef __ACTIVITIES_H__
#define __ACTIVITIES_H__

#include "pragma/networkdefinitions.h"
#include <mathutil/umath.h>

enum class DLLNETWORK Activity : uint16_t
{
	Invalid,

	Idle,
	Walk,
	Run,
	Jump,
	Crouch,
	CrouchIdle,
	CrouchWalk,
	Uncrouch,
	Alert,

	FlinchHead,
	FlinchChest,
	FlinchStomach,
	FlinchLeftArm,
	FlinchRightArm,
	FlinchLeftLeg,
	FlinchRightLeg,
	FlinchPhysics,
	FlinchGeneric,
	FlinchLight,
	FlinchMedium,
	FlinchHeavy,

	Die,
	TurnLeft,
	TurnRight,

	MeleeAttack1,
	MeleeAttack2,
	MeleeAttack3,
	MeleeAttack4,
	MeleeAttack5,
	RangeAttack1,
	RangeAttack2,
	RangeAttack3,
	RangeAttack4,
	RangeAttack5,

	VmIdle,
	VmDeploy,
	VmHolster,

	VmPrimaryAttack,
	VmSecondaryAttack,
	VmTertiaryAttack,
	VmAttack4,
	VmAttack5,
	VmAttack6,
	VmAttack7,
	VmAttack8,
	VmAttack9,
	VmAttack10,

	VmReload,

	VmRun,
	VmRunPassive,
	VmWalk,
	VmWalkCrouched,
	VmAimToPassive,

	VmPassiveToAim,
	VmAimToIronSight,
	VmIronSightToAim,

	Count
};
REGISTER_BASIC_ARITHMETIC_OPERATORS(Activity)

extern DLLNETWORK std::unordered_map<int32_t,std::string> ACTIVITY_NAMES;

namespace pragma
{
	DLLNETWORK void register_engine_activities();
};

#endif
