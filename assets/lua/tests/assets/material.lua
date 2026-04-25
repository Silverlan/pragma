-- SPDX-FileCopyrightText: (c) 2026 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

-- This test creates a material, loads it, and checks if the material properties were loaded correctly.

local testValues = {}
local function add_test_value(type, val, cmpFunc)
	table.insert(testValues, {
		type = type,
		value = val,
		compareFunction = cmpFunc
	})
end

local function comparef(a, b) return math.abs(a -b) < 0.0001 end
local function make_comparev(n, i)
	return function(a, b)
		for i=0, n -1 do
			if(i) then
				if(a:Get(i) ~= b:Get(i)) then return false end
			elseif(not comparef(a:Get(i), b:Get(i))) then return false end
		end
		return true;
	end
end

add_test_value(udm.TYPE_STRING, "some_value")

add_test_value(udm.TYPE_INT8, 17)
add_test_value(udm.TYPE_UINT8, 241)
add_test_value(udm.TYPE_INT16, 463)
add_test_value(udm.TYPE_UINT16, 868)
add_test_value(udm.TYPE_INT32, 185757)
add_test_value(udm.TYPE_UINT32, 756576)
--add_test_value(udm.TYPE_INT64, 75743335)
--add_test_value(udm.TYPE_UINT64, 15899456)

add_test_value(udm.TYPE_FLOAT, 15.128, comparef)
add_test_value(udm.TYPE_DOUBLE, 19.4673, comparef)
add_test_value(udm.TYPE_BOOLEAN, true)
add_test_value(udm.TYPE_VECTOR2, Vector2(1.53, 8.646), make_comparev(2))
--add_test_value(udm.TYPE_VECTOR2I, Vector2i(7537, 4246), make_comparev(2, true))
add_test_value(udm.TYPE_VECTOR3, Vector(746.544, 543.64, 421.45), make_comparev(3))
--add_test_value(udm.TYPE_VECTOR3I, Vectori(783, 769, 467), make_comparev(3, true))
add_test_value(udm.TYPE_VECTOR4, Vector4(754.54, 462.56, 6734.31, 16425.43), make_comparev(4))
--add_test_value(udm.TYPE_VECTOR4I, Vector4i(7467, 41245, 3242, 7542), make_comparev(4, true))
--add_test_value(udm.TYPE_QUATERNION, EulerAngles(32, 64, 13):ToQuaternion(), make_comparev(4))
--add_test_value(udm.TYPE_EULER_ANGLES, EulerAngles(32, 77, 121), make_comparev(3))
-- TODO: Test these types:
-- udm.TYPE_SRGBA
-- udm.TYPE_HDR_COLOR
-- udm.TYPE_TRANSFORM
-- udm.TYPE_SCALED_TRANSFORM
-- udm.TYPE_MAT4
-- udm.TYPE_MAT3X4
-- udm.TYPE_BLOB
-- udm.TYPE_BLOB_LZ4
-- udm.TYPE_ARRAY
-- udm.TYPE_ELEMENT
-- udm.TYPE_REFERENCE
-- udm.TYPE_HALF

local matProps = "\n\t"
for _, valInfo in ipairs(testValues) do
	local typeName = udm.enum_type_to_ascii(valInfo.type)
	local keyName = typeName .. "_value"
	matProps = matProps .. "\t$" .. typeName .. " " .. keyName .. " " .. udm.to_ascii_value(valInfo.value, valInfo.type) .. "\n\t"

	valInfo.key = keyName
	if(valInfo.compareFunction == nil) then
		valInfo.compareFunction = function(a, b) return a == b end
	end
end

local contents = [[
"pbr"
{
	"textures"
	{
		$string albedo_map "white"
	}
	"properties"
	{]] .. matProps .. [[
}
}
]]

local testMatFilePath = "materials/test_mat_props.pmat"
local function cleanup()
	file.delete(testMatFilePath)
end

if(not file.write(testMatFilePath, contents)) then
	cleanup()
	return false, "Failed to write test material '" .. testMatFilePath .. "'!"
end

local mat = asset.load("test_mat_props", asset.TYPE_MATERIAL)
if(not util.is_valid(mat)) then
	cleanup()
	return false, "Failed to load test material '" .. testMatFilePath .. "'!"
end

for _, valInfo in ipairs(testValues) do
	local val = mat:GetProperty(valInfo.key, valInfo.type)
	if(val == nil or not valInfo.compareFunction(val, valInfo.value)) then
		cleanup()
		return false, string.fmt("Value of material property '{}' does not match expected value '{}'.", tostring(val), valInfo.value)
	end
end

cleanup()
return true
