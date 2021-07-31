/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/util/bulletinfo.h"
#include <sharedutils/magic_enum.hpp>

const float BulletInfo::DEFAULT_TRACER_RADIUS = 1.f;
const Color BulletInfo::DEFAULT_TRACER_COLOR = Color(510,510,180,510);
const float BulletInfo::DEFAULT_TRACER_LENGTH = 200.f;
const float BulletInfo::DEFAULT_TRACER_SPEED = 6'000.f;
const std::string BulletInfo::DEFAULT_TRACER_MATERIAL = "particles/beam_tracer";
const float BulletInfo::DEFAULT_TRACER_BLOOM = 0.25f;

std::ostream &operator<<(std::ostream &out,const BulletInfo &o)
{
	out<<"BulletInfo";
	out<<"[Spread:"<<o.spread<<"]";
	out<<"[Force:"<<o.force<<"]";
	out<<"[Dist:"<<o.distance<<"]";
	out<<"[Dir:"<<o.direction<<"]";
	out<<"[Origin:"<<o.effectOrigin<<"]";
	out<<"[Dmg:"<<o.damage<<"]";
	out<<"[DmgType:"<<magic_enum::enum_name(o.damageType)<<"]";
	out<<"[Bullets:"<<o.bulletCount<<"]";

	out<<"[Attacker:";
	if(o.hAttacker.valid())
		const_cast<BaseEntity*>(o.hAttacker.get())->print(out);
	else
		out<<"NULL";
	out<<"]";

	out<<"[Inflictor:";
	if(o.hInflictor.valid())
		const_cast<BaseEntity*>(o.hInflictor.get())->print(out);
	else
		out<<"NULL";
	out<<"]";

	out<<"[Tracers:"<<o.tracerCount<<"]";
	out<<"[TracerRadius:"<<o.tracerRadius<<"]";
	out<<"[TracerCol:"<<o.tracerColor<<"]";
	out<<"[TracerLen:"<<o.tracerLength<<"]";
	out<<"[TracerSpeed:"<<o.tracerSpeed<<"]";
	out<<"[TracerMat:"<<o.tracerMaterial<<"]";
	out<<"[TracerBloom:"<<o.tracerBloom<<"]";
	out<<"[AmmoType:"<<o.ammoType<<"]";
	return out;
}
