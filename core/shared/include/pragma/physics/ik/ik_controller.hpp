/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __IK_CONTROLLER_HPP__
#define __IK_CONTROLLER_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/physics/ik/ik_method.hpp"
#include <string>
#include <memory>

class DLLNETWORK IKController : public std::enable_shared_from_this<IKController> {
  public:
	IKController(const std::string &effectorName, uint32_t chainLength, const std::string &type, util::ik::Method method = util::ik::Method::Default);
	IKController(const IKController &other);

	const std::string &GetEffectorName() const;
	uint32_t GetChainLength() const;
	const std::string &GetType() const;

	void SetEffectorName(const std::string &name);
	void SetChainLength(uint32_t len);
	void SetType(const std::string &type);

	void SetMethod(util::ik::Method method);
	util::ik::Method GetMethod() const;

	const std::unordered_map<std::string, std::string> &GetKeyValues() const;
	std::unordered_map<std::string, std::string> &GetKeyValues();

	bool operator==(const IKController &other) const;
	bool operator!=(const IKController &other) const { return !operator==(other); }
  protected:
	std::string m_effectorName;
	std::string m_type;
	uint32_t m_chainLength = 0u;
	util::ik::Method m_method = util::ik::Method::Default;

	std::unordered_map<std::string, std::string> m_keyValues;
};

#endif
