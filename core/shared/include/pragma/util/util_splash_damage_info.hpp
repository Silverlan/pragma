/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __UTIL_SPLASH_DAMAGE_INFO_HPP__
#define __UTIL_SPLASH_DAMAGE_INFO_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/entities/baseentity_handle.h"
#include "pragma/game/damageinfo.h"
#include <mathutil/uvec.h>
#include <optional>
#include <functional>

namespace util {
	struct DLLNETWORK SplashDamageInfo {
		SplashDamageInfo();
		DamageInfo damageInfo = {};
		Vector3 origin = {};
		float radius = 0.f;
		std::optional<std::pair<Vector3, float>> cone = {};
		std::function<bool(BaseEntity *, DamageInfo &)> callback = nullptr;
	};
};

#endif
