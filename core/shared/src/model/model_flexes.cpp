/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/model/model_flexes.hpp"
#include "pragma/model/animation/vertex_animation.hpp"

Flex::Operation::Operation(Type t,float value)
	: type(t)
{
	d.value = value;
}
Flex::Operation::Operation(Type t,int32_t index)
	: type(t)
{
	d.index = index;
}
Flex::Flex(const std::string &name)
	: m_name(name)
{}
const std::string &Flex::GetName() const {return m_name;}
const std::vector<Flex::Operation> &Flex::GetOperations() const {return const_cast<Flex*>(this)->GetOperations();}
std::vector<Flex::Operation> &Flex::GetOperations() {return m_operations;}

uint32_t Flex::GetFrameIndex() const {return m_frameIndex;}
VertexAnimation *Flex::GetVertexAnimation() const {return m_vertexAnim.lock().get();}
void Flex::SetVertexAnimation(VertexAnimation &anim,uint32_t frameIndex)
{
	m_vertexAnim = anim.shared_from_this();
	m_frameIndex = frameIndex;
}
