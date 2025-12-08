// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :model.flexes;

pragma::animation::Flex::Operation::ValueType pragma::animation::Flex::Operation::GetOperationValueType(Type type)
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
pragma::animation::Flex::Operation::Operation(Type t, float value) : type(t) { d.value = value; }
pragma::animation::Flex::Operation::Operation(Type t, int32_t index) : type(t) { d.index = index; }
pragma::animation::Flex::Flex(const std::string &name) : m_name(name) {}
std::string &pragma::animation::Flex::GetName() { return m_name; }
void pragma::animation::Flex::SetName(const std::string &name) { m_name = name; }
const std::vector<pragma::animation::Flex::Operation> &pragma::animation::Flex::GetOperations() const { return const_cast<Flex *>(this)->GetOperations(); }
std::vector<pragma::animation::Flex::Operation> &pragma::animation::Flex::GetOperations() { return m_operations; }

uint32_t pragma::animation::Flex::GetFrameIndex() const { return m_frameIndex; }
pragma::animation::VertexAnimation *pragma::animation::Flex::GetVertexAnimation() const { return m_vertexAnim.lock().get(); }
void pragma::animation::Flex::SetVertexAnimation(VertexAnimation &anim, uint32_t frameIndex)
{
	m_vertexAnim = anim.shared_from_this();
	m_frameIndex = frameIndex;
}
