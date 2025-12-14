// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :physics.ik.controller;

pragma::physics::IKController::IKController(const std::string &effectorName, uint32_t chainLength, const std::string &type, ik::Method method) : m_effectorName(effectorName), m_chainLength(chainLength), m_type(type), m_method(method) {}

pragma::physics::IKController::IKController(const IKController &other) : m_effectorName {other.m_effectorName}, m_type {other.m_type}, m_chainLength {other.m_chainLength}, m_method {other.m_method}, m_keyValues {other.m_keyValues}
{
#ifdef _MSC_VER
	static_assert(sizeof(IKController) == 152, "Update this function when making changes to this class!");
#endif
}
const std::string &pragma::physics::IKController::GetEffectorName() const { return m_effectorName; }
uint32_t pragma::physics::IKController::GetChainLength() const { return m_chainLength; }
const std::string &pragma::physics::IKController::GetType() const { return m_type; }

void pragma::physics::IKController::SetEffectorName(const std::string &name) { m_effectorName = name; }
void pragma::physics::IKController::SetChainLength(uint32_t len) { m_chainLength = len; }
void pragma::physics::IKController::SetType(const std::string &type) { m_type = type; }

void pragma::physics::IKController::SetMethod(ik::Method method) { m_method = method; }
pragma::physics::ik::Method pragma::physics::IKController::GetMethod() const { return m_method; }

const std::unordered_map<std::string, std::string> &pragma::physics::IKController::GetKeyValues() const { return const_cast<IKController *>(this)->GetKeyValues(); }
std::unordered_map<std::string, std::string> &pragma::physics::IKController::GetKeyValues() { return m_keyValues; }

bool pragma::physics::IKController::operator==(const IKController &other) const
{
#ifdef _MSC_VER
	static_assert(sizeof(IKController) == 152, "Update this function when making changes to this class!");
#endif
	return m_effectorName == other.m_effectorName && m_type == other.m_type && m_chainLength == other.m_chainLength && m_method == other.m_method && m_keyValues == other.m_keyValues;
}
