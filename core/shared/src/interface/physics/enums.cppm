// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

export module pragma.shared:physics.enums;

import pragma.math;

export {
	namespace pragma::physics {
		enum class MoveType : int32_t { None = 0, Walk, Fly, Noclip, Free, Physics };

		enum class CollisionMask : uint32_t {
			None = 0,
			Static = 1,
			Dynamic = Static << 1,
			Generic = Dynamic << 1,
			Player = Generic << 1,
			NPC = Player << 1,
			Vehicle = NPC << 1,
			Item = Vehicle << 1,
			Particle = Item << 1,
			Character = Particle << 1,
			Trigger = Character << 1,
			Water = Trigger << 1,
			WaterSurface = Water << 1,
			All = (WaterSurface << 1) - 1,
			Default = WaterSurface << 1,

			PlayerHitbox = Default << 1,
			NPCHitbox = PlayerHitbox << 1,
			CharacterHitbox = NPCHitbox | PlayerHitbox,
			AllHitbox = All & ~Player & ~NPC | CharacterHitbox,

			NoCollision = NPCHitbox << 1 // Only use as collision group, never as collision mask
		};
		enum class RayCastFlags : uint32_t {
			None = 0u,
			ReportHitPosition = 1u,
			ReportHitNormal = ReportHitPosition << 1u,
			ReportHitUV = ReportHitNormal << 1u,
			ReportAllResults = ReportHitUV << 1u,
			ReportAnyResult = ReportAllResults << 1u,
			ReportBackFaceHits = ReportAnyResult << 1u,
			Precise = ReportBackFaceHits << 1u,

			IgnoreDynamic = Precise << 1u,
			IgnoreStatic = IgnoreDynamic << 1u,
			InvertFilter = IgnoreStatic << 1u,

			Default = ReportHitPosition | ReportHitNormal | ReportHitUV
		};
		using namespace pragma::math::scoped_enum::bitwise;

		enum class CollisionType : int32_t { None = 0, AABB, OBB, Brush };
		enum class PhysicsType : int32_t { None = 0, Dynamic, Static, SoftBody, BoxController, CapsuleController };
		enum class RayCastHitType : uint8_t { None = 0, Touch, Block };
	};
	REGISTER_ENUM_FLAGS(pragma::physics::CollisionMask)
	REGISTER_ENUM_FLAGS(pragma::physics::RayCastFlags)
}
