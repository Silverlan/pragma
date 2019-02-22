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
		DLLNETWORK bool write_tga(const std::string &fileName,uint32_t w,uint32_t h,const std::vector<uint8_t> &pixels);
	};
};

#endif
