/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/physics/ik/ik_controller.hpp"

IKController::IKController(const std::string &effectorName,uint32_t chainLength,const std::string &type,util::ik::Method method)
	: m_effectorName(effectorName),m_chainLength(chainLength),m_type(type),
	m_method(method)
{}

IKController::IKController(const IKController &other)
	: m_effectorName{other.m_effectorName},m_type{other.m_type},
	m_chainLength{other.m_chainLength},m_method{other.m_method},
	m_keyValues{other.m_keyValues}
{
	static_assert(sizeof(IKController) == 152,"Update this function when making changes to this class!");
}
const std::string &IKController::GetEffectorName() const {return m_effectorName;}
uint32_t IKController::GetChainLength() const {return m_chainLength;}
const std::string &IKController::GetType() const {return m_type;}

void IKController::SetEffectorName(const std::string &name) {m_effectorName = name;}
void IKController::SetChainLength(uint32_t len) {m_chainLength = len;}
void IKController::SetType(const std::string &type) {m_type = type;}

void IKController::SetMethod(util::ik::Method method) {m_method = method;}
util::ik::Method IKController::GetMethod() const {return m_method;}

const std::unordered_map<std::string,std::string> &IKController::GetKeyValues() const {return const_cast<IKController*>(this)->GetKeyValues();}
std::unordered_map<std::string,std::string> &IKController::GetKeyValues() {return m_keyValues;}
