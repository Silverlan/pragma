-- SPDX-FileCopyrightText: (c) 2026 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

-- This test creates a localization file, makes some changes to it and then checks if the localization file was reloaded properly.

locale.change_language("en")

local contents = [[
test_change_locale = "Original"
]]

local modifiedContents = [[
test_change_locale = "Modified"
]]

local checkKeyName = "test_change_locale"

local localeFileName = "test_locale_change.txt"
local testLocaleFilePath = "scripts/localization/en/texts/" .. localeFileName
local function cleanup()
	file.delete(testLocaleFilePath)
end

if(not file.write(testLocaleFilePath, contents)) then
	cleanup()
	return false, "Failed to write test localization file '" .. testLocaleFilePath .. "'!"
end

if(not locale.load("test_locale_change.txt")) then
	cleanup()
	return false, "Failed to load test localization file '" .. localeFileName .. "'!"
end

-- We want to overwrite the file and trigger a 'Modified' event, but we overwrite the file
-- immediately it will just treat it as an 'Add' event, so we'll add a delay.
time.create_simple_timer(0.5, function()
	if(not file.write(testLocaleFilePath, modifiedContents)) then
		cleanup()
		return false, "Failed to write modified test localization file '" .. testLocaleFilePath .. "'!"
	end

	-- Give it a bit of time to detect the change
	time.create_simple_timer(0.5, function()
		local value = locale.get_text(checkKeyName)
		local expectedValue = "Modified"
		if(value ~= expectedValue) then
			cleanup()
			tests.complete(false, "Value of locale '" .. checkKeyName .. "' is '" .. tostring(value) .. "', which does not match expected value '" .. expectedValue .. "'!")
			return
		end
		cleanup()
		tests.complete(true)
	end)
end)
