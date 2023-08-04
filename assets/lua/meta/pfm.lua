--- @meta
--- 
--- @class pfm
pfm = {}


--- 
--- @class pfm.Version
--- @field major int 
--- @field minor int 
--- @field revision int 
--- @overload fun(arg1: string):pfm.Version
--- @overload fun(arg1: int, arg2: int, arg3: int):pfm.Version
--- @overload fun(arg1: int, arg2: int):pfm.Version
--- @overload fun():pfm.Version
pfm.Version = {}

--- 
--- @return string ret0
function pfm.Version:ToString() end

--- 
function pfm.Version:Reset() end

--- 
--- @param 1 pfm.Version
function pfm.Version:__le(arg1) end

--- 
--- @param 1 pfm.Version
function pfm.Version:__eq(arg1) end

--- 
--- @param 1 pfm.Version
function pfm.Version:__lt(arg1) end

--- 
function pfm.Version:__tostring() end


