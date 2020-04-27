/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_engine.h"
#include "pragma/util/transform.h"

Transform::Transform()
{}
Transform::Transform(const Vector3 &position)
	: m_pos(position)
{UpdateMatrix();}
Transform::Transform(const Quat &orientation)
	: m_orientation(orientation)
{UpdateMatrix();}
Transform::Transform(const Vector3 &position,const Quat &orientation)
	: m_pos(position),m_orientation(orientation)
{UpdateMatrix();}
const Vector3 &Transform::GetScale() const {return m_scale;}
const Vector3 &Transform::GetPosition() const {return m_pos;}
const Quat &Transform::GetOrientation() const {return m_orientation;}
const Mat4 &Transform::GetTransformationMatrix() const {return m_transform;}
void Transform::UpdateMatrix()
{
	m_transform = glm::translate(m_transform,m_pos);
	m_transform = m_transform *glm::toMat4(m_orientation);
	if(m_scale.x != 1.f || m_scale.y != 1.f || m_scale.z != 1.f)
		m_transform = glm::scale(m_transform,m_scale);
}
void Transform::SetScale(const Vector3 &scale) {m_scale = scale;}
void Transform::SetPosition(const Vector3 &pos)
{
	m_pos = pos;
}
void Transform::SetOrientation(const Quat &orientation)
{
	m_orientation = orientation;
}