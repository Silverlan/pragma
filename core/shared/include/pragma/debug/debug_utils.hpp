/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __DEBUG_UTILS_HPP__
#define __DEBUG_UTILS_HPP__

#include "pragma/networkdefinitions.h"
#include <string>
#include <optional>
#include <cinttypes>

namespace pragma::debug {
	enum class MessageBoxButton : uint32_t {
		Ok = 0,
		Cancel,
		Abort,
		Retry,
		Ignore,
		Yes,
		No,
		TryAgain,
		Continue,
	};
	enum class MessageBoxButtons : uint8_t {
		Ok = 0,
		OkCancel,
		AbortRetryIgnore,
		YesNoCancel,
		YesNo,
		RetryCancel,
		CancelTryAgainContinue,
	};
	DLLNETWORK void open_file_in_zerobrane(const std::string &fileName, uint32_t lineIdx);
	DLLNETWORK std::optional<MessageBoxButton> show_message_prompt(const std::string &msg, MessageBoxButtons bts, std::optional<std::string> title = {});
#ifdef _WIN32
	DLLNETWORK bool is_module_in_callstack(struct _EXCEPTION_POINTERS *exp, const std::string &moduleName);
#endif
};

#endif
