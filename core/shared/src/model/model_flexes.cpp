/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/model/model_flexes.hpp"
#include "pragma/model/animation/vertex_animation.hpp"

Flex::Operation::ValueType Flex::Operation::GetOperationValueType(Type type)
{
	switch(type) {
	case Type::None:
	case Type::Add:
	case Type::Sub:
	case Type::Mul:
	case Type::Div:
	case Type::Neg:
	case Type::Exp:
	case Type::Open:
	case Type::Close:
	case Type::Comma:
	case Type::Max:
	case Type::Min:
		return ValueType::None;
	case Type::Const:
		return ValueType::Value;
	case Type::Fetch:
	case Type::Fetch2:
	case Type::TwoWay0:
	case Type::TwoWay1:
	case Type::NWay:
	case Type::Combo:
	case Type::Dominate:
	case Type::DMELowerEyelid:
	case Type::DMEUpperEyelid:
		return ValueType::Index;
	}
	return ValueType::None;
}
Flex::Operation::Operation(Type t, float value) : type(t) { d.value = value; }
Flex::Operation::Operation(Type t, int32_t index) : type(t) { d.index = index; }
Flex::Flex(const std::string &name) : m_name(name) {}
std::string &Flex::GetName() { return m_name; }
void Flex::SetName(const std::string &name) { m_name = name; }
const std::vector<Flex::Operation> &Flex::GetOperations() const { return const_cast<Flex *>(this)->GetOperations(); }
std::vector<Flex::Operation> &Flex::GetOperations() { return m_operations; }

uint32_t Flex::GetFrameIndex() const { return m_frameIndex; }
VertexAnimation *Flex::GetVertexAnimation() const { return m_vertexAnim.lock().get(); }
void Flex::SetVertexAnimation(VertexAnimation &anim, uint32_t frameIndex)
{
	m_vertexAnim = anim.shared_from_this();
	m_frameIndex = frameIndex;
}
