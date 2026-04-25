-- SPDX-FileCopyrightText: (c) 2026 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

-- This test creates a lua script, makes some changes to it and then checks if the lua script was reloaded properly.

local contents = [[test_change_value = "original"]]

local modifiedContents = [[test_change_value = "modified"]]

local testLuaFilePath = "lua/test_file_change.lua"
local function cleanup()
	test_change_value = nil
	local customCleanup = tests.TEST_LUA_SCRIPT_CLEANUP
	tests.TEST_LUA_SCRIPT_CLEANUP = nil
	tests.TEST_LUA_SCRIPT_FILE_PREFIX = nil
	if(customCleanup ~= nil) then
		customCleanup()
		return
	end
	file.delete(testLuaFilePath)
end

local prefix = tests.TEST_LUA_SCRIPT_FILE_PREFIX or ""
if(not file.write(prefix .. testLuaFilePath, contents)) then
	cleanup()
	return false, "Failed to write test lua script '" .. testLuaFilePath .. "'!"
end

local relPath = util.FilePath(testLuaFilePath)
relPath:PopFront()
-- We're intentionally loading the script twice here.
-- The first time to ensure it's added to the include cache (otherwise the file modification below would be ignored).
-- The second time to force load the script even if it was loaded before (e.g. by a previous test run).
include("/" .. relPath:GetString())
include("/" .. relPath:GetString(), true)
if(test_change_value ~= "original") then
	cleanup()
	return false, "Failed to load test lua script '" .. testLuaFilePath .. "'!"
end

-- We want to overwrite the file and trigger a 'Modified' event, but we overwrite the file
-- immediately it will just treat it as an 'Add' event, so we'll add a delay.
time.create_simple_timer(0.5, function()
	if(not file.write(prefix .. testLuaFilePath, modifiedContents)) then
		cleanup()
		return false, "Failed to write modified test lua script '" .. testLuaFilePath .. "'!"
	end

	-- Give it a bit of time to detect the change
	time.create_simple_timer(0.5, function()
		local value = test_change_value
		local expectedValue = "modified"
		if(value ~= expectedValue) then
			cleanup()
			tests.complete(false, "Value 'test_change_value' of lua check script is '" .. tostring(test_change_value) .. "', which does not match expected value '" .. expectedValue .. "'!")
			return
		end
		cleanup()
		tests.complete(true)
	end)
end)
