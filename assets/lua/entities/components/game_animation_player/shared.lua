--[[
Copyright (C) 2021 Silverlan

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

local Component = util.register_class("ents.GameAnimationPlayerComponent", BaseEntityComponent)

function Component:Initialize()
	BaseEntityComponent.Initialize(self)

	self.m_currentTime = 0.0
	self.m_currentTimeFraction = 0.0
end

function Component:OnRemove()
	self:Reset()
end

function Component:Reset()
	if self.m_animations == nil then
		return
	end
	for uuid, anim in pairs(self.m_animations) do
		local ent = ents.iterator({ ents.IteratorFilterUuid(uuid) })()
		if ent ~= nil then
			local panima = ent:AddComponent(ents.COMPONENT_PANIMA)
			if panima ~= nil then
				panima:RemoveAnimationManager("game_animation")
			end
		end
	end

	self.m_animations = nil
end

function Component:SetPlaybackRate(playbackRate)
	for _, animManager in ipairs(self.m_animationManagers) do
		local player = animManager:GetPlayer()
		player:SetPlaybackRate(playbackRate)
	end
end

function Component:SetCurrentTime(t)
	self.m_currentTime = t
	local f = 1.0
	for _, animManager in ipairs(self.m_animationManagers) do
		local player = animManager:GetPlayer()
		player:SetCurrentTime(t)
		f = math.min(f, player:GetCurrentTimeFraction())
	end
	self.m_currentTimeFraction = f
end

function Component:GetCurrentTime()
	return self.m_currentTime
end
function Component:GetCurrentTimeFraction()
	return self.m_currentTimeFraction
end

function Component:Load(fileName)
	local baseName = file.remove_file_extension(fileName, { "pgma", "pgma_b" })
	self.m_animations = {}

	local udmData = udm.load(baseName .. ".pgma_b")
	if udmData == false then
		udmData = udm.load(baseName .. ".pgma")
	end
	if udmData == false then
		return false
	end
	local data = udmData:GetAssetData():GetData()
	local animations = data:Get("animations")
	for uuid, animData in pairs(animations:GetChildren()) do
		local anim = panima.Animation.load(animData)
		if anim ~= false then
			self.m_animations[uuid] = anim
		end
	end
	return true
end

function Component:PlayAnimation()
	if self.m_animations == nil then
		return
	end
	self.m_animationManagers = {}
	for uuid, anim in pairs(self.m_animations) do
		local ent = ents.iterator({ ents.IteratorFilterUuid(uuid) })()
		if ent ~= nil then
			local panima = ent:AddComponent(ents.COMPONENT_PANIMA)
			if panima ~= nil then
				local animManager = panima:AddAnimationManager("game_animation")
				panima:PlayAnimation(animManager, anim)
				table.insert(self.m_animationManagers, animManager)
			end
		end
	end
end
ents.COMPONENT_GAME_ANIMATION_PLAYER = ents.register_component(
	"game_animation_player",
	Component,
	bit.bor(ents.EntityComponent.FREGISTER_BIT_NETWORKED, ents.EntityComponent.FREGISTER_BIT_HIDE_IN_EDITOR)
)
