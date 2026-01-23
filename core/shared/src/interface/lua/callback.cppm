// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.callback;

export import :scripting.lua.function;

export class DLLNETWORK LuaCallback : public TCallback, public LuaFunction {
  public:
	LuaCallback(const luabind::object &o);
	virtual ~LuaCallback() override;
};
