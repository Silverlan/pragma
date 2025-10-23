// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/networkdefinitions.h"
#include <cinttypes>
#include <string>

export module pragma.shared:map.map_info;

export {
	constexpr uint32_t WLD_VERSION = 12;
	struct DLLNETWORK MapInfo {
		std::string name;
		std::string fileName;
		std::string md5;
	};
};
