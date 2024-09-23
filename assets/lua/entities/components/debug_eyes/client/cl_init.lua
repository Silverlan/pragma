--[[
    Copyright (C) 2021 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

util.register_class("ents.DebugEyes", BaseEntityComponent)
local Component = ents.DebugEyes

function Component:__init()
	BaseEntityComponent.__init(self)
end

function Component:Initialize()
	BaseEntityComponent.Initialize(self)
end

function Component:OnEntitySpawn()
	self.m_cbDrawLines = game.add_callback("Think", function()
		self:DrawLines()
	end)
end

function Component:OnRemove()
	util.remove(self.m_cbDrawLines)
end

function Component:DrawLines()
	local drawInfo = debug.DrawInfo()
	drawInfo:SetDuration(0.0001)
	local ent = self:GetEntity()
	local mdl = ent:GetModel()
	local eyeC = ent:GetComponent(ents.COMPONENT_EYE)
	local flexC = ent:GetComponent(ents.COMPONENT_FLEX)
	local numEyes = (eyeC ~= nil) and mdl:GetEyeballCount() or 0
	if mdl == nil or numEyes == 0 then
		return
	end
	for i = 0, numEyes - 1 do
		local vt = eyeC:GetViewTarget()
		local pose = eyeC:CalcEyeballPose(i)
		if pose ~= nil then
			drawInfo:SetColor(Color.Red)
			debug.draw_line(pose:GetOrigin(), vt, drawInfo)

			--[[
			-- TODO: This is probably not correct
			if(flexC ~= nil) then
				flexC:SetFlexController("eyes_updown",0)
				flexC:SetFlexController("eyes_rightleft",0)
				local eyesUpDown = flexC:GetFlexController("eyes_updown")
				local eyesRightLeft = flexC:GetFlexController("eyes_rightleft")
				if(eyesUpDown ~= 0 or eyesRightLeft ~= 0) then
					local pos = pose:GetOrigin()
					local rot = pose:GetRotation()
					local ang = rot:ToEulerAngles()
					ang.p = ang.p +eyesUpDown
					ang.y = ang.y +eyesRightLeft
					rot = ang:ToQuaternion()
					drawInfo:SetColor(Color.Lime)
				end
			end]]
		end
	end
end
ents.register_component("debug_eyes", Component, "debug")
