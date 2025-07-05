-- SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

dofile 'interpreters/luabase.lua'
local interpreter = MakeLuaInterpreter()
interpreter.name = "Pragma"
interpreter.description = "Lua-implementation for the Pragma game engine"
-- table.insert(interpreter.api,"pragma")
interpreter.api = {"pragma"}
return interpreter