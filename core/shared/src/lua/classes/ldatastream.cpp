#include "stdafx_shared.h"
#include "pragma/lua/classes/ldatastream.h"

void Lua::DataStream::register_class(luabind::class_<::DataStream> &classDef)
{
	register_class<::DataStream>(classDef);
}
