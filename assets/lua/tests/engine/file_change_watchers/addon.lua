-- SPDX-FileCopyrightText: (c) 2026 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

-- This test creates an addon to test if auto-loading of new addons works properly.

file.create_path("addons/test_change_addon/lua")

time.create_simple_timer(0.5, function()
    tests.TEST_LUA_SCRIPT_FILE_PREFIX = "addons/test_change_addon/"
    tests.TEST_LUA_SCRIPT_CLEANUP = function()
       file.delete_directory("addons/test_change_addon")
    end
    local res, msg = include("/tests/engine/file_change_watchers/lua_script.lua", true)
	if(res ~= nil) then tests.complete(res, msg) end
end)
