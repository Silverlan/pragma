// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include "pragma/game/game_limits.h"
#include "pragma/game/damagetype.h"
#include <mathutil/color.h>
#include <limits>

export module pragma.shared:game.bullet_info;

export {
	struct DLLNETWORK BulletInfo {
		static const float DEFAULT_TRACER_RADIUS;
		static const Color DEFAULT_TRACER_COLOR;
		static const float DEFAULT_TRACER_LENGTH;
		static const float DEFAULT_TRACER_SPEED;
		static const std::string DEFAULT_TRACER_MATERIAL;
		static const float DEFAULT_TRACER_BLOOM;

		EulerAngles spread = EulerAngles(0, 0, 0);
		float force = std::numeric_limits<float>::signaling_NaN(); // Overwrite for ammo type?
		float distance = static_cast<float>(umath::to_integral(GameLimits::MaxWorldDistance));
		Vector3 direction = {}; // Not used for Entity::FireBullets
		Vector3 effectOrigin = {std::numeric_limits<float>::signaling_NaN(), std::numeric_limits<float>::signaling_NaN(), std::numeric_limits<float>::signaling_NaN()};
		uint32_t damage = std::numeric_limits<uint32_t>::max();
		DAMAGETYPE damageType = static_cast<DAMAGETYPE>(std::numeric_limits<std::underlying_type_t<DAMAGETYPE>>::max());
		uint32_t bulletCount = 1;
		EntityHandle hAttacker = {};
		EntityHandle hInflictor = {};
		uint32_t tracerCount = 1;
		float tracerRadius = DEFAULT_TRACER_RADIUS;
		Color tracerColor = DEFAULT_TRACER_COLOR;
		float tracerLength = DEFAULT_TRACER_LENGTH;
		float tracerSpeed = DEFAULT_TRACER_SPEED;
		std::string tracerMaterial = DEFAULT_TRACER_MATERIAL;
		float tracerBloom = DEFAULT_TRACER_BLOOM;
		std::string ammoType = "";
		// TODO Additional Filter
	};
	lua_registercheck(BulletInfo, BulletInfo);
	DLLNETWORK std::ostream &operator<<(std::ostream &out, const BulletInfo &o);
};
