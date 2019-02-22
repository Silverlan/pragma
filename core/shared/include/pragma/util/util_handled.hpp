#ifndef __UTIL_HANDLED_HPP__
#define __UTIL_HANDLED_HPP__

#include <cinttypes>

namespace util
{
	enum class EventReply : uint8_t
	{
		Unhandled = 0,
		Handled
	};
};

#endif
