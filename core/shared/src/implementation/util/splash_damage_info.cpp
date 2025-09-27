// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_shared.h"
#include "pragma/util/util_splash_damage_info.hpp"

module pragma.shared;

import :util.splash_damage_info;

using namespace util;

SplashDamageInfo::SplashDamageInfo() { damageInfo.SetDamageType(DAMAGETYPE::EXPLOSION); }
