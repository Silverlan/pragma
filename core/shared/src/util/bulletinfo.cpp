/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/util/bulletinfo.h"

const float BulletInfo::DEFAULT_TRACER_RADIUS = 1.f;
const Color BulletInfo::DEFAULT_TRACER_COLOR = Color(510,510,180,510);
const float BulletInfo::DEFAULT_TRACER_LENGTH = 200.f;
const float BulletInfo::DEFAULT_TRACER_SPEED = 6'000.f;
const std::string BulletInfo::DEFAULT_TRACER_MATERIAL = "particles/beam_tracer";
const float BulletInfo::DEFAULT_TRACER_BLOOM = 0.25f;
