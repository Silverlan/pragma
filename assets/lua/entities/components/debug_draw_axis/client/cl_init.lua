--[[
    Copyright (C) 2024 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

local Component = util.register_class("ents.DebugDrawAxis", BaseEntityComponent)
function Component:Initialize()
	BaseEntityComponent.Initialize(self)

	self:SetTickPolicy(ents.TICK_POLICY_ALWAYS)
end

function Component:OnRemove()
	util.remove({ self.m_x, self.m_y, self.m_z })
end

function Component:OnEntitySpawn()
	local drawInfo = debug.DrawInfo()
	drawInfo:SetIgnoreDepthBuffer(true)

	drawInfo:SetColor(Color.Red)
	self.m_x = debug.draw_line(Vector(0, 0, 0), Vector(5, 0, 0), drawInfo)

	drawInfo:SetColor(Color.Lime)
	self.m_y = debug.draw_line(Vector(0, 0, 0), Vector(0, 5, 0), drawInfo)

	drawInfo:SetColor(Color.Blue)
	self.m_z = debug.draw_line(Vector(0, 0, 0), Vector(0, 0, 5), drawInfo)

	self:SetTickPolicy(ents.TICK_POLICY_ALWAYS)
end

function Component:OnTick()
	if util.is_valid(self.m_x) == false then
		return
	end
	self.m_x:SetPose(self:GetEntity():GetPose())
	self.m_y:SetPose(self:GetEntity():GetPose())
	self.m_z:SetPose(self:GetEntity():GetPose())
end
ents.COMPONENT_DEBUG_DRAW_AXIS = ents.register_component("debug_draw_axis", Component)
