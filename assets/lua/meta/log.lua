--- @meta
--- 
--- @class log
log = {}

--- 
--- @param severity enum util::LogSeverity
--- @return bool ret0
function log.is_log_level_enabled(severity) end

--- 
function log.flush_loggers() end

--- 
--- @param level enum util::LogSeverity
function log.set_console_log_level(level) end

--- 
--- @return enum util::LogSeverity ret0
function log.get_console_log_level() end

--- 
--- @param level enum util::LogSeverity
function log.set_file_log_level(level) end

--- 
--- @return enum util::LogSeverity ret0
function log.get_file_log_level() end


