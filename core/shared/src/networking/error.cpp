// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include "pragma/networking/error.hpp"
#include "pragma/networking/enums.hpp"

pragma::networking::Error::Error(ErrorCode errCode, const std::string &message, std::optional<int32_t> nativeErrCode) : m_message {message}, m_nativeErrCode {nativeErrCode} {}
pragma::networking::Error::Error() {}
const std::string &pragma::networking::Error::GetMessage() const { return m_message; }
pragma::networking::Error::operator bool() const { return m_errorCode == ErrorCode::Success; }
