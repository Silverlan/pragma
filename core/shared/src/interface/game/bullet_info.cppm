// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/networkdefinitions.h"

#include "pragma/lua/core.hpp"

export module pragma.shared:game.bullet_info;

export import :entities.base_entity_handle;
import :game.enums;

export {
	namespace bulletInfo {
		STATIC_CONST_COMPAT float DEFAULT_TRACER_RADIUS;
		STATIC_CONST_COMPAT Color DEFAULT_TRACER_COLOR;
		STATIC_CONST_COMPAT float DEFAULT_TRACER_LENGTH;
		STATIC_CONST_COMPAT float DEFAULT_TRACER_SPEED;
		STATIC_CONST_COMPAT std::string DEFAULT_TRACER_MATERIAL;
		STATIC_CONST_COMPAT float DEFAULT_TRACER_BLOOM;
	}
	struct DLLNETWORK BulletInfo {
		EulerAngles spread = EulerAngles(0, 0, 0);
		float force = std::numeric_limits<float>::signaling_NaN(); // Overwrite for ammo type?
		float distance = static_cast<float>(umath::to_integral(pragma::GameLimits::MaxWorldDistance));
		Vector3 direction = {}; // Not used for Entity::FireBullets
		Vector3 effectOrigin = {std::numeric_limits<float>::signaling_NaN(), std::numeric_limits<float>::signaling_NaN(), std::numeric_limits<float>::signaling_NaN()};
		uint32_t damage = std::numeric_limits<uint32_t>::max();
		DAMAGETYPE damageType = static_cast<DAMAGETYPE>(std::numeric_limits<std::underlying_type_t<DAMAGETYPE>>::max());
		uint32_t bulletCount = 1;
		EntityHandle hAttacker = {};
		EntityHandle hInflictor = {};
		uint32_t tracerCount = 1;
		float tracerRadius = bulletInfo::DEFAULT_TRACER_RADIUS;
		Color tracerColor = bulletInfo::DEFAULT_TRACER_COLOR;
		float tracerLength = bulletInfo::DEFAULT_TRACER_LENGTH;
		float tracerSpeed = bulletInfo::DEFAULT_TRACER_SPEED;
		std::string tracerMaterial = bulletInfo::DEFAULT_TRACER_MATERIAL;
		float tracerBloom = bulletInfo::DEFAULT_TRACER_BLOOM;
		std::string ammoType = "";
		// TODO Additional Filter
	};
	DLLNETWORK std::ostream &operator<<(std::ostream &out, const BulletInfo &o);
};
