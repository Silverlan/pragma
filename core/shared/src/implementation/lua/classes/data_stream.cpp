// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_shared.h"
#include "pragma/lua/classes/ldatastream.h"

export module pragma.shared;

import :scripting.lua.classes.data_stream;

void Lua::DataStream::register_class(luabind::class_<::DataStream> &classDef) { register_class<::DataStream>(classDef); }
