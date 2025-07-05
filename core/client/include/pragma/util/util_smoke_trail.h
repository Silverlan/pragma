// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __UTIL_SMOKE_TRAIL_H__
#define __UTIL_SMOKE_TRAIL_H__

#include "pragma/clientdefinitions.h"
#include <string>

namespace pragma {
	class CParticleSystemComponent;
};
namespace util {
	DLLCLIENT ::pragma::CParticleSystemComponent *create_smoke_trail_particle(float distance, float speed, float minSpriteSize, float maxSpriteSize, const std::string &material = "particles/smoke_sprites_dense");
};

#endif
