--- @meta
--- 
--- @class regex
regex = {}

--- 
--- @param input string
--- @param e string
--- @param format string
--- @return string ret0
--- @overload fun(input: string, e: string, format: string, regexFlags: enum std::regex_constants::match_flag_type): string
function regex.replace(input, e, format) end


--- 
--- @class regex.Result
--- @overload fun():regex.Result
regex.Result = {}

--- 
--- @param format string
--- @param flags int
function regex.Result:SetFormat(format, flags) end

--- 
function regex.Result:HasMatch() end

--- 
function regex.Result:GetPosition() end

--- 
function regex.Result:GetString() end

--- 
function regex.Result:GetMatchCount() end

--- 
function regex.Result:__tostring() end

--- 
function regex.Result:GetLength() end


