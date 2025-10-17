// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include <optional>
#include <functional>

export module pragma.shared:util.splash_damage_info;

import :game.damage_info;

export class BaseEntity;
export namespace util {
	struct DLLNETWORK SplashDamageInfo {
		SplashDamageInfo();
		DamageInfo damageInfo = {};
		Vector3 origin = {};
		float radius = 0.f;
		std::optional<std::pair<Vector3, float>> cone = {};
		std::function<bool(BaseEntity *, DamageInfo &)> callback = nullptr;
	};
};
