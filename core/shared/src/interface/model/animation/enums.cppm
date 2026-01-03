// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

export module pragma.shared:model.animation.enums;

import pragma.math;

export {
	constexpr uint32_t PRAGMA_ANIMATION_VERSION = 2;
	namespace pragma {
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
		using namespace pragma::math::scoped_enum::bitwise;
	}
	REGISTER_ENUM_FLAGS(pragma::Activity)

#ifdef WINDOWS_CLANG_COMPILER_FIX
	DLLNETWORK std::unordered_map<int32_t, std::string> &GET_ACTIVITY_NAMES();
#else
	extern DLLNETWORK std::unordered_map<int32_t, std::string> ACTIVITY_NAMES;
#endif

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
		using namespace pragma::math::scoped_enum::bitwise;
	};
	REGISTER_ENUM_FLAGS(pragma::FPlayAnim)
	REGISTER_ENUM_FLAGS(pragma::FAnim)

	namespace pragma::animation {
		using BoneId = uint16_t;
		using FlexControllerId = uint32_t;
		constexpr auto INVALID_BONE_INDEX = std::numeric_limits<BoneId>::max();
		constexpr auto INVALID_FLEX_CONTROLLER_INDEX = std::numeric_limits<FlexControllerId>::max();
	};
};
