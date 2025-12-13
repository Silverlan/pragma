// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:util.steam.vdf;

export import pragma.util;

export namespace pragma::util::steam::vdf {
	class DataBlock {
	  public:
		DataBlock() {}
		std::unordered_map<std::string, DataBlock> children;
		std::unordered_map<std::string, std::string> keyValues;
	};

	class Data {
	  public:
		Data() {}
		DataBlock dataBlock = {};
	};

	DLLNETWORK MarkupFile::ResultCode read_vdf_block(MarkupFile &mf, DataBlock &block, uint32_t depth = 0);
	DLLNETWORK bool get_external_steam_locations(const std::string &steamRootPath, std::vector<std::string> &outExtLocations);
};
