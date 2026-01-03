// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:game.bullet_info;

export import :entities.base_entity_handle;
import :game.enums;

export namespace pragma::game {
	namespace bulletInfo {
		CONSTEXPR_DLL_COMPAT float DEFAULT_TRACER_RADIUS = 1.f;
#ifdef WINDOWS_CLANG_COMPILER_FIX
		DLLNETWORK Color &DEFAULT_TRACER_COLOR {
			static Color col = Color(510, 510, 180, 510);
			return col;
		}
#else
		CONSTEXPR_DLL_COMPAT Color DEFAULT_TRACER_COLOR = Color(510, 510, 180, 510);
#endif
		CONSTEXPR_DLL_COMPAT float DEFAULT_TRACER_LENGTH = 200.f;
		CONSTEXPR_DLL_COMPAT float DEFAULT_TRACER_SPEED = 6'000.f;
		CONSTEXPR_DLL_COMPAT std::string_view DEFAULT_TRACER_MATERIAL = "particles/beam_tracer";
		CONSTEXPR_DLL_COMPAT float DEFAULT_TRACER_BLOOM = 0.25f;
	}
	struct DLLNETWORK BulletInfo {
		EulerAngles spread = EulerAngles(0, 0, 0);
		float force = std::numeric_limits<float>::signaling_NaN(); // Overwrite for ammo type?
		float distance = static_cast<float>(math::to_integral(GameLimits::MaxWorldDistance));
		Vector3 direction = {}; // Not used for Entity::FireBullets
		Vector3 effectOrigin = {std::numeric_limits<float>::signaling_NaN(), std::numeric_limits<float>::signaling_NaN(), std::numeric_limits<float>::signaling_NaN()};
		uint32_t damage = std::numeric_limits<uint32_t>::max();
		DamageType damageType = static_cast<DamageType>(std::numeric_limits<std::underlying_type_t<DamageType>>::max());
		uint32_t bulletCount = 1;
		EntityHandle hAttacker = {};
		EntityHandle hInflictor = {};
		uint32_t tracerCount = 1;
		float tracerRadius = bulletInfo::DEFAULT_TRACER_RADIUS;
		Color tracerColor = bulletInfo::DEFAULT_TRACER_COLOR;
		float tracerLength = bulletInfo::DEFAULT_TRACER_LENGTH;
		float tracerSpeed = bulletInfo::DEFAULT_TRACER_SPEED;
		std::string tracerMaterial {bulletInfo::DEFAULT_TRACER_MATERIAL};
		float tracerBloom = bulletInfo::DEFAULT_TRACER_BLOOM;
		std::string ammoType = "";
		// TODO Additional Filter
	};
	DLLNETWORK std::ostream &operator<<(std::ostream &out, const BulletInfo &o);
};
