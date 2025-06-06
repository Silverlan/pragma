/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __UTIL_VDF_HPP__
#define __UTIL_VDF_HPP__

#include <sharedutils/util_markup_file.hpp>
#include <unordered_map>
#include <vector>
#include <string>

namespace util::steam::vdf {
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

	DLLNETWORK util::MarkupFile::ResultCode read_vdf_block(util::MarkupFile &mf, util::steam::vdf::DataBlock &block, uint32_t depth = 0);
	DLLNETWORK bool get_external_steam_locations(const std::string &steamRootPath, std::vector<std::string> &outExtLocations);
};

#endif
