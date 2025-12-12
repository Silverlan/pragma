// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :scripting.lua.classes.data_stream;

void Lua::DataStream::register_class(luabind::class_<pragma::util::DataStream> &classDef) { register_class<pragma::util::DataStream>(classDef); }
