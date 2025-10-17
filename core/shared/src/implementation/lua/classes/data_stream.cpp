// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/luaapi.h"

module pragma.shared;

import :scripting.lua.classes.data_stream;

void Lua::DataStream::register_class(luabind::class_<::DataStream> &classDef) { register_class<::DataStream>(classDef); }
