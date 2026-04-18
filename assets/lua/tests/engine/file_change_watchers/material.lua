-- SPDX-FileCopyrightText: (c) 2026 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

-- This test creates a material, makes some changes to it and then checks if the material was reloaded properly.

local contents = [[
"pbr"
{
	"textures"
	{
		$string albedo_map "white"
	}
	"properties"
	{
		$string test_change_value "original"
	}
}
]]

local modifiedContents = [[
"pbr"
{
	"textures"
	{
		$string albedo_map "white"
	}
	"properties"
	{
		$string test_change_value "modified"
	}
}
]]

local checkKeyName = "test_change_value"

local testMatFilePath = "materials/test_file_change.pmat"
local function cleanup()
	file.delete(testMatFilePath)
end

if(not file.write(testMatFilePath, contents)) then
	cleanup()
	return false, "Failed to write test material '" .. testMatFilePath .. "'!"
end

local res = asset.load("test_file_change",asset.TYPE_MATERIAL)
if(not util.is_valid(res)) then
	cleanup()
	return false, "Failed to load test material '" .. testMatFilePath .. "'!"
end

-- We want to overwrite the file and trigger a 'Modified' event, but we overwrite the file
-- immediately it will just treat it as an 'Add' event, so we'll add a delay.
time.create_simple_timer(0.5, function()
	if(not file.write(testMatFilePath, modifiedContents)) then
		cleanup()
		return false, "Failed to write modified test material '" .. testMatFilePath .. "'!"
	end

	-- Give it a bit of time to detect the change
	time.create_simple_timer(0.5, function()
		if(not res:IsValid()) then
			cleanup()
			tests.complete(false, "Test material '" .. testMatFilePath .. "' is no longer valid!")
			return
		end

		local value = res:GetProperty(checkKeyName, udm.TYPE_STRING)
		local expectedValue = "modified"
		if(value ~= expectedValue) then
			cleanup()
			tests.complete(false, "Value of material property '" .. checkKeyName .. "' is '" .. tostring(value) .. "', which does not match expected value '" .. expectedValue .. "'!")
			return
		end
		cleanup()
		tests.complete(true)
	end)
end)
