--[[
Copyright (C) 2021 Silverlan

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

local Component = util.register_class("ents.GameAnimationRecorderComponent", BaseEntityComponent)

function Component:Initialize()
	BaseEntityComponent.Initialize(self)

	self.m_recordEntityList = {}
	self.m_entityRecordIndices = {}
end

function Component:OnRemove()
	util.remove(self.m_cbThink)
end

function Component:Reset()
	self.m_recordEntityList = {}
	self.m_entityRecordIndices = {}
	self.m_numRecordedFrames = 0
end

function Component:GetRecordedFrameCount()
	return self.m_numRecordedFrames
end

function Component:GetAnimations()
	return self.m_animations
end

function Component:RecordFrame(t)
	self.m_numRecordedFrames = self.m_numRecordedFrames + 1
	for name, animData in pairs(self.m_animations) do
		if animData.entity:IsValid() then
			self:RecordEntity(name, t, animData)
		end
	end
end

function Component:AddEntity(ent, properties)
	properties = properties or {
		["transform"] = { "position", "rotation" },
	}
	if self.m_entityRecordIndices[ent] ~= nil then
		assert(not self.m_recording)
		local record = self.m_recordEntityList[self.m_entityRecordIndices[ent]]
		for componentName, componentProps in pairs(properties) do
			if record.properties[componentName] == nil then
				record.properties[componentName] = componentProps
			else
				local propMap = {}
				for _, prop in ipairs(record.properties[componentName]) do
					propMap[prop] = true
				end
				for _, prop in ipairs(componentProps) do
					if propMap[prop] == nil then
						table.insert(record.properties[componentName], prop)
					end
				end
			end
		end
		return
	end
	table.insert(self.m_recordEntityList, {
		entity = ent,
		properties = properties,
	})
	self.m_entityRecordIndices[ent] = #self.m_recordEntityList
	if self.m_recording == true then
		self:InitializeChannels(self.m_recordEntityList[#self.m_recordEntityList])
	end
end

function Component:EndRecording()
	if self.m_recording ~= true then
		return 0
	end
	self.m_recording = nil
	util.remove(self.m_cbThink)

	local numProperties = 0
	for name, animData in pairs(self.m_animations) do
		local panimaC
		if animData.entity:IsValid() then
			panimaC = animData.entity:GetComponent(ents.COMPONENT_PANIMA)
		end
		for componentName, channels in pairs(animData.channels) do
			for propName, channelInfo in pairs(channels) do
				channelInfo.channel:Resize(channelInfo.size) -- Clear reserved, unused space
				numProperties = numProperties + 1

				if panimaC ~= nil then
					-- Re-enable property
					panimaC:SetPropertyEnabled(channelInfo.channel:GetTargetPath():ToUri(), true)
				end
			end
		end

		animData.animation:UpdateDuration()
	end
	return numProperties
end

function Component:GetAnimation()
	return self.m_animation
end
function Component:IsRecording()
	return self.m_recording
end

function Component:RecordEntity(channelName, time, entityInfo)
	if self.m_animations[channelName] == nil then
		return
	end
	local animData = self.m_animations[channelName]

	for componentName, channels in pairs(entityInfo.channels) do
		for propName, channelInfo in pairs(channels) do
			if channelInfo.component:IsValid() then
				local value = channelInfo.component:GetMemberValue(channelInfo.memberIndex)
				if value ~= nil then
					if channelInfo.size == channelInfo.capacity then
						local newSize = channelInfo.size + 10000 -- Reserve some space
						channelInfo.channel:Resize(newSize)
						channelInfo.capacity = newSize
					end
					local i = channelInfo.size
					local addValue = true
					if i >= 2 then
						local valueType = channelInfo.channel:GetValueType()
						local v0 = channelInfo.channel:GetValue(i - 2)
						local v1 = channelInfo.channel:GetValue(i - 1)
						if
							udm.compare_numeric_values(value, v0, valueType)
							and udm.compare_numeric_values(value, v1, valueType)
						then
							-- Value hasn't changed, just update the time from the last entry
							channelInfo.channel:SetTime(i - 1, time)
							addValue = false
						end
					end
					if addValue then
						channelInfo.channel:SetTime(i, time)
						channelInfo.channel:SetValue(i, value)
						channelInfo.size = channelInfo.size + 1
					end
				end
			end
		end
	end
end

function Component:InitializeChannels(entityInfo)
	if entityInfo.entity:IsValid() == false then
		return
	end
	local anim = panima.Animation.create()
	local animData = {
		entity = entityInfo.entity,
		animation = anim,
		channels = {},
	}
	local channels = animData.channels
	local panimaC = entityInfo.entity:GetComponent(ents.COMPONENT_PANIMA)
	for componentName, props in pairs(entityInfo.properties) do
		local c = entityInfo.entity:GetComponent(componentName)
		if c == nil then
			self:Log(
				"Unable to add animation channels for component '"
					.. componentName
					.. "' of entity "
					.. tostring(entityInfo.entity)
					.. ": Component does not exist!"
			)
		else
			for _, propName in ipairs(props) do
				local idx = c:GetMemberIndex(propName)
				if idx == nil then
					self:Log(
						"Unable to add animation channel for property '"
							.. propName
							.. "' of component '"
							.. componentName
							.. "' of entity "
							.. tostring(entityInfo.entity)
							.. ": Member does not exist!"
					)
				else
					local info = c:GetMemberInfo(idx)
					channels[componentName] = channels[componentName] or {}
					local targetPath = "ec/" .. componentName .. "/" .. propName
					channels[componentName][propName] = {
						size = 0,
						capacity = 0,
						channel = anim:AddChannel(targetPath, info.type),
						component = c,
						memberIndex = idx,
					}
					if panimaC ~= nil then
						-- Disable running animation for this property
						panimaC:SetPropertyEnabled(targetPath, false)
					end
				end
			end
		end
	end

	self.m_animations[entityInfo.entity:GetUuid()] = animData
end

function Component:Save(fileName)
	fileName = file.remove_file_extension(fileName, { "pgma", "pgma_b" }) .. ".pgma_b"

	local udmData = udm.create("PGMA")
	local data = udmData:GetAssetData():GetData():Get("animations")
	for name, animData in pairs(self.m_animations) do
		animData.animation:Save(data:Get(name))
	end
	return udmData:Save(fileName)
end

function Component:SaveAscii(fileName, saveFlags)
	fileName = file.remove_file_extension(fileName, { "pgma", "pgma_b" }) .. ".pgma"

	local udmData = udm.create("PGMA")
	local data = udmData:GetAssetData():GetData():Get("animations")
	for name, animData in pairs(self.m_animations) do
		animData.animation:Save(data:Get(name))
	end
	return udmData:SaveAscii(
		fileName,
		saveFlags or bit.bor(udm.ASCII_SAVE_FLAG_DEFAULT, udm.ASCII_SAVE_FLAG_BIT_DONT_COMPRESS_LZ4_ARRAYS)
	)
end

function Component:StartRecording()
	if self.m_recording == true then
		return
	end

	self.m_recording = true
	self.m_numRecordedFrames = 0
	self.m_animations = {}

	for _, entityInfo in ipairs(self.m_recordEntityList) do
		self:InitializeChannels(entityInfo)
	end

	self.m_startTime = time.real_time()
	self.m_cbThink = game.add_callback("Think", function()
		local t = time.real_time() - self.m_startTime
		self:RecordFrame(t)
	end)
end

ents.COMPONENT_GAME_ANIMATION_RECORDER =
	ents.register_component("game_animation_recorder", Component, ents.EntityComponent.FREGISTER_BIT_NETWORKED)
