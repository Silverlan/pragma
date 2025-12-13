// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:util.splash_damage_info;

import :game.damage_info;

export namespace pragma::ecs {
	class BaseEntity;
}
export namespace pragma::util {
	struct DLLNETWORK SplashDamageInfo {
		SplashDamageInfo();
		game::DamageInfo damageInfo = {};
		Vector3 origin = {};
		float radius = 0.f;
		std::optional<std::pair<Vector3, float>> cone = {};
		std::function<bool(ecs::BaseEntity *, game::DamageInfo &)> callback = nullptr;
	};
};
