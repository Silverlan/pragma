// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:networking.error;

export import std.compat;

#undef GetMessage

export namespace pragma::networking {
	enum class ErrorCode : uint32_t;
	class DLLNETWORK Error {
	  public:
		Error(ErrorCode errCode, const std::string &message = "", std::optional<int32_t> nativeErrCode = {});
		Error();
		const std::string &GetMessage() const;
		operator bool() const;
	  private:
		std::string m_message {};
		std::optional<int32_t> m_nativeErrCode = {};
		ErrorCode m_errorCode = static_cast<ErrorCode>(0);
	};
};
