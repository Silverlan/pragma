/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/networking/error.hpp"
#include "pragma/networking/enums.hpp"

pragma::networking::Error::Error(ErrorCode errCode, const std::string &message, std::optional<int32_t> nativeErrCode) : m_message {message}, m_nativeErrCode {nativeErrCode} {}
pragma::networking::Error::Error() {}
const std::string &pragma::networking::Error::GetMessage() const { return m_message; }
pragma::networking::Error::operator bool() const { return m_errorCode == ErrorCode::Success; }
