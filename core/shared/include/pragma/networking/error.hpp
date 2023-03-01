/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __PRAGMA_ERROR_HPP__
#define __PRAGMA_ERROR_HPP__

#include "pragma/networkdefinitions.h"
#include <string>
#include <optional>

#undef GetMessage

namespace pragma::networking {
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

#endif
