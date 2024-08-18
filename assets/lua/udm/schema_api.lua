--[[
    Copyright (C) 2021 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

include("schema.lua")

util.register_class("udm.BaseSchemaType")
function udm.BaseSchemaType:__tostring()
	local str = self.TypeName
	if self:IsValid() == false then
		str = str["INVALID"]
	else
		if self.GetUniqueId then
			str = str .. "[" .. tostring(self:GetUniqueId()) .. "]"
		end
		if self.GetName then
			str = str .. "[" .. tostring(self:GetName()) .. "]"
		end
	end
	return str
end
function udm.BaseSchemaType:Initialize(schema, udmData, parent)
	self.m_schema = schema
	self.m_udmData = udmData
	self.m_parent = parent
	self.m_typedChildren = {}
	self.m_changeListeners = self.m_changeListeners or {}
	self.m_cachedChildren = {}

	local typeData = schema:FindTypeData(self.TypeName)
	if typeData:Get("format"):IsValid() then
		self.m_udmAssetData = udmData:Get("assetData")
	end

	for name, child in pairs(typeData:Get("children"):GetChildren()) do
		local childType = child:GetValue("type", udm.TYPE_STRING)
		if childType ~= nil then
			local schemaType = schema:FindTypeData(childType)
			if schemaType ~= nil and udm.Schema.is_enum_type(schemaType:GetValue("type", udm.TYPE_STRING)) == false then
				self.m_typedChildren[name] =
					udm.create_property_from_schema(schema, childType, self, self:GetUdmData():Get(name))
			elseif udm.is_array_type(udm.ascii_type_to_enum(childType)) then
				local childValueType = child:GetValue("valueType", udm.TYPE_STRING)
				local schemaValueType = schema:FindTypeData(childValueType)
				if
					schemaValueType ~= nil
					and udm.Schema.is_enum_type(schemaValueType:GetValue("type", udm.TYPE_STRING)) == false
				then
					self.m_typedChildren[name] = {}
					for _, aChild in ipairs(self:GetUdmData():Get(name):GetArrayValues()) do
						local prop, err = udm.create_property_from_schema(schema, childValueType, self, aChild, false)
						table.insert(self:GetTypedChildren()[name], prop)
					end
				end
			end
		end
	end
	self:OnInitialize()
end
function udm.BaseSchemaType:ReloadUdmData(udmData)
	self.m_udmData = udmData
	self.m_cachedChildren = {}
	local typeData = self.m_schema:FindTypeData(self.TypeName)
	for name, child in pairs(typeData:Get("children"):GetChildren()) do
		local childType = child:GetValue("type", udm.TYPE_STRING)
		if childType ~= nil then
			local schemaType = self.m_schema:FindTypeData(childType)
			if schemaType ~= nil and udm.Schema.is_enum_type(schemaType:GetValue("type", udm.TYPE_STRING)) == false then
				self.m_typedChildren[name]:ReloadUdmData(self:GetUdmData():Get(name))
			elseif udm.is_array_type(udm.ascii_type_to_enum(childType)) then
				local childValueType = child:GetValue("valueType", udm.TYPE_STRING)
				local schemaValueType = self.m_schema:FindTypeData(childValueType)
				if
					schemaValueType ~= nil
					and udm.Schema.is_enum_type(schemaValueType:GetValue("type", udm.TYPE_STRING)) == false
				then
					for i, aChild in ipairs(self:GetUdmData():Get(name):GetArrayValues()) do
						self:GetTypedChildren()[name][i]:ReloadUdmData(aChild)
					end
				end
			end
		end
	end
end
function udm.BaseSchemaType:Reinitialize(data)
	self:CleanUp(false)

	local oldUniqueId
	local detail = self:GetSchema():GetLibrary().detail
	local referenceables = detail.referenceables
	if self.GetUniqueId then
		oldUniqueId = tostring(self:GetUniqueId())
		detail.set_uuid_object(oldUniqueId, nil)
	end

	local curData = self:GetUdmData()
	if util.is_same_object(data, curData) == false then
		curData:Merge(data, bit.bor(udm.MERGE_FLAG_BIT_DEEP_COPY, udm.MERGE_FLAG_BIT_OVERWRITE_EXISTING))
	end
	self:Initialize(self:GetSchema(), curData, self:GetParent())
	if self.GetUniqueId then
		local newUniqueId = tostring(self:GetUniqueId())
		if referenceables[newUniqueId] ~= nil then
			console.print_warning(
				"An object with the unique id "
					.. newUniqueId
					.. " already exists, keeping original unique id "
					.. oldUniqueId
					.. "!"
			)
			newUniqueId = oldUniqueId
			self:SetUniqueId(util.Uuid(newUniqueId))
		end
		detail.set_uuid_object(newUniqueId, self)
	end
end
function udm.BaseSchemaType:ChangeUniqueId(uuid)
	if self.GetUniqueId == nil then
		return
	end
	local detail = self:GetSchema():GetLibrary().detail
	local newUniqueId = tostring(uuid)
	self:SetUniqueId(util.Uuid(newUniqueId))
	detail.set_uuid_object(newUniqueId, self)
end
function udm.BaseSchemaType:OnInitialize() end
function udm.BaseSchemaType:OnUdmChildArrayInitialized(name) end
function udm.BaseSchemaType:IsValid()
	return self:GetUdmData():IsValid()
end
function udm.BaseSchemaType:Remove()
	self:OnRemove()
end
function udm.BaseSchemaType:OnRemove()
	self:CleanUp()
end
function udm.BaseSchemaType:CleanUp(clearChangeListeners)
	if clearChangeListeners == nil then
		clearChangeListeners = true
	end
	if clearChangeListeners == true then
		for name, listeners in pairs(self.m_changeListeners) do
			util.remove(listeners)
		end
	end
	if self.GetUniqueId then
		local detail = self.m_schema:GetLibrary().detail
		detail.set_uuid_object(tostring(self:GetUniqueId()), nil)
	end
	for name, child in pairs(self.m_typedChildren) do
		if type(child) == "table" then
			for _, c in ipairs(child) do
				c:OnRemove()
			end
		else
			child:OnRemove()
		end
	end
end
function udm.BaseSchemaType:GetRootUdmData()
	return self.m_udmData
end
function udm.BaseSchemaType:GetUdmData()
	return self.m_udmAssetData or self.m_udmData
end
function udm.BaseSchemaType:GetCachedChild(name)
	if self.m_cachedChildren[name] == nil then
		-- Retrieving an UDM property can be expensive if done frequently, so we'll cache them here
		self.m_cachedChildren[name] = self:GetUdmData():Get(name)
	end
	return self.m_cachedChildren[name]
end
function udm.BaseSchemaType:GetTypedChildren()
	return self.m_typedChildren
end
function udm.BaseSchemaType:GetParent()
	return self.m_parent
end
function udm.BaseSchemaType:FindAncestor(filter)
	local parent = self:GetParent()
	if parent == nil then
		return
	end
	if filter(parent) then
		return parent
	end
	return parent:FindAncestor(filter)
end
udm.BaseSchemaType.ARRAY_EVENT_ADD = 0
udm.BaseSchemaType.ARRAY_EVENT_REMOVE = 1
udm.BaseSchemaType.ARRAY_EVENT_SET = 2
udm.BaseSchemaType.ARRAY_EVENT_ADD_RANGE = 3
udm.BaseSchemaType.ARRAY_EVENT_REMOVE_RANGE = 4
function udm.BaseSchemaType:AddChangeListener(keyName, listener)
	--[[if(self:GetUdmData():Get(keyName):IsValid() == false) then
		error("Property '" .. keyName .. "' is not a valid property!")
	end]]
	local cb = util.Callback.Create(listener)
	self.m_changeListeners[keyName] = self.m_changeListeners[keyName] or {}
	table.insert(self.m_changeListeners[keyName], cb)
	return cb
end
function udm.BaseSchemaType:OnArrayValueChanged(name, idx) end
function udm.BaseSchemaType:OnArrayValueAdded(name, idx) end
function udm.BaseSchemaType:OnArrayValueRangeAdded(name, startIndex, count) end
function udm.BaseSchemaType:OnArrayValueRangeRemoved(name, startIndex, count) end
function udm.BaseSchemaType:OnArrayValueRemoved(name, idx) end
local function get_getter_name(udmChildData, name)
	local nameUpper = name:sub(1, 1):upper() .. name:sub(2)
	return udmChildData:GetValue("getterName", udm.TYPE_STRING) or ("Get" .. nameUpper)
end
local function get_setter_name(udmChildData, name)
	local nameUpper = name:sub(1, 1):upper() .. name:sub(2)
	return udmChildData:GetValue("setterName", udm.TYPE_STRING) or ("Set" .. nameUpper)
end
local function get_property_type(udmChildData)
	local stype = udmChildData:GetValue("type", udm.TYPE_STRING)
	if stype == nil then
		local default = udmChildData:Get("default")
		if default:IsValid() then
			stype = udm.enum_type_to_ascii(default:GetType())
		end
	end
	return stype
end
function udm.BaseSchemaType:GetPropertyType(keyName)
	local schema = self:GetSchema()
	local schemaType = schema:FindTypeData(self.TypeName)
	if schemaType ~= nil then
		local propSchemaData = schemaType:Get("children"):Get(keyName)
		if propSchemaData ~= nil then
			return get_property_type(propSchemaData)
		end
	end
end
function udm.BaseSchemaType:GetPropertyUdmType(keyName)
	local type = self:GetPropertyType(keyName)
	if type == nil then
		return udm.TYPE_INVALID
	end
	local schema = self:GetSchema()
	local schemaType = schema:FindTypeData(type)
	if schemaType ~= nil then
		if udm.Schema.is_enum_type(schemaType:GetValue("type", udm.TYPE_STRING)) then
			return udm.TYPE_INT32
		end
	end
	return udm.ascii_type_to_enum(type)
end
function udm.BaseSchemaType:GetPropertyValue(keyName)
	local schema = self:GetSchema()
	local schemaType = schema:FindTypeData(self.TypeName)
	if schemaType ~= nil then
		local propSchemaData = schemaType:Get("children"):Get(keyName)
		if propSchemaData ~= nil then
			local getterName = get_getter_name(propSchemaData, keyName)
			if self[getterName] ~= nil then
				return self[getterName](self)
			end
		end
	end
	return self:GetUdmData():GetValue(keyName)
end
function udm.BaseSchemaType:SetPropertyValue(keyName, value, type)
	local schema = self:GetSchema()
	local schemaType = schema:FindTypeData(self.TypeName)
	if schemaType ~= nil then
		local propSchemaData = schemaType:Get("children"):Get(keyName)
		if propSchemaData ~= nil then
			local setterName = get_setter_name(propSchemaData, keyName)
			if self[setterName] ~= nil then
				return self[setterName](self, value)
			end
		end
	end
	return self:GetUdmData():SetValue(keyName, type or self:GetPropertyUdmType(keyName), value)
end
function udm.BaseSchemaType:CallChangeListeners(keyName, newValue, ...)
	if self.m_changeListeners[keyName] == nil then
		return
	end
	local i = 1
	local listeners = self.m_changeListeners[keyName]
	while i <= #listeners do
		local cb = listeners[i]
		if cb:IsValid() then
			cb:Call(self, newValue, ...)
			i = i + 1
		else
			table.remove(listeners, i)
		end
	end
end
function udm.BaseSchemaType:GetSchema()
	return self.m_schema
end
function udm.BaseSchemaType:Copy(parent)
	local el = udm.create_element()
	el:Merge(self:GetRootUdmData(), udm.MERGE_FLAG_BIT_DEEP_COPY)
	return udm.create_property_from_schema(self:GetSchema(), self.TypeName, parent, el, false)
end

function udm.create_property_from_schema(schema, type, parent, el, populate)
	if util.is_valid(el) == false or populate == true then
		el = el or udm.create_element()
		local res, err = schema:InitializeType(el, type)
		if res ~= true then
			return false, err
		end
	end
	local obj = schema:GetLibrary()[type]()
	obj:Initialize(schema, el, parent)
	if obj.GetUniqueId then
		local detail = schema:GetLibrary().detail
		detail.set_uuid_object(tostring(obj:GetUniqueId()), obj)
	end
	return obj
end

local function get_enum_name(name, valName)
	local baseName = name .. valName:sub(1, 1):upper() .. valName:sub(2)
	local enumName = ""
	for i = 1, #baseName do
		local c = baseName:sub(i, i)
		if c == c:upper() then
			if #enumName > 0 then
				enumName = enumName .. "_"
			end
		else
			c = c:upper()
		end
		enumName = enumName .. c
	end
	return enumName
end

function udm.generate_lua_api_from_schema(schema)
	local lib = schema:GetLibrary()
	lib.detail = lib.detail or {}
	lib.detail.referenceables = lib.detail.referenceables or {}
	lib.detail.set_uuid_object = function(uuid, o)
		if o ~= nil and lib.detail.referenceables[uuid] ~= nil then
			if util.is_same_object(o, lib.detail.referenceables[uuid]) then
				return
			end
			-- Note: This error check is currently disabled, because UUIDs currently do not get cleaned up
			-- when a project is unloaded.
			--[[error(
				"Attempted to assign uuid "
					.. tostring(uuid)
					.. " to object "
					.. tostring(o)
					.. ", but uuid is already assigned to object "
					.. tostring(lib.detail.referenceables[uuid])
					.. "!"
			)]]
		end
		lib.detail.referenceables[uuid] = o
	end
	for name, udmType in pairs(schema:GetUdmData():GetChildren("types")) do
		local schemaType = udmType:GetValue("type", udm.TYPE_STRING)
		if udm.Schema.is_enum_type(schemaType) == false then
			local class = util.register_class(lib, name, udm.BaseSchemaType)

			class.create = function(schema, parent)
				local el = udm.create_element()
				return udm.create_property_from_schema(schema, class.TypeName, parent, el, true)
			end
		else
			local values = udmType:GetArrayValues("values", udm.TYPE_STRING)
			if schemaType == "enum" or schemaType == "enum_flags" then
				lib.detail = lib.detail or {}
				lib.detail.enumSets = lib.detail.enumSets or {}
				lib.detail.enumSets[name] = lib.detail.enumSets[name] or {}
				if schemaType == "enum" then
					for i, valName in ipairs(values) do
						local val = i - 1
						lib[get_enum_name(name, valName)] = val

						lib.detail.enumSets[name][valName] = val
						lib.detail.enumSets[name][val] = valName
					end
				elseif schemaType == "enum_flags" then
					for valName, val in pairs(udmType:Get("values"):GetChildren()) do
						val = val:GetValue()
						lib[get_enum_name(name, valName)] = val

						lib.detail.enumSets[name][valName] = val
						lib.detail.enumSets[name][val] = valName
					end
				end
			else
				error("Invalid schema type '" .. schemaType .. "'!")
			end
		end
	end
	for name, udmType in pairs(schema:GetUdmData():GetChildren("types")) do
		local schemaType = udmType:GetValue("type", udm.TYPE_STRING)
		if udm.Schema.is_enum_type(schemaType) == false then
			local class = lib[name]
			class.TypeName = name
			for name, udmChild in pairs(udmType:GetChildren("children")) do
				local getterName = get_getter_name(udmChild, name)
				local setterName = get_setter_name(udmChild, name)
				local stype = udmChild:GetValue("type", udm.TYPE_STRING)
				local udmType
				if stype == nil then
					local default = udmChild:Get("default")
					if default:IsValid() then
						udmType = default:GetType()
					end
				end
				if udmType == nil then
					udmType = udm.ascii_type_to_enum(stype)
				end
				if udmType == nil or udmType == udm.TYPE_INVALID then
					if stype == "Any" then
						-- TODO
					elseif stype == "Uuid" then
						class[getterName] = function(self)
							return util.Uuid(self:GetUdmData():GetValue(name, udm.TYPE_STRING))
						end
						class[setterName] = function(self, value)
							self:GetUdmData():SetValue(name, udm.TYPE_STRING, tostring(value))
							self:CallChangeListeners(
								name,
								(util.get_type_name(value) == "Uuid") and value or util.Uuid(value)
							)
						end
					elseif stype == "Reference" then
						class[getterName .. "Id"] = function(self)
							return util.Uuid(self:GetUdmData():GetValue(name, udm.TYPE_STRING))
						end
						class[setterName .. "Id"] = function(self, value)
							self:GetUdmData():SetValue(name, udm.TYPE_STRING, tostring(value))
							self:CallChangeListeners(
								name,
								(util.get_type_name(value) == "Uuid") and value or util.Uuid(value)
							)
						end
						class[getterName] = function(self)
							return lib.detail.referenceables[self:GetUdmData():GetValue(name, udm.TYPE_STRING)]
						end
						class[setterName] = function(self, value)
							local type = util.get_type_name(value)
							if type == "Uuid" or type == "string" then
								if type == "Uuid" then
									value = tostring(value)
								end
								local o = lib.detail.referenceables[value]
								if o == nil then
									error("Uuid '" .. value .. "' does not refer to known object!")
								end
							else
								value = tostring(value:GetUniqueId())
							end
							self:GetUdmData():SetValue(name, udm.TYPE_STRING, value)
							self:CallChangeListeners(name, util.Uuid(value))
						end
					else
						-- Check classes
						local childClass = lib[stype]
						if childClass ~= nil then
							class[getterName] = function(self)
								return self:GetTypedChildren()[name]
							end
						else
							local schemaType = schema:FindTypeData(stype)
							if schemaType ~= nil then
								local specType = schemaType:GetValue("type", udm.TYPE_STRING)
								if udm.Schema.is_enum_type(specType) then
									local underlyingType = schemaType:GetValue("underlyingType", udm.TYPE_STRING)
										or "int32"
									local udmUnderlyingType = udm.ascii_type_to_enum(underlyingType)
									if udmUnderlyingType == nil or udmUnderlyingType == udm.TYPE_INVALID then
										error(
											"Invalid underlying type '"
												.. underlyingType
												.. "' used for enum '"
												.. stype
												.. "'!"
										)
									else
										class[getterName .. "Name"] = function(self)
											return self:GetUdmData():GetValue(name, udm.TYPE_STRING)
										end
										if specType == "enum" then
											class[getterName] = function(self)
												local value = self:GetUdmData():GetValue(name, udm.TYPE_STRING)
												return lib.detail.enumSets[stype][value]
											end
											class[setterName] = function(self, val)
												if type(val) == "number" then
													local inVal = val
													val = lib.detail.enumSets[stype][val]
													if val == nil then
														error(
															"Not a valid enum! (Type: "
																.. tostring(stype)
																.. ", val: "
																.. tostring(inVal)
																.. ")"
														)
													end
												end
												if lib.detail.enumSets[stype][val] == nil then
													error(
														"Not a valid enum! (Type: "
															.. tostring(stype)
															.. ", val: "
															.. tostring(val)
															.. ")"
													)
												end
												self:GetUdmData():SetValue(name, udm.TYPE_STRING, val)
												self:CallChangeListeners(name, val)
											end
										elseif specType == "enum_flags" then
											class[getterName] = function(self)
												local value = self:GetUdmData():GetValue(name, udm.TYPE_STRING)
												local bitValues = 0
												for _, strVal in ipairs(string.split(value, "|")) do
													local val = lib.detail.enumSets[stype][strVal]
													if val == nil then
														error(
															"Not a valid enum! (Type: "
																.. tostring(stype)
																.. ", val: "
																.. tostring(strVal)
																.. ")"
														)
													end
													bitValues = bit.bor(bitValues, val)
												end
												return bitValues
											end
											class[setterName] = function(self, val)
												local str = ""
												for _, v in ipairs(math.get_power_of_2_values(val)) do
													if #str > 0 then
														str = str .. "|"
													end
													str = str .. lib.detail.enumSets[stype][v]
												end
												self:GetUdmData():SetValue(name, udm.TYPE_STRING, str)
												self:CallChangeListeners(name, val)
											end
										end
									end
								end
							end
						end
					end
				else
					if udm.is_array_type(udmType) then
						local baseName = udmChild:GetValue("baseName", udm.TYPE_STRING)
						if baseName == nil then
							baseName = name:sub(1, 1):upper() .. name:sub(2, #name - 1)
						end
						local valueType = udmChild:GetValue("valueType", udm.TYPE_STRING)
						local schemaValueType = schema:FindTypeData(valueType)
						local udmValueType = udm.ascii_type_to_enum(valueType)
						if
							schemaValueType ~= nil
							and udm.Schema.is_enum_type(schemaValueType:GetValue("type", udm.TYPE_STRING))
						then
							schemaValueType = nil
							udmValueType = udm.TYPE_UINT32
						end
						local elementGetterName = "Get" .. baseName
						if elementGetterName ~= nil then
							class[elementGetterName] = function(self, i)
								if schemaValueType ~= nil then
									return self:GetTypedChildren()[name][i + 1]
								end
								local child = self:GetCachedChild(name)
								if child == nil then
									return
								end
								return child:GetArrayValueDirect(i)
							end
						end

						if schemaValueType == nil then
							if valueType == "Any" then
								local setValueTypeName = "Set"
									.. (name:sub(1, 1):upper() .. name:sub(2, #name))
									.. "ValueType"
								if setValueTypeName ~= nil then
									class[setValueTypeName] = function(self, valueType)
										local udmValue = self:GetUdmData():Get(name)
										if udmValue:IsValid() == false then
											self:GetUdmData():AddArray(
												name,
												0,
												valueType,
												(udmType == udm.TYPE_ARRAY_LZ4) and udm.ARRAY_TYPE_COMPRESSED
													or udm.ARRAY_TYPE_RAW
											)
											self:OnUdmChildArrayInitialized(name)
										else
											self:GetUdmData():Get(name):SetValueType(valueType)
										end
									end
								end
							end
							local elementSetterName = "Set" .. baseName
							if elementSetterName ~= nil then
								class[elementSetterName] = function(self, i, value)
									local child = self:GetCachedChild(name)
									if child ~= nil then
										local type = udmValueType
										if type == udm.TYPE_INVALID then
											type = child:GetValueType()
										end
										child:SetValue(i, type, value)
									end
									self:OnArrayValueChanged(name, i)
									self:CallChangeListeners(name, i, udm.BaseSchemaType.ARRAY_EVENT_SET)
								end
							end
						end

						local adderName = "Add" .. baseName
						if adderName ~= nil then
							class[adderName] = function(self, value)
								local el = self:GetCachedChild(name)
								if el == nil then
									return
								end
								local newSize = el:GetSize() + 1
								el:Resize(newSize)
								if schemaValueType ~= nil then
									local child = el:Get(el:GetSize() - 1)
									local prop, err =
										udm.create_property_from_schema(schema, valueType, self, child, true)
									table.insert(self:GetTypedChildren()[name], prop)
									self:OnArrayValueAdded(name, newSize - 1)
									self:CallChangeListeners(name, newSize - 1, udm.BaseSchemaType.ARRAY_EVENT_ADD)
									return prop
								end
								el:SetValue(el:GetSize() - 1, el:GetValueType(), value)
							end
						end

						local insertValueRangeName = "Insert" .. baseName .. "Range"
						if insertValueRangeName ~= nil then
							class[insertValueRangeName] = function(self, startIndex, count)
								local el = self:GetCachedChild(name)
								if el == nil then
									return
								end
								el:AddValueRange(startIndex, count)
								if schemaValueType ~= nil then
									local children = self:GetTypedChildren()[name]
									for i = startIndex, startIndex + count - 1 do
										local child = el:Get(i)
										local prop, err =
											udm.create_property_from_schema(schema, valueType, self, child, true)
										table.insert(self:GetTypedChildren()[name], i + 1, prop)
									end

									-- Update UDM data for typed children (due to index change)
									for i = startIndex + count, el:GetSize() - 1 do
										local child = children[i]
										child.m_udmData = el:Get(i - 1)
									end
								end
								self:OnArrayValueRangeAdded(name, startIndex, count)
								self:CallChangeListeners(
									name,
									nil,
									udm.BaseSchemaType.ARRAY_EVENT_ADD_RANGE,
									startIndex,
									count
								)
							end
						end

						local countName = "Get" .. baseName .. "Count"
						local removeValueRangeName = "Remove" .. baseName .. "Range"
						if removeValueRangeName ~= nil then
							class[removeValueRangeName] = function(self, startIndex, count)
								local el = self:GetCachedChild(name)
								if el == nil then
									return
								end
								if schemaValueType ~= nil then
									local children = self:GetTypedChildren()[name]
									for i = startIndex, startIndex + count - 1 do
										children[i]:OnRemove()
									end
								end
								el:RemoveValueRange(startIndex, count)
								if schemaValueType ~= nil then
									local children = self:GetTypedChildren()[name]
									-- Update UDM data for typed children (due to index change)
									local size = el:GetSize()
									for i = startIndex, size - 1 do
										local child = children[i]
										child.m_udmData = el:Get(i - 1)
									end

									while #children > size do
										children[#children] = nil
									end
								end
								self:OnArrayValueRangeRemoved(name, startIndex, count)
								self:CallChangeListeners(
									name,
									nil,
									udm.BaseSchemaType.ARRAY_EVENT_REMOVE_RANGE,
									startIndex,
									count
								)
							end
							local clearName = "Clear" .. baseName .. "s"
							class[clearName] = function(self)
								local n = class[countName](self)
								class[removeValueRangeName](self, 0, n)
							end
						end

						local removerName = "Remove" .. baseName
						if removerName ~= nil then
							class[removerName] = function(self, idx)
								if udmValueType == udm.TYPE_INVALID and type(idx) ~= "number" then
									local found = false
									for i, val in ipairs(class[getterName](self)) do
										if util.is_same_object(val, idx) then
											idx = i - 1
											found = true
											break
										end
									end
									if found == false then
										return false
									end
								end
								local a = self:GetCachedChild(name)
								if a == nil or idx >= a:GetSize() then
									return false
								end
								local children = self:GetTypedChildren()[name]
								local child = children[idx + 1]
								local curVal = self[elementGetterName](self, idx)
								child:OnRemove()
								a:RemoveValue(idx)
								table.remove(children, idx + 1)

								-- Update UDM data for typed children (due to index change)
								local udmData = self:GetUdmData():Get(name)
								for i = idx + 1, #children do
									local child = children[i]
									child:ReloadUdmData(udmData:Get(i - 1))
								end
								self:OnArrayValueRemoved(name, idx)
								self:CallChangeListeners(name, idx, udm.BaseSchemaType.ARRAY_EVENT_REMOVE, curVal)
								return true
							end
						end

						if countName ~= nil then
							class[countName] = function(self, idx)
								local a = self:GetCachedChild(name)
								return (a ~= nil) and a:GetSize() or 0
							end
						end

						class["Get" .. baseName .. "ArrayValueType"] = function(self)
							return self:GetUdmData():Get(name):GetValueType()
						end
						if udmValueType ~= udm.TYPE_INVALID or schemaValueType ~= nil then
							class[getterName] = function(self)
								if schemaValueType ~= nil then
									return self:GetTypedChildren()[name]
								end
								return self:GetUdmData():GetArrayValues(name, udmValueType)
							end
						else
							class[getterName] = function(self)
								return self:GetUdmData()
									:GetArrayValues(name, self:GetUdmData():Get(name):GetValueType())
							end
						end
					elseif udmType == udm.TYPE_ELEMENT then
						class[getterName] = function(self)
							return self:GetUdmData():Get(name)
						end
					else
						class[getterName] = function(self)
							return self:GetUdmData():GetValue(name, udmType)
						end
						local optional = udmChild:GetValue("optional", udm.TYPE_BOOLEAN) or false
						class[setterName] = function(self, value)
							if optional and value == nil then
								self:GetUdmData():RemoveValue(name)
							else
								self:GetUdmData():SetValue(name, udmType, value)
							end
							self:CallChangeListeners(name, value)
						end
					end
				end
			end
		end
	end
	return true
end
