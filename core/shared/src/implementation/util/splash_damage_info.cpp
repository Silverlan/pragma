// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :util.splash_damage_info;

using namespace util;

SplashDamageInfo::SplashDamageInfo() { damageInfo.SetDamageType(DamageType::Explosion); }
