// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:physics.ik.controller;

export import :physics.ik.method;

export namespace pragma::physics {
	class DLLNETWORK IKController : public std::enable_shared_from_this<IKController> {
	  public:
		IKController(const std::string &effectorName, uint32_t chainLength, const std::string &type, ik::Method method = ik::Method::Default);
		IKController(const IKController &other);

		const std::string &GetEffectorName() const;
		uint32_t GetChainLength() const;
		const std::string &GetType() const;

		void SetEffectorName(const std::string &name);
		void SetChainLength(uint32_t len);
		void SetType(const std::string &type);

		void SetMethod(ik::Method method);
		ik::Method GetMethod() const;

		const std::unordered_map<std::string, std::string> &GetKeyValues() const;
		std::unordered_map<std::string, std::string> &GetKeyValues();

		bool operator==(const IKController &other) const;
		bool operator!=(const IKController &other) const { return !operator==(other); }
	  protected:
		std::string m_effectorName;
		std::string m_type;
		uint32_t m_chainLength = 0u;
		ik::Method m_method = ik::Method::Default;

		std::unordered_map<std::string, std::string> m_keyValues;
	};
};
