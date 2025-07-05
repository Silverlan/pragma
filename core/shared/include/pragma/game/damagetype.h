// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __DAMAGETYPE_H__
#define __DAMAGETYPE_H__
#include "pragma/networkdefinitions.h"
#include <cinttypes>

enum DLLNETWORK DAMAGETYPE : uint32_t { GENERIC = 0, EXPLOSION = 1, BULLET = 2, BASH = 32, CRUSH = 64, SLASH = 128, ELECTRICITY = 256, ENERGY = 512, FIRE = 1024, PLASMA = 2048, IGNITE = FIRE | 4096 };

#endif
