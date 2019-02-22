#include "stdafx_shared.h"
#include "pragma/lua/luacallback.h"

LuaCallback::LuaCallback(const luabind::object &o)
	: 
#ifdef CALLBACK_SANITY_CHECK_ENABLED
	TCallback(std::numeric_limits<size_t>::max()),
#else
	TCallback(),
#endif
	LuaFunction(o)
{}
LuaCallback::~LuaCallback()
{}