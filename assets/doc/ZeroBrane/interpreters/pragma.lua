dofile 'interpreters/luabase.lua'
local interpreter = MakeLuaInterpreter()
interpreter.name = "Pragma"
interpreter.description = "Lua-implementation for the Pragma game engine"
-- table.insert(interpreter.api,"pragma")
interpreter.api = {"pragma"}
return interpreter