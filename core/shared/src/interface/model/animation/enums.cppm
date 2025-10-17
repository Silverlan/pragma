// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include <unordered_map>
#include <string>

export module pragma.shared:model.animation.enums;

export {
    constexpr uint32_t PRAGMA_ANIMATION_VERSION = 2;
	enum class Activity : uint16_t {
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

		GestureTurnLeft,
		GestureTurnRight,

		Count
	};
	REGISTER_BASIC_ARITHMETIC_OPERATORS(Activity)

	extern DLLNETWORK std::unordered_map<int32_t, std::string> ACTIVITY_NAMES;

	namespace pragma {
		DLLNETWORK void register_engine_activities();
	};

	namespace pragma {
		enum class FPlayAnim : uint32_t {
			None = 0u,
			Reset = 1u,
			Transmit = Reset << 1u,  // Only used for entities
			SnapTo = Transmit << 1u, // Unused
			Loop = SnapTo << 1u,

			Default = Transmit
		};
	};
	REGISTER_BASIC_BITWISE_OPERATORS(pragma::FPlayAnim)

	enum class FAnim : uint32_t {
		None = 0,
		Loop = 1,
		NoRepeat = 2,
		MoveX = 32,
		MoveZ = 64,
		Autoplay = 128,
		Gesture = 256,
		NoMoveBlend = 512,

		Count = 7
	};
	REGISTER_BASIC_BITWISE_OPERATORS(FAnim)

	namespace pragma::animation {
		using BoneId = uint16_t;
		using FlexControllerId = uint32_t;
		constexpr auto INVALID_BONE_INDEX = std::numeric_limits<BoneId>::max();
		constexpr auto INVALID_FLEX_CONTROLLER_INDEX = std::numeric_limits<FlexControllerId>::max();
	};
};
