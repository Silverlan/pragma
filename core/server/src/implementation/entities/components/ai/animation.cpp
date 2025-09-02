// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"

module pragma.server.entities.components.ai;

using namespace pragma;

void SAIComponent::LockAnimation(bool b) { m_bAnimLocked = b; }
bool SAIComponent::IsAnimationLocked() const { return m_bAnimLocked; }

void SAIComponent::MaintainAnimationMovement(const Vector3 &disp)
{
	if(uvec::length_sqr(disp) == 0.f)
		return;
	Vector3 dirAnimMove = uvec::get_normal(disp);
	Vector3 dirMove = uvec::get_normal(m_posMove); // TODO: Move in actual animation move dir, not disp
	float dot = glm::dot(dirAnimMove, dirMove);
	dot = (dot + 1.f) / 2.f;
	Vector3 mv = disp * dot; // TODO
	                         //SetVelocity(mv);
}
