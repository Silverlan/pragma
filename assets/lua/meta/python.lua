--- @meta
--- 
--- @class python
python = {}

--- 
--- @return bool ret0
function python.init_blender() end

--- 
function python.reload() end

--- 
--- @param fileName string
--- @param args table
--- @return bool ret0_1
--- @return string ret0_2
--- @overload fun(fileName: string): bool, string
function python.exec(fileName, args) end

--- 
--- @param code string
--- @return bool ret0_1
--- @return string ret0_2
function python.run(code) end


