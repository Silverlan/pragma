--[[
    Copyright (C) 2021 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

-- Based on Khronos implementation: https://github.com/KhronosGroup/glTF-Sample-Viewer/blob/d32ca25dc273c0b0982e29efcea01b45d0c85105/src/user_camera.js

util.register_class("ents.ViewerCamera", BaseEntityComponent)
function ents.ViewerCamera:Initialize()
	BaseEntityComponent.Initialize(self)

	self.m_lookAtTarget = Vector()
	self.m_zoom = 1.0
	self.m_up = Vector(0, 1, 0)
	self.m_xRot = 0.0
	self.m_yRot = 0.0
	self.m_scaleFactor = 1.0
	self.m_tLastUpdate = time.real_time()

	self:AddEntityComponent(ents.COMPONENT_TRANSFORM)
	self:AddEntityComponent(ents.COMPONENT_TOGGLE)
	self:AddEntityComponent(ents.COMPONENT_CAMERA)
end

function ents.ViewerCamera:GetLastUpdateTime()
	return self.m_tLastUpdate
end

function ents.ViewerCamera:SetZoom(zoom)
	self.m_zoom = zoom

	self:UpdatePosition()
	self:UpdatePose()
end
function ents.ViewerCamera:GetZoom()
	return self.m_zoom
end

function ents.ViewerCamera:GetLookAtTarget()
	return self.m_lookAtTarget
end
function ents.ViewerCamera:SetLookAtTarget(tgt)
	self.m_lookAtTarget = tgt
end

function ents.ViewerCamera:GetRotation()
	return self.m_xRot, self.m_yRot
end
function ents.ViewerCamera:SetRotation(xRot, yRot)
	self.m_xRot = xRot
	self.m_yRot = yRot
	self.m_tLastUpdate = time.real_time()
end

function ents.ViewerCamera:Rotate(x, y)
	local yMax = math.pi / 2.0 - 0.01
	self.m_xRot = self.m_xRot + math.rad(x)
	self.m_yRot = self.m_yRot + math.rad(y)
	self.m_yRot = math.clamp(self.m_yRot, -yMax, yMax)

	self:UpdatePose()
end

function ents.ViewerCamera:UpdatePose()
	self:UpdatePosition()
	local position = self:GetEntity():GetPos()
	local target = self:GetLookAtTarget()
	local diff = target - position
	local l = diff:Length()
	local dir = vector.FORWARD
	if l > 0.0001 then
		dir = diff / l
	end
	local rot = Quaternion(dir, self.m_up)
	self:GetEntity():SetRotation(rot)

	local camC = self:GetEntity():GetComponent(ents.COMPONENT_CAMERA)
	if camC ~= nil then
		camC:UpdateViewMatrix()
	end

	self.m_tLastUpdate = time.real_time()
	self:BroadcastEvent(ents.ViewerCamera.EVENT_ON_CAMERA_UPDATED, { camC })
end

function ents.ViewerCamera:GetPivotPos()
	local ent = self:GetEntity()
	local pos = ent:GetPos()
	return pos + ent:GetForward() * self:GetZoom()
end

function ents.ViewerCamera:SetPose(pose)
	local rot = pose:GetRotation()
	local dir = rot:GetForward()
	local dist = self:GetZoom()
	local posTgt = pose:GetOrigin() + dir * dist
	self:SetLookAtTarget(posTgt)

	local ang = rot:ToEulerAngles()
	self.m_xRot = math.rad(180 - ang.y)
	self.m_yRot = math.rad(ang.p)
end

function ents.ViewerCamera:ToLocalRotation(vector)
	vector:RotateAround(Vector(), EulerAngles(-math.deg(self.m_yRot), -math.deg(self.m_xRot), 0))
end

function ents.ViewerCamera:UpdatePosition()
	-- calculate direction from focus to camera (assuming camera is at positive z)
	-- yRot rotates *around* x-axis, xRot rotates *around* y-axis
	local direction = Vector(0, 0, 1)
	self:ToLocalRotation(direction)
	local pos = direction * self.m_zoom
	pos = pos + self.m_lookAtTarget
	self:GetEntity():SetPos(pos)
	self.m_tLastUpdate = time.real_time()
end

function ents.ViewerCamera:Pan(x, y)
	local moveSpeed = 1 / (self.m_scaleFactor * 200)

	local left = Vector(-1, 0, 0)
	self:ToLocalRotation(left)
	left = left * (x * moveSpeed)

	local up = Vector(0, 1, 0)
	self:ToLocalRotation(up)
	up = up * (y * moveSpeed)

	self.m_lookAtTarget = self.m_lookAtTarget + up
	self.m_lookAtTarget = self.m_lookAtTarget + left

	self:UpdatePose()
end

function ents.ViewerCamera:GetSceneExtents()
	local renderC = util.is_valid(self.m_target) and self.m_target:GetComponent(ents.COMPONENT_RENDER) or nil
	local min = Vector()
	local max = Vector()
	if renderC ~= nil then
		min, max = renderC:GetLocalRenderBounds()
	else
		local ptC = util.is_valid(self.m_target) and self.m_target:GetComponent(ents.COMPONENT_PARTICLE_SYSTEM) or nil
		if ptC ~= nil then
			min, max = ptC:CalcRenderBounds()
		end
	end
	if min:Distance(max) < 0.0001 then
		local d = util.metres_to_units(5)
		min = Vector(-d, 0, -d)
		max = Vector(d, d, d)
	end
	return min, max
end
function ents.ViewerCamera:FitCameraTargetToExtents(min, max)
	self.m_lookAtTarget = (min + max) / 2.0

	self:UpdatePose()
end
function ents.ViewerCamera:GetFittingZoom(axisLength)
	local camC = self:GetEntity():GetComponent(ents.COMPONENT_CAMERA)
	if camC == nil then
		return 0.0
	end
	local xFov = math.rad(camC:GetFOV())
	local yFov = xFov * camC:GetAspectRatio()
	local yZoom = axisLength / 2.0 / math.tan(yFov / 2.0)
	local xZoom = axisLength / 2.0 / math.tan(xFov / 2.0)
	return math.max(xZoom, yZoom)
end
function ents.ViewerCamera:FitZoomToExtents(min, max)
	local maxAxisLength = math.max(max.x - min.x, max.y - min.y, max.z - min.z)
	self.m_zoom = self:GetFittingZoom(maxAxisLength)

	self:UpdatePose()
end
function ents.ViewerCamera:FitViewToScene(min, max)
	if min == nil then
		min, max = self:GetSceneExtents()
	end
	self:FitCameraTargetToExtents(min, max)
	self:FitZoomToExtents(min, max)

	-- Zoom out a little bit
	self:SetZoom(self:GetZoom() + 15.0)
	return min, max
end
function ents.ViewerCamera:SetTarget(target)
	self.m_target = target
	self:FitViewToScene()
	self:SetRotation(0.0, 0.0)

	self:Rotate(-35, 15)
end
function ents.ViewerCamera:GetTarget()
	return self.m_target
end
function ents.ViewerCamera:SetFovY(fov)
	local camC = self:GetEntity():GetComponent(ents.COMPONENT_CAMERA)
	if camC == nil then
		return
	end
	camC:SetFOV(math.vertical_fov_to_horizontal_fov(fov, self.m_width, self.m_height))
	camC:UpdateProjectionMatrix()
	self.m_tLastUpdate = time.real_time()
end
function ents.ViewerCamera:Setup(width, height)
	self.m_width = width
	self.m_height = height
	local camC = self:GetEntity():GetComponent(ents.COMPONENT_CAMERA)
	if camC == nil then
		return
	end
	camC:SetAspectRatio(width / height)
	camC:UpdateViewMatrix()
	camC:UpdateProjectionMatrix()
	self.m_tLastUpdate = time.real_time()

	self:BroadcastEvent(ents.ViewerCamera.EVENT_ON_CAMERA_UPDATED, { camC })
end
function ents.ViewerCamera:OnEntitySpawn() end
ents.COMPONENT_VIEWER_CAMERA = ents.register_component("viewer_camera", ents.ViewerCamera)
ents.ViewerCamera.EVENT_ON_CAMERA_UPDATED =
	ents.register_component_event(ents.COMPONENT_VIEWER_CAMERA, "on_camera_updated")
