--- @meta
--- 
--- @class resource
resource = {}

--- 
--- @param res string
--- @return bool ret0
--- @overload fun(res: string, stream: bool): bool
function resource.add_file(res) end

--- 
--- @param f string
--- @return bool ret0
function resource.add_lua_file(f) end

--- 
--- @return any ret0
function resource.get_list() end


