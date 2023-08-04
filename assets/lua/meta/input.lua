--- @meta
--- 
--- @class input
input = {}

--- 
--- @param arg0 class CEngine
--- @param arg1 string
--- @return bool ret0
function input.remove_input_binding_layer(arg0, arg1) end

--- 
--- @param key int
--- @return string ret0
function input.key_to_text(key) end

--- 
--- @param mouseButton enum GLFW::MouseButton
--- @return enum GLFW::KeyState ret0
function input.get_mouse_button_state(mouseButton) end

--- 
--- @return bool ret0
function input.is_alt_key_down() end

--- 
--- @param key enum GLFW::Key
--- @return enum GLFW::KeyState ret0
function input.get_key_state(key) end

--- 
--- @return math.Vector2 ret0
function input.get_cursor_pos() end

--- 
--- @param pos math.Vector2
function input.set_cursor_pos(pos) end

--- 
function input.center_cursor() end

--- 
--- @return bool ret0
function input.is_shift_key_down() end

--- 
--- @return bool ret0
function input.is_ctrl_key_down() end

--- 
--- @return int ret0
function input.get_controller_count() end

--- 
--- @param arg0 class CEngine
--- @param arg1 string
--- @return input.InputBindingLayer ret0
function input.get_input_binding_layer(arg0, arg1) end

--- 
--- @param arg0 class CEngine
function input.update_effective_input_bindings(arg0) end

--- 
--- @param joystickId int
--- @return string ret0
function input.get_controller_name(joystickId) end

--- 
--- @param joystickId int
--- @return table ret0
function input.get_controller_axes(joystickId) end

--- 
--- @param joystickId int
--- @return table ret0
function input.get_controller_buttons(joystickId) end

--- 
--- @param key int
--- @return string ret0
function input.key_to_string(key) end

--- 
--- @param str string
--- @return int ret0
function input.string_to_key(str) end

--- 
--- @param cvarName string
--- @param max int
--- @return table ret0
--- @overload fun(cvarName: string, max: int): table
function input.get_mapped_keys(cvarName, max) end

--- 
--- @param en class CEngine
--- @param layer input.InputBindingLayer
function input.add_input_binding_layer(en, layer) end

--- 
--- @param arg0 class CEngine
--- @return table ret0
function input.get_input_binding_layers(arg0) end

--- 
--- @param arg0 class CEngine
--- @return input.InputBindingLayer ret0
function input.get_core_input_binding_layers(arg0) end

--- 
--- @param en class CEngine
--- @param layerName string
--- @param enabled bool
function input.set_binding_layer_enabled(en, layerName, enabled) end

--- 
--- @param arg0 class CEngine
--- @return input.InputBindingLayer ret0
function input.get_effective_input_binding_layer(arg0) end

--- 
--- @param en class CEngine
--- @param layerName string
--- @return bool ret0
function input.is_binding_layer_enabled(en, layerName) end

--- 
--- @param identifier string
--- @param f unknown
--- @return util.Callback ret0
function input.add_callback(identifier, f) end


--- 
--- @class input.InputBindingLayer
--- @field identifier string 
--- @field enabled bool 
--- @field priority int 
--- @overload fun(args: string):input.InputBindingLayer
input.InputBindingLayer = {}

--- 
function input.InputBindingLayer:ClearKeyMappings() end

--- 
--- @param cmd string
--- @return table ret0
function input.InputBindingLayer:FindBoundKeys(cmd) end

--- 
function input.InputBindingLayer:ClearLuaKeyMappings() end

--- 
--- @param key string
--- @param function any
--- @overload fun(key: string, cmd: string): 
function input.InputBindingLayer:BindKey(key, function_) end

--- 
--- @param key string
function input.InputBindingLayer:UnbindKey(key) end


