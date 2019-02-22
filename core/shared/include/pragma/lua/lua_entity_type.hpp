#ifndef __LUA_ENTITY_TYPE_HPP__
#define __LUA_ENTITY_TYPE_HPP__

#include <cinttypes>

enum class LuaEntityType : uint8_t
{
	NetworkLocal = 0u,
	Shared
};

#endif
