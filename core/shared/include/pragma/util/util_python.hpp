// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __PRAGMA_UTIL_PYTHON_HPP__
#define __PRAGMA_UTIL_PYTHON_HPP__

#include "pragma/networkdefinitions.h"
#include <cinttypes>

namespace pragma::python {
	DLLNETWORK bool run(const char *code);
	DLLNETWORK bool exec(std::string fileName, uint32_t argc, const char **argv);
	DLLNETWORK std::optional<std::string> get_last_error();
	DLLNETWORK void reload();

	DLLNETWORK bool init_blender();
};

#endif
