--- @meta
--- 
--- @class engine
engine = {}

--- 
--- @return string ret0
function engine.get_working_directory() end

--- 
--- @param record bool
function engine.set_record_console_output(record) end

--- 
--- @return int ret0
function engine.get_tick_count() end

--- 
--- @param path string
--- @return variant ret0
function engine.load_library(path) end

--- 
--- @param library string
--- @return bool ret0
function engine.library_exists(library) end

--- 
--- @return table ret0
function engine.get_git_info() end

--- 
--- @return table ret0
function engine.get_info() end

--- 
--- @return string ret0_1
--- @return enum Con::MessageFlags ret0_2
--- @return util.Color ret0_3
function engine.poll_console_output() end

--- 
function engine.shutdown() end

--- 
--- @param identifier string
--- @param fontSetName string
--- @param features enum FontSetFlag
--- @param size int
--- @return engine.FontInfo ret0
--- @overload fun(identifier: string, fontSetName: string, features: enum FontSetFlag, size: int, reload: bool): engine.FontInfo
function engine.create_font(identifier, fontSetName, features, size) end

--- 
--- @param identifier string
--- @return engine.FontInfo ret0
function engine.get_font(identifier) end

--- 
--- @param fps int
function engine.set_fixed_frame_delta_time_interpretation(fps) end

--- 
function engine.clear_fixed_frame_delta_time_interpretation() end

--- 
--- @return math.Vector2i ret0
function engine.get_window_resolution() end

--- 
--- @param tieToFrameRate bool
function engine.set_tick_delta_time_tied_to_frame_rate(tieToFrameRate) end

--- 
--- @return math.Vector2i ret0
function engine.get_render_resolution() end

--- 
--- @return prosper.RenderTarget ret0
function engine.get_staging_render_target() end

--- 
--- @return int ret0
function engine.get_current_frame_index() end

--- 
--- @param arg0 class CEngine
--- @return string ret0
function engine.get_default_font_set_name(arg0) end

--- 
--- @param text string
--- @param font engine.FontInfo
--- @return math.Vector2i ret0
--- @overload fun(text: string, font: string): math.Vector2i
function engine.get_text_size(text, font) end


--- 
--- @class engine.FontInfo
engine.FontInfo = {}


