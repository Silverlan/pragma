-- SPDX-FileCopyrightText: (c) 2026 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

local ViewerCamera = ents.ViewerCamera

function ViewerCamera:FlyToTarget(target, flyTime)
	local targets = target
	if(type(targets) ~= "table") then targets = {targets} end
	if(#targets == 0) then return end
	util.remove(self.m_cbFlyToTarget)
	local camC = self:GetEntityComponent(ents.COMPONENT_CAMERA)
	if(camC == nil) then return end
	flyTime = flyTime or 0.5

	local startTime = time.real_time()
	local duration = flyTime

	self:SyncPose()
	local startPos = self:GetPivotPos()

	local startZoom = self:GetZoom()

	local min = Vector(math.huge, math.huge, math.huge)
	local max = Vector(-math.huge, -math.huge, -math.huge)
	for _,ent in ipairs(targets) do
		local pos = ent:GetPos()
		local entMin, entMax = util.viewer_camera.get_view_extents(ent)
		entMin = pos +entMin
		entMax = pos +entMax
		min = vector.min(min, entMin)
		max = vector.max(max, entMax)
	end
	local centerPos = (min +max) /2.0
	
	local targetPos = centerPos
	min = min -centerPos
	max = max -centerPos
	local targetZoom = util.viewer_camera.calc_fitting_distance_from_extents(min, max, camC:GetFOV(), camC:GetAspectRatio())
	targetZoom = targetZoom +5.0

	local dtPos = targetPos -startPos
	local dtZoom = targetZoom -startZoom

	self.m_cbFlyToTarget = game.add_callback("Think",function()
		local dt = math.min(time.real_time() -startTime, duration)
		local newPos = pfm.util.easing_quad_ease_in_out(dt, startPos, dtPos, duration)

		local newZoom = pfm.util.easing_quad_ease_in_out(dt, startZoom, dtZoom, duration)
		newZoom = newZoom

		self:SetLookAtTarget(newPos)
		self:SetZoom(newZoom)
		self:UpdatePose()

		if(math.abs(dt -duration) < 0.001) then
			util.remove(self.m_cbFlyToTarget)
		end
	end)
end
