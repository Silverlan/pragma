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
	DLLNETWORK void open_file_in_zerobrane(const std::string &fileName, uint32_t lineIdx);
#ifdef _WIN32
	DLLNETWORK bool is_module_in_callstack(struct _EXCEPTION_POINTERS *exp, const std::string &moduleName);
#endif
	DLLNETWORK void start_profiling_task(const char *taskName);
	DLLNETWORK void end_profiling_task();
};

#endif
