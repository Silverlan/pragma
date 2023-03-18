-- See https://github.com/actboy168/lua-debug/blob/master/examples/standalone/debugger.lua

local function dofile(filename, ...)
    local load = _VERSION == "Lua 5.1" and loadstring or load
    print("cwd: ",io.popen"cd":read'*l')
    print("filename: ",filename)
    local f = assert(io.open(filename))
    local str = f:read "*a"
    f:close()
    return assert(load(str, "=(debugger.lua)"))(...)
end

local path = "lua/modules/lua-debug"
return dofile(path .. "/script/debugger.lua", path)
