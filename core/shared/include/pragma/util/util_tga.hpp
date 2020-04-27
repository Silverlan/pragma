/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __UTIL_TGA_HPP__
#define __UTIL_TGA_HPP__

#include "pragma/networkdefinitions.h"
#include <cinttypes>
#include <vector>
#include <string>

class VFilePtrInternalReal;
namespace util
{
	namespace tga
	{
		DLLNETWORK void write_tga(std::shared_ptr<VFilePtrInternalReal> &f,uint32_t w,uint32_t h,const std::vector<uint8_t> &pixels);
		DLLNETWORK void write_tga(std::shared_ptr<VFilePtrInternalReal> &f,uint32_t w,uint32_t h,const uint8_t *pixelData);
		DLLNETWORK bool write_tga(const std::string &fileName,uint32_t w,uint32_t h,const std::vector<uint8_t> &pixels);
	};
};

#endif
