/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#include "stdafx_server.h"
#include "pragma/entities/components/s_ai_component.hpp"

using namespace pragma;

void SAIComponent::LockAnimation(bool b) {m_bAnimLocked = b;}
bool SAIComponent::IsAnimationLocked() const {return m_bAnimLocked;}

void SAIComponent::MaintainAnimationMovement(const Vector3 &disp)
{
	if(uvec::length_sqr(disp) == 0.f)
		return;
	Vector3 dirAnimMove = uvec::get_normal(disp);
	Vector3 dirMove = uvec::get_normal(m_posMove); // TODO: Move in actual animation move dir, not disp
	float dot = glm::dot(dirAnimMove,dirMove);
	dot = (dot +1.f) /2.f;
	Vector3 mv = disp *dot; // TODO
	//SetVelocity(mv);
}

