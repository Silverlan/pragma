--[[
	Copyright (C) 2021 Silverlan

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

local function load_udm(fileName,includeCache)
	includeCache = includeCache or {}
	local filePath = "udm/schemas/" .. fileName

	local nFilePath = util.Path.CreateFilePath(filePath)
	nFilePath:RemoveFileExtension({"udms","udms_b"})
	nFilePath = nFilePath:GetString():lower()
	if(includeCache[nFilePath] ~= nil) then return end -- Already included
	includeCache[nFilePath] = true

	local udmData,err = udm.load(nFilePath .. ".udms")
	if(udmData == false) then udmData,err = udm.load(nFilePath .. ".udms_b") end
	if(udmData == false) then return false,err end
	local assetData = udmData:GetAssetData()
	local data = assetData:GetData()
	local udmSchema = data:Get("schema")

	for _,include in ipairs(udmSchema:GetArrayValues("includes",udm.TYPE_STRING)) do
		local res,err = load_udm(include,includeCache)
		if(res ~= nil) then
			if(res == false) then return false,"Failed to load include '" .. include .. "': " .. err end
			udmSchema:Merge(res:Get("schema"))
		end
	end
	return data:ClaimOwnership()
end

local function is_enum_type(schemaType) return schemaType == "enum" or schemaType == "enum_flags" end
local function initialize_udm_data_from_schema_object(udmData,udmSchemaType,udmSchema)
	local udmAsset = udmSchemaType:Get("format")
	if(udmAsset:IsValid()) then
		local version = udmAsset:GetValue("version",udm.TYPE_UINT32)
		local type = udmAsset:GetValue("type",udm.TYPE_STRING)
		udmData:SetValue("assetType",udm.TYPE_STRING,type)
		udmData:SetValue("assetVersion",udm.TYPE_UINT32,version)
		udmData = udmData:Add("assetData")
	end

	local udmSchemaTypes = udmSchema:GetUdmData():Get("types")
	for name,udmSchemaChild in pairs(udmSchemaType:Get("children"):GetChildren()) do
		local type = udmSchemaChild:GetValue("type",udm.TYPE_STRING)
		local default = udmSchemaChild:Get("default")
		if(type == nil) then
			if(default:IsValid() == false) then error("Property '" .. name .. "' in UDM schema has neither type nor default value!") end
			type = udm.enum_type_to_ascii(default:GetType())
		end

		local udmValue = udmData:Get(name)
		local udmSchemaChildType = udmSchemaTypes:Get(type)
		if(udmSchemaChildType:IsValid()) then
			local schemaChildType = udmSchemaChildType:GetValue("type",udm.TYPE_STRING)
			if(is_enum_type(schemaChildType)) then
				if(default:IsValid() == false) then error("Enum property '" .. name .. "' of type '" .. type .. "' in UDM schema has no default value!") end
				local value = default:GetValue(udm.TYPE_STRING)
				local enumSet = udmSchema:GetEnumSet(type)
				if(enumSet == nil) then error("Enum value '" .. value .. "' of property '" .. name .. "' of type '" .. type .. "' references unknown enum set!") end
				local ivalue = enumSet[value]
				if(ivalue == nil) then error("Enum value '" .. value .. "' of property '" .. name .. "' of type '" .. type .. "' is not a valid enum!") end
				udmData:SetValue(name,udm.TYPE_STRING,value)
			elseif(schemaChildType ~= nil) then
				error("Unknown schema element type '" .. schemaChildType .. "'!")
			else
				if(udmValue:IsValid() == false) then udmValue = udmData:Add(name,udm.TYPE_ELEMENT) end
				local res,msg = initialize_udm_data_from_schema_object(udmValue,udmSchemaChildType,udmSchema)
				if(res ~= true) then return res,msg end
			end
		elseif(type == "Reference") then
			udmData:SetValue(name,udm.TYPE_STRING,"")
		elseif(type == "Uuid") then
			udmData:SetValue(name,udm.TYPE_STRING,tostring(util.generate_uuid_v4()))
		else
			local udmType = udm.ascii_type_to_enum(type)
			if(udmType == nil or udmType == udm.TYPE_INVALID) then error("Type '" .. type .. "' of property '" .. name .. "' is not a known type!") end
			if(udm.is_array_type(udmType)) then
				local valueType = udmSchemaChild:GetValue("valueType",udm.TYPE_STRING)
				if(valueType == nil) then error("Property '" .. name .. "' is array type, but no value type has been specified for array!") end
				local udmSchemaValueType = udmSchemaTypes:Get(valueType)
				local udmValueType
				if(udmSchemaValueType:IsValid()) then
					if(is_enum_type(udmSchemaValueType:GetValue("type",udm.TYPE_STRING))) then udmValueType = udm.TYPE_UINT32
					else udmValueType = udm.TYPE_ELEMENT end
				else udmValueType = udm.ascii_type_to_enum(valueType) end
				if(valueType ~= "Any") then -- "any" is a special case where we don't know the actual type yet
					if(udmValueType == nil or udmValueType == udm.TYPE_INVALID) then error("Property '" .. name .. "' is array type, but specified value type '" .. valueType .. "' is not a known type!") end
					udmData:AddArray(name,0,udmValueType,(udmType == udm.TYPE_ARRAY_LZ4) and udm.ARRAY_TYPE_COMPRESSED or udm.ARRAY_TYPE_RAW)
				end
			elseif(udmType == udm.TYPE_ELEMENT) then
				udmData:Add(name)
			else
				-- Initialize value with default
				if(default:IsValid() == false) then
					local opt = udmSchemaChild:Get("optional") or false
					if(opt == false) then error("Missing default value for UDM type '" .. type .. "' of property '" .. name .. "'!") end
					udmData:SetValue(name,udm.TYPE_NIL,nil)
				else udmData:SetValue(name,udmType,default:GetValue()) end
			end
		end
	end
	return true
end

local function initialize_udm_data_from_schema(udmData,udmSchema,baseType)
	local udmTypes = udmSchema:GetUdmData():GetChildren("types")
	local udmSchemaBaseType = udmTypes[baseType]
	if(udmSchemaBaseType == nil) then
		error("Type '" .. baseType .. "' is not a known type!")
	end
	return initialize_udm_data_from_schema_object(udmData,udmSchemaBaseType,udmSchema)
end

util.register_class("udm.Schema")
function udm.Schema:__init()
	self.m_seed = 0
end
function udm.Schema:GetUdmData() return self.m_udmData:Get("schema") end
function udm.Schema:GetEnumSet(name) return self.m_enumSets[name] end
function udm.Schema:FindTypeData(type,includeEnumTypes)
	if(includeEnumTypes == nil) then includeEnumTypes = true end
	local prop = self:GetUdmData():Get("types"):Get(type)
	if(prop:IsValid() == false) then return end
	if(includeEnumTypes == false) then
		local type = prop:GetValue("type",udm.TYPE_STRING)
		if(is_enum_type(type)) then return end
	end
	return prop
end
function udm.Schema:InitializeType(udmData,type)
	return initialize_udm_data_from_schema(udmData,self,type)
end
function udm.Schema:SetSeed(seed) self.m_seed = seed end
function udm.Schema:GetSeed() return self.m_seed end
function udm.Schema:SetLibrary(lib) self.m_library = lib end
function udm.Schema:GetLibrary() return self.m_library end

udm.dereference = function(schema,uid)
	local lib = schema:GetLibrary()
	if(lib.detail == nil or lib.detail.referenceables == nil) then return end
	return lib.detail.referenceables[uid]
end

udm.Schema.is_enum_type = is_enum_type
udm.Schema.load = function(fileName)
	local udmData,msg = load_udm(fileName)
	if(udmData == false) then return false,msg end
	local schema = udm.Schema()
	schema.m_udmData = udmData
	schema.m_enumSets = {}

	local udmSchemaTypes = schema:GetUdmData():Get("types")
	for name,udmChild in pairs(udmSchemaTypes:GetChildren()) do
		local type = udmChild:GetValue("type",udm.TYPE_STRING)
		if(is_enum_type(type)) then
			if(type == "enum") then
				local values = udmChild:GetArrayValues("values",udm.TYPE_STRING)
				local enumSet = {}
				for i,name in ipairs(values) do
					enumSet[name] = i -1
				end

				schema.m_enumSets[name] = enumSet
			elseif(type == "enum_flags") then
				local enumSet = {}
				for name,val in pairs(udmChild:Get("values"):GetChildren()) do
					enumSet[name] = val:GetValue()
				end

				schema.m_enumSets[name] = enumSet
			end
		end
	end
	return schema
end
