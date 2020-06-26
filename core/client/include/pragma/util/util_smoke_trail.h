/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __UTIL_SMOKE_TRAIL_H__
#define __UTIL_SMOKE_TRAIL_H__

#include "pragma/clientdefinitions.h"
#include <string>

namespace pragma {class CParticleSystemComponent;};
namespace util
{
	DLLCLIENT ::pragma::CParticleSystemComponent *create_smoke_trail_particle(float distance,float speed,float minSpriteSize,float maxSpriteSize,const std::string &material="particles/smoke_sprites_dense");
};

#endif
