-- SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

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
ents.register_component("debug_draw_axis", Component, "debug")
