// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :util.splash_damage_info;

pragma::util::SplashDamageInfo::SplashDamageInfo() { damageInfo.SetDamageType(Explosion); }
