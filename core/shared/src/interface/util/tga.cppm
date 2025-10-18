// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include <cinttypes>
#include <vector>
#include <string>
#include <memory>

export module pragma.shared:util.tga;

export import pragma.filesystem;

export namespace util {
	namespace tga {
		DLLNETWORK void write_tga(std::shared_ptr<VFilePtrInternalReal> &f, uint32_t w, uint32_t h, const std::vector<uint8_t> &pixels);
		DLLNETWORK void write_tga(std::shared_ptr<VFilePtrInternalReal> &f, uint32_t w, uint32_t h, const uint8_t *pixelData);
		DLLNETWORK bool write_tga(const std::string &fileName, uint32_t w, uint32_t h, const std::vector<uint8_t> &pixels);
	};
};
