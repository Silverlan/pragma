#ifndef __DEBUG_LUA_ZEROBRANE_HPP__
#define __DEBUG_LUA_ZEROBRANE_HPP__

#include "pragma/networkdefinitions.h"
#include <string>
#include <cinttypes>

namespace debug
{
	DLLNETWORK void open_file_in_zerobrane(const std::string &fileName,uint32_t lineIdx);
};

#endif
