--- @meta
--- 
--- @class console
console = {}

--- 
--- @param cmd string
--- @param type enum udm::Type
--- @param def struct luabind::adl::udm_type<class luabind::adl::object,1,1,1>
--- @param flags enum ConVarFlags
--- @param help string
--- @return console.Var ret0
function console.register_variable(cmd, type, def, flags, help) end

--- 
--- @param name string
--- @return console.Var ret0
function console.get_convar(name) end

--- 
--- @param arg1 string
--- @param arg2 unknown
--- @overload fun(arg1: string, arg2: unknown, arg3: string): 
--- @overload fun(arg1: string, arg2: unknown, arg3: enum ConVarFlags): 
--- @overload fun(arg1: string, arg2: unknown, arg3: enum ConVarFlags, arg4: string): 
function console.register_command(arg1, arg2) end

--- 
--- @param conVar string
--- @return bool ret0
function console.get_convar_bool(conVar) end

--- 
--- @param conVar string
--- @return int ret0
function console.get_convar_int(conVar) end

--- 
--- @param conVar string
--- @return string ret0
function console.get_convar_string(conVar) end

--- 
--- @param conVar string
--- @return number ret0
function console.get_convar_float(conVar) end

--- 
--- @param conVar string
--- @return enum ConVarFlags ret0
function console.get_convar_flags(conVar) end

--- 
--- @param src string
--- @param dst string
function console.register_override(src, dst) end

--- 
--- @param src string
function console.clear_override(src) end


--- 
--- @class console.Var
console.Var = {}

--- 
--- @return enum ConVarFlags ret0
function console.Var:GetFlags() end

--- 
--- @return int ret0
function console.Var:GetInt() end

--- 
--- @return string ret0
function console.Var:GetDefault() end

--- 
--- @return number ret0
function console.Var:GetFloat() end

--- 
--- @return bool ret0
function console.Var:GetBool() end

--- 
--- @return string ret0
function console.Var:GetHelpText() end

--- 
--- @return string ret0
function console.Var:GetString() end

--- 
--- @param arg2 unknown
function console.Var:AddChangeCallback(arg2) end

--- 
--- @return string ret0
function console.Var:GetName() end


